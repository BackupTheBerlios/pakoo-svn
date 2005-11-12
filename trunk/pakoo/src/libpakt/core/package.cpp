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


namespace libpakt {

/**
 * Initialize the package with name and category.
 * Note that you mustn't delete the given category object,
 * because it is now considered to be owned by this Package object.
 */
Package::Package( PackageCategory* category, const QString& name )
	: m_name(name)
{
	if( category == NULL )
		m_category = new PackageCategory();
	else
		m_category = category;
}

/**
 * Destructor. Deletes the category object.
 */
Package::~Package()
{
	delete m_category;
}

/**
 * Get the short name of the package, not including the category name.
 * It doesn't have to be unique. (For a unique name of the package, including
 * the category name, rather use uniqueName().)
 * For Portage, this will be something like "pakoo".
 */
const QString& Package::name() const
{
	return m_name;
}

/**
 * Get the category of this package.
 */
PackageCategory* Package::category()
{
	return m_category;
}

/**
 * Get the full, unique name of the package.
 * For Portage, this will be something like "app-portage/pakoo".
 */
QString Package::uniqueName() const
{
	return m_category->uniqueName() + "/" + m_name;
}

/**
 * Remove all versions from the package's version list.
 */
void Package::clear()
{
	m_versions.clear();
}

/**
 * Remove a version from the package's version list.
 *
 * @param versionString  The package version, e.g. "2.6.11-r6".
 */
void Package::removeVersion( const QString& versionString )
{
	PackageVersionMap::iterator iterator = m_versions.find( versionString );
	if( iterator == m_versions.end() )
		return;
	else
		delete *iterator;

	m_versions.remove( versionString );
}

/**
 * Create a PackageVersion object and add it to the tree. If a version
 * with same version string already exists, it is replaced.
 * No version is created if the version string is empty.
 *
 * @param versionString  The version string, e.g. "2.6.11-r6".
 * @return  A pointer to the new PackageVersion object.
 *          NULL if the version string was empty and the object
 *          has not been created.
 */
PackageVersion* Package::insertVersion( const QString& versionString )
{
	if( versionString.isEmpty() )
		return NULL;

	PackageVersion* version = createPackageVersion( versionString );
	PackageVersionMap::iterator versionIterator =
		m_versions.insert( version->version(), version );

	if( versionIterator == m_versions.end() )
		return NULL; // could not be inserted into m_versions
	else
		return *versionIterator;
}

/**
 * Determine if this package contains any versions at all.
 *
 * @return  true if this package contains a version, false otherwise.
 */
bool Package::containsVersions()
{
	if( m_versions.count() > 0 )
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
bool Package::containsVersion( const QString& versionString )
{
	if( m_versions.contains(versionString) )
		return true;
	else
		return false;
}

/**
 * Returns true if this package contains a version that is
 * installed on this system, false otherwise.
 */
bool Package::containsInstalledVersion()
{
	PackageVersionMap::iterator versionIterator;

	for( versionIterator = m_versions.begin();
	     versionIterator != m_versions.end(); versionIterator++ )
	{
		if( (*versionIterator)->isInstalled() == true ) {
			return true;
		}
	}
	return false;
}

/**
 * Returns true if this package contains a version that can be
 * installed on this system, false otherwise.
 *
 * @return  true if this package contains the version, false otherwise.
 */
bool Package::containsAvailableVersion()
{
	PackageVersionMap::iterator versionIterator;

	for( versionIterator = m_versions.begin();
			versionIterator != m_versions.end(); versionIterator++ )
	{
		if( (*versionIterator)->isAvailable() == true ) {
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
PackageVersion* Package::version( const QString& versionString )
{
	PackageVersionMap::iterator versionIterator =
		m_versions.find( versionString );
	if( versionIterator == m_versions.end() ) {
		// return the newly created version pointer
		return insertVersion( versionString );
	}
	else {
		// return the pointer that's already in the map
		return *versionIterator;
	}
}

/**
 * Return a list of PackageVersion objects sorted by their version numbers,
 * with the oldest version at the beginning and the latest version at the end
 * of the list.
 */
QValueList<PackageVersion*> Package::sortedVersionList()
{
	QValueList<PackageVersion*> sortedVersions;
	QValueList<PackageVersion*>::iterator sortedVersionIterator;
	PackageVersionMap::iterator versionIterator;

	for( versionIterator = m_versions.begin();
	     versionIterator != m_versions.end(); versionIterator++ )
	{
		if( versionIterator == m_versions.begin() ) {
			sortedVersions.append( *versionIterator );
			continue; // if there is only one version, it can't be compared
		}

		// reverse iteration through the sorted version list
		sortedVersionIterator = sortedVersions.end();
		while( true )
		{
			if( sortedVersionIterator == sortedVersions.begin() )
			{
				sortedVersions.prepend( *versionIterator );
				break;
			}

			sortedVersionIterator--;
			if( (*versionIterator)->isNewerThan(
			      (*sortedVersionIterator)->version() ) )
			{
				sortedVersionIterator++; // insert after the compared one, not before
				sortedVersions.insert( sortedVersionIterator, *versionIterator );
				break;
			}
		}
	}
	return sortedVersions;
} // end of sortedVersionList()

/**
 * Retrieve the latest version of this package, not taking stability
 * (masking, etc.) into account. If this package contains no versions,
 * NULL is returned.
 */
PackageVersion* Package::latestVersion()
{
	if( m_versions.count() == 0 )
		return NULL;

	QValueList<PackageVersion*> sortedVersions = this->sortedVersionList();
	return sortedVersions.last();
}

/**
 * Retrieve the latest version of this package that can be installed.
 * If this package contains no available versions, NULL is returned.
 */
PackageVersion* Package::latestVersionAvailable()
{
	if( m_versions.count() == 0 )
		return NULL;

	QValueList<PackageVersion*> sortedVersions = this->sortedVersionList();
	QValueList<PackageVersion*>::iterator versionIterator
		= sortedVersions.end();

	// Iterate through the versions, starting at the latest one.
	// If it's stable, we have a result.
	while(true)
	{
		versionIterator--;

		if( (*versionIterator)->isAvailable() )
			return (*versionIterator);

		if( versionIterator == sortedVersions.begin() )
			break;
	}

	return NULL; // if there is no stable version
}

/**
 * Check if there is an update available for any version of the package.
 * This default implementation skips installed packages and checks
 * canUpdate(PackageVersion*) to see if it's actually updatable.
 *
 * @returns  true if there is any update on arch, false otherwise.
 */
bool Package::canUpdate()
{
	PackageVersionMap::iterator versionIterator;

	for( versionIterator = m_versions.begin();
	     versionIterator != m_versions.end(); versionIterator++ )
	{
		if( (*versionIterator)->isInstalled() == false )
			continue;

		// only installed versions are checked for an update
		if( this->canUpdate( *versionIterator ) == true )
			return true;
	}
	// if the loop hasn't already returned true, there are no updates
	return false;
}


/**
 * Check if there is an update available from the given installed version.
 * This function is used by the canUpdate() default implementation.
 * This implementation simply checks if there is a newer version than the
 * given version, which should suffice in all cases except when there are
 * different aspects to be considered than just the version number.
 *
 * @param version  An installed version which will be checked on updates.
 * @returns  true if there is an update for version on arch, false otherwise.
 */
bool Package::canUpdate( PackageVersion* version )
{
	QValueList<PackageVersion*> sortedVersions = this->sortedVersionList();
	QValueList<PackageVersion*>::iterator versionIterator;
	bool check = false;

	// go through each version and check if it's newer than the given one
	for( versionIterator = sortedVersions.begin();
	     versionIterator != sortedVersions.end(); versionIterator++ )
	{
		// Don't check versions that are not newer than the given one
		if( check == false
		    && (*versionIterator)->version() != version->version() )
		{
			continue;
		}
		else if( check == true )
		{
			// If we get here, the versionIterator is newer
			// than the given version.

			if( (*versionIterator)->isInstalled() == true ) {
				continue; // if it's installed, it's not upgradable. next one.
			}
			if( (*versionIterator)->isAvailable() ) {
				return true;
			}
		}
		else // if( (*versionIterator).version == versionString )
		{
			// From the next iteration on,
			// all versions are newer than the given one:
			check = true;
		}
	}
	// if the loop hasn't already returned true, there are no updates
	return false;
} // end of canUpdate()


Package::versioniterator Package::versionBegin()
{
	return m_versions.begin();
}

Package::versioniterator Package::versionEnd()
{
	return m_versions.end();
}

Package::const_versioniterator Package::versionBegin() const
{
	return m_versions.begin();
}

Package::const_versioniterator Package::versionEnd() const
{
	return m_versions.end();
}

} // namespace
