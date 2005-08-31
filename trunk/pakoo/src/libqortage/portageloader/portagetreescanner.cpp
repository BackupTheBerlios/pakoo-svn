/***************************************************************************
 *   Copyright (C) 2004 by karye <karye@users.sourceforge.net>             *
 *   Copyright (C) 2005 by Jakob Petsovits <jpetso@gmx.at>                 *
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

#include "portagetreescanner.h"

#include "../core/packageversion.h"
#include "../core/package.h"
#include "../core/packagelist.h"
#include "../core/portagesettings.h"
#include "../core/portagecategory.h"

#include <qdatetime.h>
#include <qapplication.h>

#include <klocale.h>
#include <kdebug.h>

#define DO_ABORT { \
	kdDebug() << i18n( "PortageTreeScanner debug output", \
		"PortageTreeScanner: Aborting on user request." ) \
		<< endl; \
	return Failure; \
}


namespace libpakt {

PortageTreeScanner::PortageTreeScanner()
: ThreadedJob(),
  m_rxVersion("(-\\d+(?:\\.\\d+)*[a-z]?)")
{
	m_packages = NULL;
	m_scanAvailablePackages = true;
	m_scanInstalledPackages = true;
}

/**
 * Set the PortageSettings object that contains directory and cache info.
 */
void PortageTreeScanner::setSettingsObject( PortageSettings* settings )
{
	m_settings = settings;
}

/**
 * Set the PackageList object that will be filled with packages.
 */
void PortageTreeScanner::setPackageList( PackageList* packages )
{
	m_packages = packages;
}

/**
 * Define if you'd like to scan the Portage tree for available packages,
 * which are the packages in the mainline tree and the overlay ones.
 * By default, this is set to true.
 */
void PortageTreeScanner::setScanAvailablePackages(
	bool scanAvailablePackages )
{
	m_scanAvailablePackages = scanAvailablePackages;
}

/**
 * Define if you'd like to scan the Portage tree for installed packages.
 * In most (but not all) cases, package versions found here will also be
 * found when scanning for available packages - but the version's 'installed'
 * flag will only be set when it is found in the installed packages database.
 * By default, this is set to true.
 */
void PortageTreeScanner::setScanInstalledPackages(
	bool scanInstalledPackages )
{
	m_scanInstalledPackages = scanInstalledPackages;
}


/**
 * Load a package list by scanning the portage tree for packages.
 */
IJob::JobResult PortageTreeScanner::performThread()
{
	// load the settings
	if( m_settings == NULL ) {
		kdDebug() << i18n( "PortageTreeScanner debug output",
			"PortageTreeScanner::performThread(): "
			"Didn't start because the settings object is NULL" )
			<< endl;
		return Failure;
	}
	else {
		m_preferCache = m_settings->preferCache();
		m_mainlineTreeDir = m_settings->mainlineTreeDirectory();
		m_overlayTreeDirs = m_settings->overlayTreeDirectories();
		m_installedPackagesDir = m_settings->installedPackagesDirectory();
		m_cacheDir = m_settings->cacheDirectory();
	}

	// initialize package count
	m_packageCountAvailable = 0;
	m_packageCountInstalled = 0;

	if( m_packages == NULL ) {
		kdDebug() << i18n( "PortageTreeScanner debug output",
			"PortageTreeScanner::performThread(): "
			"Didn't start because the PackageList object is NULL" )
			<< endl;
		return Failure;
	}

	QDateTime startTime = QDateTime::currentDateTime();
	kdDebug() << i18n( "PortageTreeScanner debug output",
		"PortageTreeScanner::performThread(): "
		"Scanning the portage tree..." )
		<< endl;

	if( m_scanAvailablePackages == true )
	{
		// scan the mainline tree
		if( !scanTree(m_mainlineTreeDir, Mainline) )
			DO_ABORT;

		// scan the overlay trees
		for( QStringList::iterator overlayIterator = m_overlayTreeDirs.begin();
		     overlayIterator != m_overlayTreeDirs.end(); overlayIterator++ )
		{
			if( !scanTree(*overlayIterator, Overlay) )
				DO_ABORT;
		}
	}
	if( m_scanInstalledPackages == true )
	{
		// scan the installed packages database
		if( !scanTree(m_installedPackagesDir, Installed) )
			DO_ABORT;
	}


	kdDebug() << i18n( "PortageTreeScanner debug output",
		"PortageTreeScanner::performThread(): "
		"Finished scanning the whole Portage tree in %1 seconds" )
			.arg( startTime.secsTo(QDateTime::currentDateTime()) )
		<< endl;
	emitFinishedLoading();
	return Success;
}


/**
 * Scan a search directory for packages. The treeType parameter defines
 * which ones of the tree directories is searched. This function assumes
 * that this object's packages member is a valid PackageList object
 * (especially not NULL).
 *
 * @param treeDir  The search directory containing package information
 * @param treeType  Defines which directory should be searched.
 *                  This is one of the Mainline, Overlay or Installed
 *                  values of this class's TreeType enumeration.
 *
 * @return false if the thread is aborting, true otherwise. In case
 *         of errors, a debug error message is emitted through debugOutput().
 */
bool PortageTreeScanner::scanTree( const QString& treeDir,
                                   TreeType treeType )
{
	QDateTime startTime = QDateTime::currentDateTime();
	QDir d;
	int pos;

	// set the d directory to the treeDir string
	d.setPath( treeDir );
	if( treeDir.isNull() || treeDir.isEmpty() || !d.exists() ) {
		kdDebug() << i18n( "PortageTreeScanner debug output",
			"PortageTreeScanner::performThread(): Invalid tree directory." )
			<< endl;
		return true;
	}

	if( treeType == Installed ) {
		// Scan only folders, no files
		// (the package name and version is contained in the folder name)
		d.setFilter( QDir::Dirs | QDir::NoSymLinks );
	}
	else {
		// Scan folders and (ebuild) files
		d.setFilter( QDir::Dirs | QDir::NoSymLinks | QDir::Files );
	}
	d.setSorting( QDir::Name );

	m_currentPackage = NULL;
	m_currentVersion = NULL;

	// Iterate through the available categories (e.g. sys-kernel)
	QStringList categories = d.entryList();
	QStringList::iterator categoryIteratorEnd = categories.end();
	for ( QStringList::iterator categoryIterator = categories.begin();
	      categoryIterator != categoryIteratorEnd; ++categoryIterator )
	{
		pos = (*categoryIterator).find('-', 1);

		// don't process unwanted directories
		if( pos == -1 ) { // doesn't contain '-', so it's a non-package dir
			continue;
		}

		// Extract the category and subcategory from the folder name

		m_currentCategory.setCategory( (*categoryIterator).left(pos),
		                             (*categoryIterator).mid(pos+1) );

		// If the Portage cache is searched, do this
		if( (treeType == Mainline) && (m_preferCache == true) )
		{
			// Compose the folder name of the current category
			d.setPath( m_cacheDir + m_mainlineTreeDir + "/"
			           + (*categoryIterator) );
			if( !d.exists() )
				continue;

			scanCacheCategory(d);

			if( aborting() )
				return false; // means: abort!
		}
		// If the normal portage tree is searched, do that
		else
		{
			// Compose the folder name of the current category
			d.setPath( treeDir + "/" + (*categoryIterator) );
			if( !d.exists() ) {
				continue;
			}

			// Iterate through the available package dirs
			// in the current category
			QStringList packageNames = d.entryList();
			QStringList::iterator packageNameIterator = packageNames.begin();
			QStringList::iterator packageNameIteratorEnd = packageNames.end();
			for( ; packageNameIterator != packageNameIteratorEnd;
			     ++packageNameIterator )
			{
				// no "." or ".." directories
				if( (*packageNameIterator)[0] == '.' )
					continue;

				// Compose the folder name of the current package and check it
				d.setPath( treeDir + "/" + (*categoryIterator)
				           + "/" + (*packageNameIterator) );
				if ( !d.exists() ) {
					continue;
				}

				if( treeType == Mainline )
				{
					m_currentPackage = m_packages->package(
						new PortageCategory(m_currentCategory),
						*packageNameIterator
					);
					scanTreePackage( d, false );
				}
				else if( treeType == Overlay )
				{
					m_currentPackage = m_packages->package(
						new PortageCategory(m_currentCategory),
						*packageNameIterator
					);
					scanTreePackage( d, true );
				}
				else // if( treeType == Installed )
				{
					scanInstalledPackage( d );
				}

				// send a status update every 20 packages
				if( (m_packageCountAvailable + m_packageCountInstalled) % 20
				    == 0 )
				{
					emitPackagesScanned();
				}

				if( aborting() )
					return false; // means: abort!

			} // end of package iteration
		} // end of if( mainline/edb )
	} // end of category iteration

	// report success on this tree, using debug output
	QString treeName;
	switch( treeType )
	{
	case Mainline:
		treeName = i18n("PortageTreeScanner tree type string #1 "
		                "(for debug output)",
		                "mainline tree"); break;
	case Overlay:
		treeName = i18n("PortageTreeScanner tree type string #2"
		                "(for debug output)",
		                "overlay tree"); break;
	case Installed:
		treeName = i18n("PortageTreeScanner tree type string #3"
		                "(for debug output)",
		                "installed packages database"); break;
	}
	kdDebug() << i18n( "PortageTreeScanner debug output."
	                   "%1 is a PortageTreeScanner tree type string, "
	                   "%2 is its directory and %3 are the seconds.",
		"PortageTreeScanner::performThread(): "
		"Finished scanning %1 in %2... (%3 seconds)")
			.arg( treeName )
			.arg( treeDir )
			.arg( startTime.secsTo(QDateTime::currentDateTime()) )
		<< endl;

	return true;
} // end of scanPackageList()


/**
 * Iterate through a directory's ebuild files and add the found
 * package versions to the m_currentPackage object.
 *
 * @param d        The directory containing the ebuilds
 * @param overlay  The value for version->overlay
 *                 (set true if it's an overlay directory)
 */
void PortageTreeScanner::scanTreePackage( QDir& d, bool overlay )
{
	// Iterate through all ebuild files of the current m_currentPackage
	QStringList ebuilds = d.entryList( "*.ebuild" );
	QStringList::iterator ebuildIteratorEnd = ebuilds.end();

	for ( QStringList::iterator ebuildIterator = ebuilds.begin();
	      ebuildIterator != ebuildIteratorEnd; ++ebuildIterator )
	{
		// add version info
		m_currentVersion = m_currentPackage->version(
			(*ebuildIterator).mid( // extract the package version string
				(m_currentPackage->name()).length() + 1,
				(*ebuildIterator).length() - 7
					- ((m_currentPackage->name()).length() + 1)
			)
		);
		m_currentVersion->overlay = overlay;
	}
	m_packageCountAvailable++;
} // end of scanTreePackage()

/**
 * Search a category in the installed packages database (edb/dep/...)
 * and add the found packages and package versions the package list.
 *
 * @param d  The category directory containing the package files
 */
void PortageTreeScanner::scanCacheCategory( QDir& d )
{
	QString packageName;

	// Iterate through all ebuild files of the current m_currentPackage
	QStringList files = d.entryList();
	QStringList::iterator fileIteratorEnd = files.end();

	for ( QStringList::iterator fileIterator = files.begin();
	      fileIterator != fileIteratorEnd; ++fileIterator )
	{
		// no "." or ".." directories
		if( (*fileIterator)[0] == '.' )
			continue;


		int packageNameEndIndex = (*fileIterator).findRev( m_rxVersion );
		packageName = (*fileIterator).left( packageNameEndIndex );

		// See if it's a new package (if not, it's just another version)
		if( (m_currentPackage == NULL)
		    || (packageName != m_currentPackage->name()) )
		{
			// Separate package name from version
			m_currentPackage = m_packages->package(
				new PortageCategory( m_currentCategory ),
				packageName
			);
			m_packageCountAvailable++;

			// send a status update every 20 packages
			if( (m_packageCountAvailable + m_packageCountInstalled) % 20
			    == 0 )
			{
				emitPackagesScanned();
			}
		}

		// extract the package version string, and add version info
		m_currentVersion = m_currentPackage->version(
			(*fileIterator).mid( (m_currentPackage->name()).length() + 1 )
		);
		m_currentVersion->overlay = false;
	}
} // end of scanCacheCategory()

/**
 * Extract package name, version, and modification date from a directory name,
 * and add a corresponding package to the package list.
 *
 * @param d  The directory named after the (installed) package
 */
void PortageTreeScanner::scanInstalledPackage( QDir& d )
{
	QString dirName = d.dirName();
	int packageNameEndIndex = dirName.findRev( m_rxVersion );

	// Separate package name from version
	m_currentPackage = m_packages->package(
		new PortageCategory( m_currentCategory ),
		dirName.left( packageNameEndIndex ) // package name
	);
	m_currentVersion = m_currentPackage->version(
		dirName.mid( packageNameEndIndex + 1 )
	);
	m_currentVersion->installed = true;

	m_packageCountInstalled++;
}


/**
 * From within the thread, emit a finishedLoading() signal to the main thread.
 */
void PortageTreeScanner::emitFinishedLoading()
{
	FinishedLoadingEvent* event = new FinishedLoadingEvent();
	event->packages = m_packages;
	QApplication::postEvent( this, event );
}

/**
 * From within the thread, emit a packagesScanned() signal to the main thread.
 */
void PortageTreeScanner::emitPackagesScanned()
{
	PackagesScannedEvent* event  = new PackagesScannedEvent();
	event->packageCountAvailable = m_packageCountAvailable;
	event->packageCountInstalled = m_packageCountInstalled;
	QApplication::postEvent( this, event );
}

/**
 * Translates QCustomEvents into signals. This function is called from Qt
 * in the main thread, which guarantees safety for emitting signals.
 */
void PortageTreeScanner::customEvent( QCustomEvent* event )
{
	switch( event->type() )
	{
	case (int) PackagesScannedEventType:
		emit packagesScanned(
			((PackagesScannedEvent*)event)->packageCountAvailable,
			((PackagesScannedEvent*)event)->packageCountInstalled
		);
		break;

	case (int) FinishedLoadingEventType:
		emit finishedLoading( m_packages );

	default:
		ThreadedJob::customEvent( event );
		break;
	}
}

} // namespace
