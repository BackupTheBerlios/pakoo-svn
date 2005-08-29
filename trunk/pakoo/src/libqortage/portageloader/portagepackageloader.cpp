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

#include "portagepackageloader.h"

#include "../core/packageversion.h"
#include "../core/package.h"
#include "../core/portagesettings.h"
#include "../core/portagecategory.h"

#include <qdir.h>
#include <qfileinfo.h>
#include <qdatetime.h>

// QRegExp regular expression strings
#define RXDESCRIPTION "^DESCRIPTION=\"(.*)\"\\s*$"
#define RXHOMEPAGE       "^HOMEPAGE=\"(.*)\"\\s*$"
#define RXSLOT               "^SLOT=\"(.*)\"\\s*$"
#define RXLICENSES        "^LICENSE=\"(.*)\"\\s*$"
#define RXKEYWORDS       "^KEYWORDS=\"(.*)\"\\s*$"
#define RXUSEFLAGS           "^IUSE=\"(.*)\"\\s*$"


namespace libpakt {

/**
 * Initialize this object. You still have to set the PortageSettings
 * object containing directory and cache info.
 *
 * @see setSettingsObject
 */
PortagePackageLoader::PortagePackageLoader()
: PackageLoader(),
  rxDescription(RXDESCRIPTION), rxHomepage(RXHOMEPAGE),
  rxSlot(RXSLOT), rxLicenses(RXLICENSES),
  rxKeywords(RXKEYWORDS), rxUseflags(RXUSEFLAGS)
{
	settings = NULL;
}

/**
 * Set the PortageSettings object that contains directory and cache info.
 */
void PortagePackageLoader::setSettingsObject( PortageSettings* settings )
{
	this->settings = settings;
}


/**
 * The function that is called when a new thread is started.
 * It should be called using start() after the scanner configuration
 * has been set up, which is at least a call of setPackage().
 */
IJob::JobResult PortagePackageLoader::performThread()
{
	// load the settings
	if( settings == NULL ) {
		emit debugOutput(
			"Didn't start the profile loader because "
			"the settings object has not been set"
		);
		return Failure;
	}
	else {
		preferCache = settings->preferCache();
		mainlineTreeDir = settings->mainlineTreeDirectory();
		overlayTreeDirs = settings->overlayTreeDirectories();
		installedPackagesDir = settings->installedPackagesDirectory();
		cacheDir = settings->cacheDirectory();
	}

	if( package() == NULL )
	{
		// Happens if none of the set*Mode() functions have been called
		emitDebugOutput( "Didn't start scanning because "
		            "you didn't specify a package to scan." );
		return Failure;
	}


	QDateTime startTime = QDateTime::currentDateTime();

	emitDebugOutput(
		QString("Scanning the single package %1...")
		.arg( package()->uniqueName() )
	);

	// scan the current package
	if( scanPackage() == false )
	{
		// any errors errors emitted with debugOutput()
		return Failure;
	}
	else
	{
		// Success, inform main thread that the scan is complete
		emitDebugOutput(
			QString("Finished package scanning in %1 seconds.")
			.arg( startTime.secsTo(QDateTime::currentDateTime()) )
		);
		return Success;
	}
}

/**
 * Scan each package version's ebuild and digest files and
 * add the found information to the current package's PackageVersion objects.
 * This function assumes that the package member is a valid Package object,
 * and especially not NULL.
 */
bool PortagePackageLoader::scanPackage()
{
	QString filename;

	PackageVersionMap* versions = package()->versionMap();
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
			filename = this->mainlineTreeDir + "/"
				+ package()->category()->uniqueName() + "/"
				+ package()->name() + "/files/digest-" + package()->name()
				+ "-" + (*versionIterator).version;
			scanDigest( &(*versionIterator), filename );

			if( preferCache == true )
			{
				filename = this->cacheDir + this->mainlineTreeDir + "/"
					+ package()->category()->uniqueName() + "/"
					+ package()->name() + "-" + (*versionIterator).version;

				// try to scan this edb file
				if( scanEdbFile( &(*versionIterator), filename ) == true ) {
					continue; // no need to scan the installed package
				}
			}
			else
			{
				filename = this->mainlineTreeDir + "/"
					+ package()->category()->uniqueName() + "/"
					+ package()->name() + "/" + package()->name() + "-"
					+ (*versionIterator).version + ".ebuild";

				// try to scan this ebuild
				if( scanEbuild( &(*versionIterator), filename ) == true ) {
					continue; // no need to scan the installed package
				}
			}
		}
		else  // from the overlay tree
		{
			if( scanOverlayPackage( &(*versionIterator) ) == true )
				continue; // no need to scan the installed package
		}

		// no luck, try the installed version (the ebuild in /var/db/pkg/*/*/)
		if( (*versionIterator).installed == true )
		{
			filename = this->installedPackagesDir + "/"
				+ package()->category()->uniqueName() + "/"
				+ package()->name() + "-" + (*versionIterator).version + "/"
				+ package()->name() + "-" + (*versionIterator).version
				+ ".ebuild";

			scanEbuild( &(*versionIterator), filename );
		}
	}

	emitPackageLoaded();
	return true;
}

/**
 * Scan the ebuild and digest files for a specific version of a package
 * in one of the overlay tree. This function basically tries to call
 * scanDigest() and scanEbuild() for each possible overlay path
 * until it works.
 *
 * @param version  The package version of the ebuild and digest files.
 * @return  true if the files have been loaded, false otherwise.
 */
bool PortagePackageLoader::scanOverlayPackage( PackageVersion* version )
{
	QString filename;
	for( QStringList::iterator overlayIterator = overlayTreeDirs.begin();
	     overlayIterator != overlayTreeDirs.end(); overlayIterator++ )
	{
		// Get package size from the digest
		filename = (*overlayIterator) + "/"
			+ package()->category()->uniqueName() + "/" + package()->name()
			+ "/files/digest-" + package()->name() + "-" + version->version;
		if( scanDigest( version, filename ) == false )
			continue;

		filename = (*overlayIterator) + "/"
			+ package()->category()->uniqueName() + "/" + package()->name()
			+ "/" + package()->name() + "-" + version->version + ".ebuild";

		// try to scan this ebuild
		if( scanEbuild( version, filename ) == true )
			return true;
	}
	// none of the paths contains the package? well then:
	return false;
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
bool PortagePackageLoader::scanEbuild( PackageVersion* version,
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
bool PortagePackageLoader::scanEdbFile( PackageVersion* version,
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
bool PortagePackageLoader::scanDigest( PackageVersion* version,
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
		QString bla = line.section(' ', -1);
		version->size = bla.toLong();
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
bool PortagePackageLoader::extractStringList( const QString& string,
                                              QRegExp* rx,
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

} // namespace
