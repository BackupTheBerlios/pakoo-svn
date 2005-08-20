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
#include "../core/portagetree.h"
#include "../core/portagesettings.h"
#include "../core/portagecategory.h"

#include <qdatetime.h>
#include <qapplication.h>

#include <klocale.h>
#include <kdebug.h>

#define DO_ABORT { \
	emitDebugOutput( "Aborting the tree scanner on request." ); \
	return Failure; \
}


namespace libpakt {

PortageTreeScanner::PortageTreeScanner()
: ThreadedJob(),
  rxVersion("(-\\d+(?:\\.\\d+)*[a-z]?)")
{
	packages = NULL;
	scanAvailablePackages = true;
	scanInstalledPackages = true;
}

/**
 * Set the PortageSettings object that contains directory and cache info.
 */
void PortageTreeScanner::setSettingsObject( PortageSettings* settings )
{
	this->settings = settings;
}

/**
 * Set the PackageList object that will be filled with packages.
 */
void PortageTreeScanner::setPackageList( PackageList* packages )
{
	this->packages = packages;
}

/**
 * Define if you'd like to scan the Portage tree for available packages,
 * which are the packages in the mainline tree and the overlay ones.
 * By default, this is set to true.
 */
void PortageTreeScanner::setScanAvailablePackages(
	bool scanAvailablePackages )
{
	this->scanAvailablePackages = scanAvailablePackages;
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
	this->scanInstalledPackages = scanInstalledPackages;
}


/**
 * Load a package list by scanning the portage tree for packages.
 */
IJob::JobResult PortageTreeScanner::performThread()
{
	// load the settings
	if( settings == NULL ) {
		kdDebug() << i18n( "PortageTreeScanner debug output",
			"PortageTreeScanner::performThread(): "
			"Didn't start because the settings object is NULL" )
			<< endl;
		return Failure;
	}
	else {
		preferCache = settings->preferCache();
		mainlineTreeDir = settings->mainlineTreeDirectory();
		overlayTreeDirs = settings->overlayTreeDirectories();
		installedPackagesDir = settings->installedPackagesDirectory();
		cacheDir = settings->cacheDirectory();
	}

	// initialize package count
	packageCountAvailable = 0;
	packageCountInstalled = 0;

	if( packages == NULL ) {
		kdDebug() << i18n( "PortageTreeScanner debug output",
			"PortageTreeScanner::performThread(): "
			"Didn't start because the PackageList object is NULL" )
			<< endl;
		return Failure;
	}

	QDateTime startTime = QDateTime::currentDateTime();
	emitDebugOutput( "Scanning the portage tree..." );

	if( scanAvailablePackages == true )
	{
		// scan the mainline tree
		if( !scanTree(mainlineTreeDir, Mainline) )
			DO_ABORT;

		// scan the overlay trees
		for( QStringList::iterator overlayIterator = overlayTreeDirs.begin();
		     overlayIterator != overlayTreeDirs.end(); overlayIterator++ )
		{
			if( !scanTree(*overlayIterator, Overlay) )
				DO_ABORT;
		}
	}
	if( scanInstalledPackages == true )
	{
		// scan the installed packages database
		if( !scanTree(installedPackagesDir, Installed) )
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
		emitDebugOutput( "Invalid tree directory." );
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

	currentPackage = NULL;
	currentVersion = NULL;

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

		currentCategory.setCategory( (*categoryIterator).left(pos),
		                             (*categoryIterator).mid(pos+1) );

		// If the Portage cache is searched, do this
		if( (treeType == Mainline) && (preferCache == true) )
		{
			// Compose the folder name of the current category
			d.setPath( cacheDir + mainlineTreeDir + "/" + (*categoryIterator) );
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
					currentPackage = packages->package(
						new PortageCategory(currentCategory),
						*packageNameIterator
					);
					scanTreePackage( d, false );
				}
				else if( treeType == Overlay )
				{
					currentPackage = packages->package(
						new PortageCategory(currentCategory),
						*packageNameIterator
					);
					scanTreePackage( d, true );
				}
				else // if( treeType == Installed )
				{
					scanInstalledPackage( d );
				}

				// send a status update every 20 packages
				if( (packageCountAvailable + packageCountInstalled) % 20 == 0 )
					emitPackagesScanned();

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
		treeName = "mainline tree"; break;
	case Overlay:
		treeName = "overlay tree"; break;
	case Installed:
		treeName = "installed packages database"; break;
	}
	emitDebugOutput(
		QString("Finished scanning %1 in %2... (%3 seconds)")
			.arg( treeName )
			.arg( treeDir )
			.arg( startTime.secsTo(QDateTime::currentDateTime()) )
	);

	return true;
} // end of scanPackageList()


/**
 * Iterate through a directory's ebuild files and add the found
 * package versions to the currentPackage object.
 *
 * @param d        The directory containing the ebuilds
 * @param overlay  The value for version->overlay
 *                 (set true if it's an overlay directory)
 */
void PortageTreeScanner::scanTreePackage( QDir& d, bool overlay )
{
	// Iterate through all ebuild files of the current currentPackage
	QStringList ebuilds = d.entryList( "*.ebuild" );
	QStringList::iterator ebuildIteratorEnd = ebuilds.end();

	for ( QStringList::iterator ebuildIterator = ebuilds.begin();
	      ebuildIterator != ebuildIteratorEnd; ++ebuildIterator )
	{
		// add version info
		currentVersion = currentPackage->version(
			(*ebuildIterator).mid( // extract the package version string
				(currentPackage->name()).length() + 1,
				(*ebuildIterator).length() - 7
					- ((currentPackage->name()).length() + 1)
			)
		);
		currentVersion->overlay = overlay;
	}
	packageCountAvailable++;
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

	// Iterate through all ebuild files of the current currentPackage
	QStringList files = d.entryList();
	QStringList::iterator fileIteratorEnd = files.end();

	for ( QStringList::iterator fileIterator = files.begin();
	      fileIterator != fileIteratorEnd; ++fileIterator )
	{
		// no "." or ".." directories
		if( (*fileIterator)[0] == '.' )
			continue;


		int packageNameEndIndex = (*fileIterator).findRev( rxVersion );
		packageName = (*fileIterator).left( packageNameEndIndex );

		// See if it's a new package (if not, it's just another version)
		if( (currentPackage == NULL)
		    || (packageName != currentPackage->name()) )
		{
			// Separate package name from version
			currentPackage = packages->package(
				new PortageCategory( currentCategory ),
				packageName
			);
			packageCountAvailable++;

			// send a status update every 20 packages
			if( (packageCountAvailable + packageCountInstalled) % 20 == 0 )
				emitPackagesScanned();
		}

		// extract the package version string, and add version info
		currentVersion = currentPackage->version(
			(*fileIterator).mid( (currentPackage->name()).length() + 1 )
		);
		currentVersion->overlay = false;
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
	int packageNameEndIndex = dirName.findRev( rxVersion );

	// Separate package name from version
	currentPackage = packages->package(
		new PortageCategory( currentCategory ),
		dirName.left( packageNameEndIndex ) // package name
	);
	currentVersion = currentPackage->version(
		dirName.mid( packageNameEndIndex + 1 )
	);
	currentVersion->installed = true;

	packageCountInstalled++;
}


/**
 * From within the thread, emit a finishedLoading() signal to the main thread.
 */
void PortageTreeScanner::emitFinishedLoading()
{
	FinishedLoadingEvent* event = new FinishedLoadingEvent();
	event->packages = this->packages;
	QApplication::postEvent( this, event );
}

/**
 * From within the thread, emit a packagesScanned() signal to the main thread.
 */
void PortageTreeScanner::emitPackagesScanned()
{
	PackagesScannedEvent* event  = new PackagesScannedEvent();
	event->packageCountAvailable = this->packageCountAvailable;
	event->packageCountInstalled = this->packageCountInstalled;
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
		emit finishedLoading( packages );

	default:
		ThreadedJob::customEvent( event );
		break;
	}
}

} // namespace
