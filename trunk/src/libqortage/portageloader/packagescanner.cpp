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

#include "packagescanner.h"

#include "../core/packageversion.h"
#include "../core/package.h"
#include "../core/portagetree.h"
#include "loadingevent.h"

#include <qfileinfo.h>
#include <qdatetime.h>
#include <qapplication.h>

// QRegExp regular expression strings
#define RXDESCRIPTION "^DESCRIPTION=\"(.*)\"\\s*$"
#define RXHOMEPAGE       "^HOMEPAGE=\"(.*)\"\\s*$"
#define RXSLOT               "^SLOT=\"(.*)\"\\s*$"
#define RXLICENSES        "^LICENSE=\"(.*)\"\\s*$"
#define RXKEYWORDS       "^KEYWORDS=\"(.*)\"\\s*$"
#define RXUSEFLAGS           "^IUSE=\"(.*)\"\\s*$"


/**
 * Initialize this object, defining where to search for packages.
 */
PackageScanner::PackageScanner( QString treeDir, QString overlayDir,
                                QString installedDir, QString edbDepDir )
: PortageLoaderBase(),
  portageTreeDir(treeDir), portageOverlayDir(overlayDir),
  installedPackagesDir(installedDir), edbDir(edbDepDir),
  rxDescription(RXDESCRIPTION), rxHomepage(RXHOMEPAGE),
  rxSlot(RXSLOT), rxLicenses(RXLICENSES),
  rxKeywords(RXKEYWORDS), rxUseflags(RXUSEFLAGS)
{
	package = NULL;
	preferEdb = true;
	doFilterInstalled = false;
	scanInstalled = false;
}

/**
 * Initialize this object, copying the configuration from another
 * PackageScanner object.
 */
PackageScanner::PackageScanner( PackageScanner* anotherScanner )
: PortageLoaderBase(),
  rxDescription(RXDESCRIPTION), rxHomepage(RXHOMEPAGE),
  rxSlot(RXSLOT), rxLicenses(RXLICENSES),
  rxKeywords(RXKEYWORDS), rxUseflags(RXUSEFLAGS)
{
	package = NULL;
	if( anotherScanner != NULL ) {
		portageTreeDir = anotherScanner->portageTreeDir;
		portageOverlayDir = anotherScanner->portageOverlayDir;
		installedPackagesDir = anotherScanner->installedPackagesDir;
		edbDir = anotherScanner->edbDir;
		preferEdb = anotherScanner->preferEdb;
		doFilterInstalled = anotherScanner->doFilterInstalled;
		scanInstalled = anotherScanner->scanInstalled;
	}
}

/**
 * Enable or disable filtering installed packages out or in.
 * Filtering is disabled by default.
 */
void PackageScanner::setFilterInstalled( bool doFilter, bool scanInstalled )
{
	this->doFilterInstalled = doFilter;
	this->scanInstalled = scanInstalled;
}

/**
 * Set the portage tree search directory to a new value.
 */
void PackageScanner::setPortageTreeDirectory( QString directory )
{
	portageTreeDir = directory;
}

/**
 * Set the portage cache directory to a new value.
 */
void PackageScanner::setEdbDepDirectory( QString directory )
{
	edbDir = directory;
}

/**
 * Set the portage overlay directory to a new value.
 */
void PackageScanner::setOverlayDirectory( QString directory )
{
	portageOverlayDir = directory;
}

/**
 * Set the installed packages database directory to a new value.
 */
void PackageScanner::setInstalledPackagesDirectory( QString directory )
{
	installedPackagesDir = directory;
}

/**
 * Perform scanPackage on all packages of the given tree that match
 * the category and subcategory filters.
 *
 * @param tree  The PortageTree object containing the packages.
 * @param category  The searched category. QString::null means
 *                  that there is no category filtering.
 * @param subcategory  The searched subcategory. QString::null means
 *                     that there is no subcategory filtering.
 * @param preferEdb  If true, the edb/dep/ directory will be searched instead
 *                   of the mainline tree, which will probably be much faster.
 *
 * @return  PortageLoaderBase::NoError if scanning was done.
 *          PortageLoaderBase::NullTreeError if the tree is NULL.
 *          PortageLoaderBase::AbortedError if the thread was aborted.
 */
PortageLoaderBase::Error PackageScanner::scanCategory(
	PortageTree* tree, QString category, QString subcategory, bool preferEdb )
{
	if( tree == NULL ) {
		return NullTreeError;
	}

	PortageLoaderBase::Error result;

	PackageMap* packages = tree->packageMap();
	PackageMap::iterator packageIteratorEnd = packages->end();

	if( category.isNull() ) // no category filter at all, process all
	{
		// Iterate through all packages
		for( PackageMap::iterator packageIterator = packages->begin();
		     packageIterator != packageIteratorEnd; ++packageIterator )
		{
			if( this->stop == true )
				return AbortedError;

			result = scanPackage( &(*packageIterator), preferEdb );

			// Inform main thread , but only when running as thread
			if( this->running() == true )
			{
				LoadingPackageCompleteEvent *event = new LoadingPackageCompleteEvent();
				event->error = result;
				event->action = ScanPackage;
				event->package = &(*packageIterator);
				event->packageScanner = this;
				QApplication::postEvent( receiver, event );
			}
		}
	}
	else // at least the main-category filter is set
	{
		// Iterate through all packages
		for( PackageMap::iterator packageIterator = packages->begin();
		     packageIterator != packageIteratorEnd; ++packageIterator )
		{
			// Only process categories that equal the given filter category.
			if( category != (*packageIterator).category )
				continue;

			// Only process packages if no subcategory filter is set,
			// or if the current package's subcategory equals the given one.
			if( subcategory.isNull()
			    || (*packageIterator).subcategory == subcategory )
			{
				if( this->stop == true )
					return AbortedError;

				result = scanPackage( &(*packageIterator), preferEdb );

				// Inform main thread , but only when running as thread
				if( this->running() == true )
				{
					LoadingPackageCompleteEvent *event = new LoadingPackageCompleteEvent();
					event->error = result;
					event->action = ScanPackage;
					event->package = &(*packageIterator);
					event->packageScanner = this;
					QApplication::postEvent( receiver, event );
				}
			}
		}
	}

	return NoError;
}


/**
 * Scan the package version's ebuild and digest files and
 * add the found information to the PackageVersion objects.
 *
 * @param package  The package that will be filled with detailed info.
 * @param preferEdb  If true, the edb/dep/ directory will be searched instead
 *                   of the mainline tree, which will probably be much faster.
 *
 * @return  PortageLoaderBase::NoError if scanning was done.
 *          PortageLoaderBase::AbortedError if only installed packages
 *          are scanned (when the filter is enabled) and this package
 *          doesn't have any installed versions.
 *          PortageLoaderBase::NullTreeError if the package is NULL.
 */
PortageLoaderBase::Error PackageScanner::scanPackage(
	Package* package, bool preferEdb )
{
	if( package == NULL ) {
		return NullTreeError;
	}

	// filter installed packages out or in, if enabled
	if( this->doFilterInstalled == true)
	{
		if( this->scanInstalled && package->hasInstalledVersion() == false )
			return AbortedError;
		else if( !this->scanInstalled && package->hasInstalledVersion() == true )
			return AbortedError;
	}

	QFile file;
	QString filename;

	PackageVersionMap* versions = package->versionMap();
	PackageVersionMap::iterator versionIterator;

	// Scan information for each package version
	for( versionIterator = versions->begin();
	     versionIterator != versions->end(); versionIterator++ )
	{
		if( (*versionIterator).hasDetailedInfo == true )
			continue;  // version is already stuffed with info, so abort
		else
			(*versionIterator).hasDetailedInfo = true;

		// construct the file name of the ebuild in the tree
		if( (*versionIterator).overlay == false ) // from the mainline tree
		{
			// Get package size from the digest
			filename = this->portageTreeDir + "/" + package->category + "-"
				+ package->subcategory + "/" + package->name
				+ "/files/digest-" + package->name + "-"
				+ (*versionIterator).version;
			scanDigest( &(*versionIterator), filename );

			if( preferEdb == true )
			{
				filename = this->edbDir + this->portageTreeDir + "/"
					+ package->category + "-" + package->subcategory + "/"
					+ package->name + "-" + (*versionIterator).version;

				// try to scan this edb file
				if( scanEdbFile( &(*versionIterator), filename ) == true ) {
					continue; // no need to scan the installed package
				}
			}
			else
			{
				filename = this->portageTreeDir + "/" + package->category
					+ "-" + package->subcategory + "/" + package->name + "/"
					+ package->name + "-" + (*versionIterator).version
					+ ".ebuild";

				// try to scan this ebuild
				if( scanEbuild( &(*versionIterator), filename ) == true ) {
					continue; // no need to scan the installed package
				}
			}
		}
		else  // from the overlay tree
		{
			// Get package size from the digest
			filename = this->portageOverlayDir + "/" + package->category
				+ "-" + package->subcategory + "/" + package->name
				+ "/files/digest-" + package->name + "-"
				+ (*versionIterator).version;
			scanDigest( &(*versionIterator), filename );

			filename = this->portageOverlayDir + "/" + package->category
				+ "-" + package->subcategory + "/" + package->name + "/"
				+ package->name + "-" + (*versionIterator).version
				+ ".ebuild";

			// try to scan this ebuild
			if( scanEbuild( &(*versionIterator), filename ) == true ) {
				continue; // no need to scan the installed package
			}
		}

		// no luck, try the installed version (the ebuild in /var/db/pkg/*/*/)
		if( (*versionIterator).installed == true )
		{
			filename = this->installedPackagesDir + "/" + package->category
				+ "-" + package->subcategory + "/" + package->name + "-"
				+ (*versionIterator).version + "/" + package->name + "-"
				+ (*versionIterator).version + ".ebuild";

			scanEbuild( &(*versionIterator), filename );
		}
	}

	return NoError;
}

/**
 * Extract package info from an ebuild file and store it into an existing
 * package version info. Description, homepage, package slot, keywords,
 * licenses and useflags are extracted. This is most likely to be slower than
 * and not as correct as scanEdbFile(), which fulfills the same purpose.
 *
 * @param version   The package version of the ebuild file.
 * @param filename  The path to the package's ebuild file.
 * @return  false if the file can't be opened, true otherwise.
 */
bool PackageScanner::scanEbuild( PackageVersion* version,
                                 const QString& filename )
{
	QFile file(filename);

	if( !file.open(IO_ReadOnly) ) {
		return false;
	}

	QString line;
	QTextStream stream( &file );

	// Read out the package info strings
	while ( !stream.atEnd() )
	{
		line = stream.readLine();

		if( rxDescription.search(line) > -1 ) // found a description
		{
			// store the first (and, most probably, only) match
			version->description = rxDescription.cap( 1 );
			continue;
		}
		if( rxHomepage.search(line) > -1 ) // found a homepage string
		{
			version->homepage = rxHomepage.cap( 1 );
			continue;
		}
		if( rxSlot.search(line) > -1 ) // found a slot string
		{
			version->slot = rxSlot.cap( 1 );
			continue;
		}
		if( this->extractStringList( line, &rxLicenses, &(version->licenses) )
			== true )
		{
			continue;
		}
		if( this->extractStringList( line, &rxKeywords, &(version->keywords) )
			== true )
		{
			continue;
		}
		if( this->extractStringList( line, &rxUseflags, &(version->useflags) )
			== true )
		{
			continue;
		}
	}
	file.close();

	QFileInfo fileInfo(filename);
	QDateTime date = fileInfo.created();
	version->date = date.toString("yyyy MM dd");

	return true;

} // end of scanEbuild()

/**
 * Extract package info from a file in the portage cache (/var/cache/edb/dep)
 * and store it into an existing package version info. Description, homepage,
 * package slot, keywords, licenses and useflags are extracted.
 *
 * @param version   The package version of the edb file.
 * @param filename  The path to the package's edb file.
 * @return  false if the file can't be opened, true otherwise.
 */
bool PackageScanner::scanEdbFile( PackageVersion* version,
                                  const QString& filename )
{
	QFile file(filename);

	if( !file.open(IO_ReadOnly) ) {
		return false;
	}

	QString line;
	QTextStream stream( &file );
	int lineNumber = 0;

	// Read out the package info strings
	while ( !stream.atEnd() )
	{
		line = stream.readLine();
		lineNumber++;

		// each line has a fixed meaning, as it seems.
		// so iterate through the lines and quick-get the info outta there.
		switch( lineNumber )
		{
		case 1: // some dependency stuff
			break;
		case 2: // some other dependency stuff
			break;
		case 3: // the package slot
			version->slot = line;
			break;
		case 4: // file location, starting with mirror://
			break;
		case 5: // empty?
			break;
		case 6: // home page
			version->homepage = line;
			break;
		case 7: // licenses
			version->licenses = QStringList::split( ' ', line );
			break;
		case 8: // description
			version->description = line;
			break;
		case 9: // keywords
			version->keywords = QStringList::split( ' ', line );
			break;
		case 10: // inherited eclasses?
			break;
		case 11: // useflags
			version->useflags = QStringList::split( ' ', line );
			break;
		default:
			break;
		}
	}
	file.close();

	QFileInfo fileInfo(filename);
	QDateTime date = fileInfo.created();
	version->date = date.toString("yyyy MM dd");

	return true;

} // end of scanEdbFile()


/**
 * Extract the package size from the so-called digest for a package version.
 *
 * @param version   The package version of the digest file.
 * @param filename  The path to the package's digest file.
 * @return  false if the file can't be opened, true otherwise.
 */
bool PackageScanner::scanDigest( PackageVersion* version,
                                 const QString& filename )
{
	QFile file(filename);

	if( !file.open(IO_ReadOnly) ) {
		return false;
	}

	QString line;
	QTextStream stream( &file );

	while ( !stream.atEnd() ) {
		line = stream.readLine();
		version->size = line.section(' ', -1).toLong();
	}
	file.close();

	return true;
}


/**
 * Split the first match of a given regexp into single strings,
 * using spaces as delimiter.
 *
 * @param string      The string that will be searched.
 * @param rx          The regular expression used for searching.
 * @param targetList  The QStringList that will be filled with matches.
 * @return  false if rx or targetList is NULL, true otherwise.
 */
bool PackageScanner::extractStringList( const QString& string, QRegExp* rx,
                                        QStringList* targetList )
{
	if( rx == NULL || targetList == NULL )
		return false;

	if( rx->exactMatch( string ) == false ) { // no match
		return false;
	}
	else
	{
		*targetList = QStringList::split( ' ', rx->cap(1) );
	}
	return true;
}


/**
 * Start a thread that will be scanning the specified package for detailed
 * info. When loading is done, a LoadingTreeCompleteEvent will be posted
 * and the thread exits. The thread will not be started if it's already running.
 *
 * @param receiver  A QObject that receives loading status events.
 * @param package   The package that will be filled with detailed info.
 * @param preferEdb  If true, the edb/dep/ directory will be searched instead
 *                   of the mainline tree, which will probably be much faster.
 *
 * @returns  true if the thread was started, false if it's already running.
 */
bool PackageScanner::startScanningPackage( QObject* receiver,
                                           Package* package, bool preferEdb )
{
	if( this->running() == true )
		return false;

	this->action = ScanPackage;
	this->receiver = receiver;
	this->package = package;
	this->preferEdb = preferEdb;
	this->start();
	return true;
}

/**
 * Start a thread that will be scanning all packages in the specified category
 * for detailed info. After each scanned package, a LoadingTreeCompleteEvent
 * will be posted and the thread exits. The thread will not be started
 * if it's already running.
 *
 * @param receiver  A QObject that receives loading status events.
 * @param tree      The PortageTree object containing the packages.
 * @param category  The searched category. QString::null means
 *                  that there is no category filtering.
 * @param subcategory  The searched subcategory. QString::null means
 *                     that there is no subcategory filtering.
 * @param preferEdb  If true, the edb/dep/ directory will be searched instead
 *                   of the mainline tree, which will probably be much faster.
 *
 * @returns  true if the thread was started, false if it's already running.
 */
bool PackageScanner::startScanningCategory( QObject* receiver,
	PortageTree* tree, QString category, QString subcategory, bool preferEdb )
{
	if( this->running() == true )
		return false;

	this->action = ScanCategory;
	this->receiver = receiver;
	this->tree = tree;
	this->category = category;
	this->subcategory = subcategory;
	this->preferEdb = preferEdb;
	this->start();
	return true;
}


/**
 * The function that is called when a new thread is started.
 * It can not be called directly with packageScanner->start().
 * That's not necessary, because PackageScanner has convenient
 * member functions to start the thread and set up the configuration
 * for it (which is startScanningPackage()).
 */
void PackageScanner::run()
{
	PortageLoaderBase::Error result;
	QObject* receiver = this->receiver;

	if( action == ScanPackage )
	{
		if( working == true )
			return;
		else
			initProcessing();

		result = scanPackage( this->package, preferEdb );
		finishProcessing();

		// Inform main thread that scan is complete
		LoadingPackageCompleteEvent *event = new LoadingPackageCompleteEvent();
		event->error = result;
		event->action = ScanPackage;
		event->package = package;
		event->packageScanner = this;
		QApplication::postEvent( receiver, event );
	}
	else if( action == ScanCategory )
	{
		if( working == true )
			return;
		else
			initProcessing();

		result = scanCategory( tree, category, subcategory, preferEdb );
		finishProcessing();

		LoadingPackageCompleteEvent *event = new LoadingPackageCompleteEvent();
		event->error = result;
		event->action = ScanCategory;
		event->package = NULL;
		event->packageScanner = this;
		QApplication::postEvent( receiver, event );
	}
}
