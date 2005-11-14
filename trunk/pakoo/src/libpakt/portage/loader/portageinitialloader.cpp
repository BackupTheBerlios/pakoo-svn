/***************************************************************************
 *   Copyright (C) 2005 by Jakob Petsovits                                 *
 *   jpetso@gmx.at                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "portageinitialloader.h"

#include "../core/portagepackage.h"
#include "../../core/packagelist.h"
#include "../core/portagesettings.h"
#include "profileloader.h"
#include "portagetreescanner.h"
#include "portageml.h"
#include "filepackagemaskloader.h"
#include "filepackagekeywordsloader.h"

#include <qapplication.h>

#include <klocale.h>
#include <kglobalsettings.h>
#include <kdebug.h>


#define CHECK_ABORT if( aborting() ) { \
	emitCurrentTaskChanged( \
		i18n("PortageInitialLoader aborted by abort()", \
			 "Loading packages was aborted on user request.") \
	); \
	return Failure; \
}

#define DO_FAILURE { \
	emitCurrentTaskChanged( \
		i18n("PortageInitialLoader error", \
		     "Could not load packages (there was an error).") \
	); \
	return Failure; \
}


namespace libpakt {

PortageInitialLoader::PortageInitialLoader() : InitialLoader()
{}

/**
 * Set the PortageSettings object that will be
 * filled with configuration values.
 */
void PortageInitialLoader::setSettingsObject( PortageSettings* settings )
{
	m_settings = settings;
}

/**
 * Load everything that's needed for initially displaying the package
 * tree. In case of Portage, this is the global settings and the
 * package tree.
 */
IJob::JobResult PortageInitialLoader::performThread()
{
	if( m_packages == NULL ) {
		kdDebug() << i18n( "PortageInitialLoader debug output",
			"PortageInitialLoader::performThread(): "
			"Didn't start because the PackageList object is NULL" )
			<< endl;
		return Failure;
	}
	TemplatedPackageList<PortagePackage>* portagePackages =
		(TemplatedPackageList<PortagePackage>*) m_packages;

	JobResult result;

	//
	//TODO: Configuration values that should be
	//      read from a configuration file (KConfigXT for the lib, please?)
	//
	QString filename = KGlobalSettings::documentPath() + "/portagetree.xml";
	QString globalPackageMaskFile = "profiles/package.mask";
	QString etcPackageMaskFile = "/etc/portage/package.mask";
	QString etcPackageUnmaskFile = "/etc/portage/package.unmask";
	QString etcPackageKeywordsFile = "/etc/portage/package.keywords";


	//
	// load global Portage settings, like Portage directories and ARCH
	//
	emitCurrentTaskChanged(
		i18n("PortageInitialLoader task #1",
		     "Loading global Portage settings...")
	);
	ProfileLoader* profileLoader = new ProfileLoader();
	profileLoader->setSettingsObject( m_settings );

	result = profileLoader->perform();
	profileLoader->deleteLater();

	if( result == IJob::Failure )
		DO_FAILURE;

	emitProgressChanged( 1, 10 );


	//
	// set up the TreeScanner and load the package tree
	//
	emitCurrentTaskChanged(
		i18n("PortageInitialLoader task #2",
		     "Loading packages from the Portage tree...")
	);
	PortageTreeScanner* treeScanner = new PortageTreeScanner();
	treeScanner->setPackageList( portagePackages );
	treeScanner->setSettingsObject( m_settings );

	connect( treeScanner, SIGNAL( packagesScanned(int,int) ),
	         this,          SLOT( emitPackagesScanned(int,int) ) );
	connect( this,        SIGNAL( aborted() ),
	         treeScanner,   SLOT( abort() ) );

	result = treeScanner->perform();
	this->disconnect( treeScanner ); // disconnects abort()
	treeScanner->deleteLater(); // disconnects everything else
	CHECK_ABORT;

	if( result == Failure )
	{
		emitCurrentTaskChanged(
			i18n("PortageInitialLoader task #3 (%1 is the filename)",
				 "Loading packages from %1...")
			.arg( filename )
		);
		PortageML* portageML = new PortageML();
		portageML->setAction( PortageML::LoadFile );
		portageML->setPackageList( portagePackages );
		portageML->setFileName( filename );

		connect( portageML, SIGNAL( packagesScanned(int,int) ),
		         this,        SLOT( emitPackagesScanned(int,int) ) );
		connect( this,      SIGNAL( aborted() ),
		         portageML,   SLOT( abort() ) );

		result = portageML->perform();
		this->disconnect( portageML ); // disconnects abort()
		portageML->deleteLater(); // disconnects everything else
		CHECK_ABORT;

		if( result == Failure )
			DO_FAILURE;
		// else: go on
	}

	emitProgressChanged( 9, 10 );


	//
	// modify the loaded packages according to the entries in
	// package.keywords, package.mask and package.unmask
	//
	FilePackageMaskLoader* maskLoader = new FilePackageMaskLoader();
	maskLoader->setPackageList( portagePackages );

	// package.mask files (in /usr/portage/profiles and /etc/portage)
	maskLoader->setMode( FilePackageMaskLoader::Mask );
	maskLoader->setFileName(
		m_settings->mainlineTreeDirectory() + "/" + globalPackageMaskFile );
	maskLoader->perform();
	maskLoader->setFileName( etcPackageMaskFile );
	maskLoader->perform();

	// package.unmask file (in /etc/portage)
	maskLoader->setMode( FilePackageMaskLoader::Unmask );
	maskLoader->setFileName( etcPackageMaskFile );
	maskLoader->perform();

	// package.keywords file (in /etc/portage)
	FilePackageKeywordsLoader* keywordsLoader
		= new FilePackageKeywordsLoader();
	keywordsLoader->setPackageList( portagePackages );
	keywordsLoader->setFileName( etcPackageKeywordsFile );
	keywordsLoader->perform();

	// done!
	emitFinishedLoading( m_packages );

	return Success;
}

/**
 * Reimplemented to make child objects abort immediately.
 */
void PortageInitialLoader::abort( )
{
	ThreadedJob::abort();
	emit aborted();
}

/**
 * From within the thread, emit a currentTaskChanged() signal to the main
 * thread, containing info about the number of currently scanned packages.
 */
void PortageInitialLoader::emitPackagesScanned( int packageCountAvailable,
                                                int packageCountInstalled )
{
	emitCurrentTaskChanged(
		i18n("PortageInitialLoader task #2a",
			 "Loading packages from the Portage tree: "
		     "%1 packages, %2 installed...")
		.arg( packageCountAvailable )
		.arg( packageCountInstalled )
	);
}

/**
 * Additionally to the outcome of InitialLoader::emitFinishedLoading(),
 * emit currentTaskChanged() and progressChanged() to signal completeness
 * of the whole operation.
 */
void PortageInitialLoader::emitFinishedLoading( PackageList* packages )
{
	PortageFinishedLoadingEvent* event = new PortageFinishedLoadingEvent();
	QApplication::postEvent( this, event );
	InitialLoader::emitFinishedLoading( packages );
}

/**
 * Translates QCustomEvents into signals. This function is called from Qt
 * in the main thread, which guarantees safety for emitting signals.
 */
void PortageInitialLoader::customEvent( QCustomEvent* event )
{
	switch( event->type() )
	{
	case (int) PortageFinishedLoadingEventType:
		emit currentTaskChanged( i18n("PortageInitialLoader tasks completed",
			"Packages have successfully been loaded.") );
		emit progressChanged( 10, 10 );

	default:
		InitialLoader::customEvent( event );
		break;
	}
}

}
