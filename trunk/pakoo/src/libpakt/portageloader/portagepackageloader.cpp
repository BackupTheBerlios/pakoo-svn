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

#include "../core/portagepackageversion.h"
#include "../core/portagepackage.h"
#include "../core/portagesettings.h"
#include "../core/portagecategory.h"

#include <qdir.h>
#include <qfileinfo.h>
#include <qdatetime.h>

#include <klocale.h>
#include <kdebug.h>

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
	m_rxDescription(RXDESCRIPTION), m_rxHomepage(RXHOMEPAGE),
	m_rxSlot(RXSLOT), m_rxLicenses(RXLICENSES),
	m_rxKeywords(RXKEYWORDS), m_rxUseflags(RXUSEFLAGS)
{
	m_settings = NULL;
}

/**
 * Set the PortageSettings object that contains directory and cache info.
 */
void PortagePackageLoader::setSettingsObject( PortageSettings* settings )
{
	m_settings = settings;
}


/**
 * The function that is called when a new thread is started.
 * It should be called using start() after the scanner configuration
 * has been set up, which is at least a call of setPackage().
 */
IJob::JobResult PortagePackageLoader::performThread()
{
	// load the settings
	if( m_settings == NULL ) {
		kdDebug() << i18n( "PortagePackageLoader debug output.",
			"Didn't start the profile loader because "
			"the settings object has not been set" )
			<< endl;
		return Failure;
	}
	else {
		//FIXME: Ugly hack below. But since CDB cache isn't implanted yet it works.
		m_preferCache = ( m_settings->preferredPackageSource() == FlatCache );
		m_mainlineTreeDir = m_settings->mainlineTreeDirectory();
		m_overlayTreeDirs = m_settings->overlayTreeDirectories();
		m_installedPackagesDir = m_settings->installedPackagesDirectory();
		m_cacheDir = m_settings->cacheDirectory();
	}

	if( package() == NULL )
	{
		// Happens if none of the set*Mode() functions have been called
		kdDebug() << i18n( "PortagePackageLoader debug output.",
			"PortagePackageLoader::performThread(): Didn't start scanning "
			"because you didn't specify a package to scan." )
			<< endl;
		return Failure;
	}


	QDateTime startTime = QDateTime::currentDateTime();

	/*
	kdDebug() << i18n( "PortagePackageLoader debug output. "
	                   "%1 is the package name.",
		"PortagePackageLoader::performThread(): "
		"Scanning the single package %1..." )
			.arg( package()->uniqueName() )
		<< endl;
	*/

	// scan the current package
	if( scanPackage() == false ) {
		return Failure;
	}
	else
	{
		// Success, inform main thread that the scan is complete
		/*
		kdDebug() << i18n( "PortagePackageLoader debug output.",
			"PortagePackageLoader::performThread(): "
			"Finished package scanning in %1 seconds." )
				.arg( startTime.secsTo(QDateTime::currentDateTime()) )
			<< endl;
		*/
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

	// Scan information for each package version
	for( Package::versioniterator versionIterator = package()->versionBegin();
	     versionIterator != package()->versionEnd(); versionIterator++ )
	{
		PortagePackageVersion* version =
			(PortagePackageVersion*) *versionIterator;

		if( version->hasDetailedInfo() == true )
			continue;  // version is already stuffed with info, so abort
		else
			version->setHasDetailedInfo( true );

		// construct the file name of the ebuild in the tree
		if( version->isOverlay() == false )
		{ // then it has an ebuild from the mainline tree

			// Get package size from the digest
			filename = m_mainlineTreeDir + "/"
				+ package()->category()->uniqueName() + "/"
				+ package()->name() + "/files/digest-" + package()->name()
				+ "-" + version->version();
			scanDigest( version, filename );

			if( m_preferCache == true )
			{
				filename = m_cacheDir + m_mainlineTreeDir + "/"
					+ package()->category()->uniqueName() + "/"
					+ package()->name() + "-" + version->version();

				// try to scan this edb file
				if( scanEdbFile( version, filename ) == true ) {
					continue; // no need to scan the installed package
				}
			}
			else
			{
				filename = m_mainlineTreeDir + "/"
					+ package()->category()->uniqueName() + "/"
					+ package()->name() + "/" + package()->name() + "-"
					+ version->version() + ".ebuild";

				// try to scan this ebuild
				if( scanEbuild( version, filename ) == true ) {
					continue; // no need to scan the installed package
				}
			}
		}
		else  // from the overlay tree
		{
			if( scanOverlayPackage( version ) == true )
				continue; // no need to scan the installed package
		}

		// no luck, try the installed version (the ebuild in /var/db/pkg/*/*/)
		if( version->isInstalled() == true )
		{
			filename = m_installedPackagesDir + "/"
				+ package()->category()->uniqueName() + "/"
				+ package()->name() + "-" + version->version() + "/"
				+ package()->name() + "-" + version->version()
				+ ".ebuild";

			scanEbuild( version, filename );
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
bool PortagePackageLoader::scanOverlayPackage( PortagePackageVersion* version )
{
	QString filename;
	for( QStringList::iterator overlayIterator = m_overlayTreeDirs.begin();
	     overlayIterator != m_overlayTreeDirs.end(); overlayIterator++ )
	{
		// Get package size from the digest
		filename = (*overlayIterator) + "/"
			+ package()->category()->uniqueName() + "/" + package()->name()
			+ "/files/digest-" + package()->name() + "-" + version->version();
		if( scanDigest( version, filename ) == false )
			continue;

		filename = (*overlayIterator) + "/"
			+ package()->category()->uniqueName() + "/" + package()->name()
			+ "/" + package()->name() + "-" + version->version() + ".ebuild";

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
bool PortagePackageLoader::scanEbuild( PortagePackageVersion* version,
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

		if( m_rxDescription.search(line) > -1 ) // found a description
		{
			// store the first (and, most probably, only) match
			version->setDescription( m_rxDescription.cap(1) );
			continue;
		}
		if( m_rxHomepage.search(line) > -1 ) // found a homepage string
		{
			version->setHomepage( m_rxHomepage.cap(1) );
			continue;
		}
		if( m_rxSlot.search(line) > -1 ) // found a slot string
		{
			version->setSlot( m_rxSlot.cap(1) );
			continue;
		}
		if( this->extractStringList( line, &m_rxLicenses,
		                             &(version->licenses()) ) == true )
		{
			continue;
		}
		if( this->extractStringList( line, &m_rxKeywords,
		                             &(version->keywords()) ) == true )
		{
			continue;
		}
		if( this->extractStringList( line, &m_rxUseflags,
		                             &(version->useflags()) ) == true )
		{
			continue;
		}
	}
	file.close();

	QFileInfo fileInfo( filename );
	QDateTime date = fileInfo.created();
	version->setDate( date.toString("yyyy MM dd") );

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
bool PortagePackageLoader::scanEdbFile( PortagePackageVersion* version,
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
			version->setSlot( line );
			break;
		case 4: // file location, starting with mirror://
			break;
		case 5: // empty?
			break;
		case 6: // home page
			version->setHomepage( line );
			break;
		case 7: // licenses
			version->setLicenses( QStringList::split(' ', line) );
			break;
		case 8: // description
			version->setDescription( line );
			break;
		case 9: // keywords
			version->setKeywords( QStringList::split(' ', line) );
			break;
		case 10: // inherited eclasses?
			break;
		case 11: // useflags
			version->setUseflags( QStringList::split(' ', line) );
			break;
		default:
			break;
		}
	}
	file.close();

	QFileInfo fileInfo(filename);
	QDateTime date = fileInfo.created();
	version->setDate( date.toString("yyyy MM dd") );

	return true;

} // end of scanEdbFile()


/**
 * Extract the package size from the so-called digest for a package version.
 *
 * @param version   The package version of the digest file.
 * @param filename  The path to the package's digest file.
 * @return  false if the file can't be opened, true otherwise.
 */
bool PortagePackageLoader::scanDigest( PortagePackageVersion* version,
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
		version->setSize( line.section(' ', -1).toLong() );
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
