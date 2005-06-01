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

#include "package.h"

#include "packageversion.h"


/**
 * Initialize the package with name, category and subcategory.
 */
Package::Package( QString category, QString subcategory, QString name )
{
	this->category = category;
	this->subcategory = subcategory;
	this->name = name;
}

/**
 * Remove all versions from the package's version list.
 */
void Package::clear()
{
	versions.clear();
}

/**
 * Remove a version from the package's version list.
 *
 * @param versionString  The package version, e.g. "2.6.11-r6".
 */
void Package::removeVersion( QString versionString )
{
	versions.remove( versionString );
}

/**
 * Add an existing PackageVersion object to the tree. If a version with same
 * version string already exists, it is replaced.
 *
 * @return  true if the version has been added to the package. false if it
 *          has not been added because its version string was empty.
 */
bool Package::setVersion( PackageVersion& version )
{
	if( version.version == "" )
		return false;

	this->versions[ version.version ] = version;
	return true;
}

/**
 * Create a PackageVersion object and add it to the tree. If a version
 * with same version string already exists, it is replaced.
 *
 * @param versionString  The version string, e.g. "2.6.11-r6".
 * @return  true if the version has been added to the package. false if it
 *          has not been added because the version string was empty.
 */
bool Package::setVersion( QString versionString )
{
	PackageVersion version( versionString );
	return this->setVersion( version );
}

/**
 * Determine if this package contains any versions at all.
 *
 * @return  true if this package contains a version, false otherwise.
 */
bool Package::hasVersions()
{
	if( this->versions.count() > 0 )
		return true;
	else
		return false;
}

/**
 * Determine if this package contains a specific version.
 *
 * @param versionString  The requested package version string.
 * @return  true if this package contains the version, false otherwise.
 */
bool Package::hasVersion( QString versionString )
{
	if( this->versions.contains(versionString) )
		return true;
	else
		return false;
}

/**
 * Determine if this package contains a version that is
 * installed on this system.
 *
 * @return  true if this package contains the version, false otherwise.
 */
bool Package::hasInstalledVersion()
{
	QMap<QString, PackageVersion>::iterator versionIterator;

	for( versionIterator = this->versions.begin();
	     versionIterator != this->versions.end(); versionIterator++ )
	{
		if( (*versionIterator).installed == true ) {
			return true;
		}
	}
	return false;
}

/**
 * Return a PackageVersion struct for a given version string. If the struct
 * doesn't exist yet, it is created with (empty) default values.
 *
 * @param versionString  The requested package version string,
 *                       e.g. "2.6.11-r6".
 * @return  A pointer to the requested PackageVersion object.
 *          (If you change its values, they are also changed
 *          in this Package object.)
 */
PackageVersion* Package::version( QString versionString )
{
	PackageVersion* version = &(this->versions[versionString]);
	// if( version->version == "" ) {
	version->version = versionString;
	// }

	return version;
}

/**
 * Return a pointer of the PackageVersion object map.
 */
PackageVersionMap* Package::versionMap()
{
	return &(this->versions);
}


/**
 * Return a list of PackageVersion objects sorted by their version numbers,
 * with the oldest version at the beginning and the latest version at the end
 * of the list.
 */
QValueList<PackageVersion> Package::sortedVersionList()
{
	QValueList<PackageVersion> sortedVersions;
	QValueList<PackageVersion>::iterator sortedVersionIterator;
	QMap<QString, PackageVersion>::iterator versionIterator;

	for( versionIterator = this->versions.begin();
	     versionIterator != this->versions.end(); versionIterator++ )
	{
		if( versionIterator == this->versions.begin() ) {
			sortedVersions.append( *versionIterator );
			continue; // if there is only one version, it can't be compared
		}

		// reverse iteration through the sorted version list
		sortedVersionIterator = sortedVersions.end();
		while( true )
		{
			if( sortedVersionIterator == sortedVersions.begin() )
			{
				sortedVersions.prepend( (*versionIterator) );
				break;
			}

			sortedVersionIterator--;
			if( (*versionIterator).isNewerThan( (*sortedVersionIterator).version ) )
			{
				sortedVersionIterator++; // insert after the compared one, not before
				sortedVersions.insert( sortedVersionIterator, (*versionIterator) );
				break;
			}
		}
	}
	return sortedVersions;
} // end of sortedVersionList()

/**
 * Check if there is an update available for any version of the package.
 */
bool Package::hasUpdate( const QString& arch )
{
	PackageVersionMap::iterator versionIterator;

	for( versionIterator = this->versions.begin();
	     versionIterator != versions.end(); versionIterator++ )
	{
		if( (*versionIterator).installed == false )
			continue;

		// only installed versions are checked for an update
		if( this->hasUpdate( &(*versionIterator), arch ) == true )
			return true;
	}
	// if the loop hasn't already returned true, there are no updates
	return false;
}

/**
 * Check if there is an update available if a given version is installed.
 */
bool Package::hasUpdate( PackageVersion* version, const QString& arch )
{
	QValueList<PackageVersion> sortedVersions = this->sortedVersionList();
	QValueList<PackageVersion>::iterator versionIterator;
	bool check = false;

	for( versionIterator = sortedVersions.begin();
	     versionIterator != sortedVersions.end(); versionIterator++ )
	{
		if( check == false && (*versionIterator).version != version->version ) {
			continue;
		}
		else if( check == true )
		{
			// If we get here, the versionIterator is newer
			// than the given version.

			if( (*versionIterator).installed == true ) {
				continue; // if it's installed, it's not upgradable. next one.
			}
			if( version->slot == (*versionIterator).slot &&
			    (*versionIterator).stability(arch) == PackageVersion::Stable )
			{
				return true;
			}
		}
		else // if( (*versionIterator).version == versionString )
		{
			check = true;
		}
	}
	// if the loop hasn't already returned true, there are no updates
	return false;
} // end of hasUpdate()
