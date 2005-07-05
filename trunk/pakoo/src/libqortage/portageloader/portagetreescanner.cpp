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
#include "packagescanner.h"
#include "loadingevent.h"

#include <qevent.h>
#include <qdatetime.h>
#include <qapplication.h>

#define DO_ABORT { finishProcessing(); return AbortedError; }


/**
 * Initialize this object, defining where to search for packages.
 *
 * @param treeDir       The mainline tree directory.
 * @param overlayDirs   The overlay tree directories.
 * @param installedDir  The installed packages database directory.
 * @param edbDepDir     The directory where the portage cache resides.
 */
PortageTreeScanner::PortageTreeScanner(
	QString treeDir, QStringList overlayDirs,
	QString installedDir, QString edbDepDir )
: portageTreeDir(treeDir), portageOverlayDirs(overlayDirs),
  installedPackagesDir(installedDir), edbDir(edbDepDir),
  rxVersion("(-\\d+(?:\\.\\d+)*[a-z]?)")
{
	tree = NULL;
	pkgScanner = new PackageScanner( treeDir, overlayDirs, installedDir );
}

/**
 * Deconstruct this object.
 */
PortageTreeScanner::~PortageTreeScanner()
{
	delete pkgScanner;
}

/**
 * Set the portage tree search directory to a new value.
 */
void PortageTreeScanner::setMainlineTreeDirectory( QString directory )
{
	portageTreeDir = directory;
	pkgScanner->setMainlineTreeDirectory( directory );
}

/**
 * Set the portage cache directory to a new value.
 */
void PortageTreeScanner::setEdbDepDirectory( QString directory )
{
	edbDir = directory;
	pkgScanner->setEdbDepDirectory( directory );
}

/**
 * Set the list of portage overlay directories to a new value.
 */
void PortageTreeScanner::setOverlayTreeDirectories( QStringList directories )
{
	portageOverlayDirs = directories;
	pkgScanner->setOverlayTreeDirectories( directories );
}

/**
 * Set the installed packages database directory to a new value.
 */
void PortageTreeScanner::setInstalledPackagesDirectory( QString directory )
{
	installedPackagesDir = directory;
	pkgScanner->setInstalledPackagesDirectory( directory );
}

/**
 * Return a pointer to a PackageScanner object with same settings
 * (search folders, and stuff) like this PortageTreeScanner has.
 */
PackageScanner* PortageTreeScanner::packageScanner()
{
	return pkgScanner;
}


/**
 * Load a portage tree by scanning portage for packages.
 *
 * @param portageTree    The PortageTree object that will be filled with packages.
 * @param searchedTrees  Which ones of the trees will be searched.
 * @param preferEdb      If true, the edb/dep/ directory will be searched
 *                       instead of the mainline tree, which will probably
 *                       be much faster.
 */
PortageLoaderBase::Error PortageTreeScanner::scanTrees(
	PortageTree* portageTree, PortageTree::Trees searchedTrees, bool preferEdb
)
{
	if( this->working == true)
		return AlreadyRunningError;
	else
		initProcessing();

	// initialize package count
	installedPackageCount = 0;

	if( portageTree == NULL ) {
		finishProcessing();
		return NullObjectError;
	}

	tree = portageTree;
	this->preferEdb = preferEdb;

	if( searchedTrees == PortageTree::All ) {
		if( scanMainlineTree() == AbortedError ) DO_ABORT;
		if( scanOverlayTrees() == AbortedError ) DO_ABORT;
		if( scanInstalledTree() == AbortedError ) DO_ABORT;
	}
	else if( searchedTrees == PortageTree::MainlineAndOverlay ) {
		if( scanMainlineTree() == AbortedError ) DO_ABORT;
		if( scanOverlayTrees() == AbortedError ) DO_ABORT;
	}
	else if( searchedTrees == PortageTree::Mainline ) {
		if( scanMainlineTree() == AbortedError ) DO_ABORT;
	}
	else if( searchedTrees == PortageTree::Overlay ) {
		if( scanOverlayTrees() == AbortedError ) DO_ABORT;
	}
	else if( searchedTrees == PortageTree::Installed ) {
		if( scanInstalledTree() == AbortedError ) DO_ABORT;
	}

	finishProcessing();
	return PortageLoaderBase::NoError;
}

/**
 * Scan the mainline tree for packages.
 * @return  The result of the internal scanTree() call.
 */
PortageLoaderBase::Error PortageTreeScanner::scanMainlineTree()
{
	QDateTime startTime = QDateTime::currentDateTime();
	localTreeCount = 0;
	Error result = scanTree(portageTreeDir, PortageTree::Mainline);
	if( result != AbortedError ) {
		postPartlyCompleteEvent( startTime, PortageTree::Mainline );
	}
	return result;
}

/**
 * Scan the overlay trees for packages.
 * @return  PortageLoaderBase::AbortedError if the thread was aborted,
 *          PortageLoaderBase::NoError otherwise.
 */
PortageLoaderBase::Error PortageTreeScanner::scanOverlayTrees()
{
	QDateTime startTime = QDateTime::currentDateTime();
	localTreeCount = 0;

	for( QStringList::iterator overlayIterator = portageOverlayDirs.begin();
	     overlayIterator != portageOverlayDirs.end(); overlayIterator++ )
	{
		Error result = scanTree( *overlayIterator, PortageTree::Overlay );

		if( result != AbortedError )
			postPartlyCompleteEvent( startTime, PortageTree::Overlay );
		else
			return AbortedError;
	}
	return NoError;
}

/**
 * Scan the installed packages tree for packages.
 * @return  The result of the internal scanTree() call.
 */
PortageLoaderBase::Error PortageTreeScanner::scanInstalledTree()
{
	QDateTime startTime = QDateTime::currentDateTime();
	localTreeCount = 0;
	Error result = scanTree( installedPackagesDir, PortageTree::Installed );
	if( result != AbortedError ) {
		postPartlyCompleteEvent( startTime, PortageTree::Installed );
	}
	return result;
}


/**
 * Post a LoadingTreePartiallyCompleteEvent to the event receiver.
 * This is called every time when scanning for one of the search trees
 * has been completed.
 *
 * @param since  The time when searching started.
 *               This is used to calculate the elapsed time.
 * @param searchedTrees  The directory that has been searched. Must be one
 *                       of PortageTree::Mainline, PortageTree::Overlay
 *                       or PortageTree::Installed.
 */
void PortageTreeScanner::postPartlyCompleteEvent(
	QDateTime& since, PortageTree::Trees searchedTree )
{
	// only post if running as thread
	if( this->running() == true )
	{
		LoadingTreePartiallyCompleteEvent *event =
			new LoadingTreePartiallyCompleteEvent();

		event->packageCount = localTreeCount;
		event->secondsElapsed = since.secsTo( QDateTime::currentDateTime() );
		event->searchedTree = searchedTree;
		QApplication::postEvent( receiver, event );
	}
}


/**
 * Scan a search directory for packages. The searchedTree parameter defines
 * which ones of the tree directories is searched. This function assumes
 * that tree is a valid PortageTree object (especially not NULL).
 *
 * @param treeDir  The search directory containing package information
 * @param searchedTree  Defines which directory should be searched. This is
 *                      one of PortageTree::Mainline,
 *                      PortageTree::Overlay or
 *                      PortageTree::Installed.
 * @return PortageLoaderBase::NoError if the tree was scanned successful.
 *         PortageLoaderBase::AbortedError if the thread was aborted
 *         by setting this->stop to true.
 *         PortageLoaderBase::AlreadyRunningError if any thread is
 *         currently scanning (but there may only be one at a time).
 *         PortageLoaderBase::NullObjectError if the tree directory was
 *         either "" or could not be entered.
 */
PortageLoaderBase::Error PortageTreeScanner::scanTree(
	QString treeDir, PortageTree::Trees searchedTree
)
{
	QDir d;
	LoadingTreeProgressEvent* progressEvent;
	int pos;

	// set the d directory to the treeDir string
	if( treeDir.isNull() || treeDir.isEmpty() || !d.cd(treeDir) ) {
		return NullObjectError;
	}

	if( searchedTree == PortageTree::Installed ) {
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
	QStringList::Iterator categoryIteratorEnd = categories.end();
	for ( QStringList::Iterator categoryIterator = categories.begin();
	      categoryIterator != categoryIteratorEnd; ++categoryIterator )
	{
		pos = (*categoryIterator).find('-', 1);

		// don't process unwanted directories
		if( pos == -1 ) { // doesn't contain '-', so it's a non-package dir
			continue;
		}

		// Extract the category and subcategory from the folder name
		currentCategory  = (*categoryIterator).left(pos);
		currentSubcategory = (*categoryIterator).mid(pos+1);

		// If the edb database is searched, do this
		if( (searchedTree == PortageTree::Mainline) && (preferEdb == true) )
		{
			// Compose the folder name of the current category
			if( !d.cd(edbDir + portageTreeDir + "/" + (*categoryIterator)) )
				continue;

			scanEdbCategory(d);

			if( this->stop == true )
				return AbortedError;
			else
				continue;
		}
		// If the normal portage tree is searched, do that
		else
		{
			// Compose the folder name of the current category
			if( !d.cd(treeDir + "/" + (*categoryIterator)) ) {
				continue;
			}

			// Iterate through the available package dirs in the current category
			QStringList packages = d.entryList();
			QStringList::Iterator packageIteratorEnd = packages.end();
			for ( QStringList::Iterator packageIterator = packages.begin();
				packageIterator != packageIteratorEnd; ++packageIterator )
			{
				// no "." or ".." directories
				if( (*packageIterator)[0] == '.' )
					continue;

				// Compose the folder name of the current package and cd there
				if ( !d.cd( treeDir + "/" +
							(*categoryIterator) + "/" +
							(*packageIterator)) )
				{
					continue;
				}

				if( searchedTree == PortageTree::Mainline )
				{
					currentPackage = tree->package(
						currentCategory, currentSubcategory, *packageIterator
					);
					scanTreePackage( d, false );
				}
				else if( searchedTree == PortageTree::Overlay )
				{
					currentPackage = tree->package(
						currentCategory, currentSubcategory, *packageIterator
					);
					scanTreePackage( d, true );
				}
				else // if( searchedTree == PortageTree::Installed )
				{
					scanInstalledPackage( d );
					installedPackageCount++;
				}

				if( this->running() )
				{
					// increase package counter, and notify the event receiver
					if ( (++localTreeCount % 20) == 0 ) {
						progressEvent = new LoadingTreeProgressEvent();
						progressEvent->packageCount = localTreeCount;
						progressEvent->searchedTree = searchedTree;
						progressEvent->method = LoadingTreeEvent::ScanPortageTree;
						QApplication::postEvent( receiver, progressEvent );
					}

					if( this->stop == true ) {
						return AbortedError;
					}
				}
			} // end of package iteration
		} // end of if( mainline/edb )
	} // end of category iteration
	return NoError;
} // end of scanPortageTree()


/**
 * Iterate through a directory's ebuild files and add the found
 * package versions to the currentPackage object.
 *
 * @param d        The directory containing the ebuilds.
 * @param overlay  The value for version->overlay
 *                 (set true if it's an overlay directory)
 */
void PortageTreeScanner::scanTreePackage( QDir& d, bool overlay )
{
	// Iterate through all ebuild files of the current currentPackage
	QStringList ebuilds = d.entryList( "*.ebuild" );
	QStringList::Iterator ebuildIteratorEnd = ebuilds.end();

	for ( QStringList::Iterator ebuildIterator = ebuilds.begin();
	      ebuildIterator != ebuildIteratorEnd; ++ebuildIterator )
	{
		// add version info
		currentVersion = currentPackage->version(
			(*ebuildIterator).mid( // extract the package version string
				(currentPackage->name).length() + 1,
				(*ebuildIterator).length() - 7 - ((currentPackage->name).length() + 1)
			)
		);
		currentVersion->overlay = overlay;
	}
} // end of scanTreePackage()

/**
 * Search a category in edb/dep/ and add the found packages and
 * package versions the tree's package list.
 *
 * @param d  The category directory containing the package files.
 */
void PortageTreeScanner::scanEdbCategory( QDir& d )
{
	QString packageName;
	LoadingTreeProgressEvent* progressEvent;

	// Iterate through all ebuild files of the current currentPackage
	QStringList files = d.entryList();
	QStringList::Iterator fileIteratorEnd = files.end();

	for ( QStringList::Iterator fileIterator = files.begin();
	      fileIterator != fileIteratorEnd; ++fileIterator )
	{
		// no "." or ".." directories
		if( (*fileIterator)[0] == '.' )
			continue;


		int packageNameEndIndex = (*fileIterator).findRev( rxVersion );
		packageName = (*fileIterator).left( packageNameEndIndex );

		// See if it's a new package (if not, it's just another version)
		if( (currentPackage == NULL) || (packageName != currentPackage->name) )
		{
			// Separate package name from version
			currentPackage = tree->package(
				currentCategory, currentSubcategory, packageName
			);

			// in case we're running as thread, inform the receiver
			if( this->running() )
			{
				if ( (++localTreeCount % 20) == 0 ) {
					progressEvent = new LoadingTreeProgressEvent();
					progressEvent->packageCount = localTreeCount;
					progressEvent->searchedTree = PortageTree::Mainline;
					progressEvent->method = LoadingTreeEvent::ScanPortageTree;
					QApplication::postEvent( receiver, progressEvent );
				}
			}
		}

		// extract the package version string, and add version info
		currentVersion = currentPackage->version(
			(*fileIterator).mid( (currentPackage->name).length() + 1 )
		);
		currentVersion->overlay = false;
	}
} // end of scanEdbCategory()

/**
 * Extract package name, version, and modification date from a directory name,
 * and add a corresponding package to the tree's package list.
 *
 * @param d  The directory named after the (installed) package.
 */
void PortageTreeScanner::scanInstalledPackage( QDir& d )
{
	QString dirName = d.dirName();
	int packageNameEndIndex = dirName.findRev( rxVersion );

	// Separate package name from version
	currentPackage = tree->package(
		currentCategory, currentSubcategory,
		dirName.left( packageNameEndIndex ) // package name
	);
	currentVersion = currentPackage->version(
		dirName.mid( packageNameEndIndex + 1 )
	);
	currentVersion->installed = true;
}


/**
 * Start a thread that will be loading a portage tree by scanning portage
 * for packages. While loading packages, the thread will post
 * LoadingTreeProgressEvent objects to the receiver, and after each scanned
 * tree there will be a LoadingTreePartiallyCompleteEvent. When loading is
 * done, a LoadingTreeCompleteEvent will be posted and the thread exits.
 * The thread will not be started if it's already running.
 *
 * @param receiver     A QObject that receives loading status events.
 * @param portageTree  A pointer to an existing PortageTree object.
 *                     This tree will be cleared and filled with the
 *                     packages that are found in portage.
 * @param searchedTrees  Which ones of the trees will be searched.
 *
 * @returns  true if the thread was started, false if it's already running.
 */
bool PortageTreeScanner::startScanningTrees( QObject* receiver,
                                           PortageTree* portageTree,
                                           PortageTree::Trees searchedTrees,
                                           bool preferEdb )
{
	if( this->running() == true )
		return false;

	this->receiver = receiver;
	this->tree = portageTree;
	this->searchedTrees = searchedTrees;
	this->preferEdb = preferEdb;
	this->start();
	return true;
}

/**
 * The function that is called when a new thread is started.
 * It can not be called directly with portageScannerObject->start().
 * That's not necessary, because PortageTreeScanner has a convenient
 * member function to start the thread and set up the configuration
 * for it (which is startScanningPortage()).
 */
void PortageTreeScanner::run()
{
	PortageLoaderBase::Error result;
	QDateTime startTime = QDateTime::currentDateTime();
	QObject* receiver = this->receiver;

	result = scanTrees( tree, searchedTrees, preferEdb );

	// Inform main thread that scan is complete
	LoadingTreeCompleteEvent *event = new LoadingTreeCompleteEvent();
	event->error = result;
	event->packageCount = tree->packageCount();
	event->packageCountInstalled = installedPackageCount;
	event->method = LoadingTreeEvent::ScanPortageTree;
	event->secondsElapsed = startTime.secsTo( QDateTime::currentDateTime() );
	QApplication::postEvent( receiver, event );
}
