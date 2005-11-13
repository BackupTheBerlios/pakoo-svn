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

#include "portagepackage.h"

#include "portagepackageversion.h"


namespace libpakt {

/**
 * Initialize the package with name and category.
 * Note that you mustn't delete the given category object,
 * because it is now considered to be owned by this Package object.
 */
PortagePackage::PortagePackage( PackageCategory* category,
                                const QString& name )
	: Package( category, name )
{
	m_cachedDescription = QString::null;
}

/**
 * Allocates a new PortagePackageVersion on the heap.
 */
PortagePackageVersion* PortagePackage::createPackageVersion(
	const QString& versionString )
{
	m_cachedDescription = QString::null; // clear cache
	return new PortagePackageVersion( this, versionString );
};

/**
 * Overloaded for caching support.
 * @see PackageVersion::clear()
 */
void PortagePackage::clear()
{
	m_cachedDescription = QString::null; // clear cache
	Package::clear();
}

/**
 * Overloaded for caching support.
 * @see PackageVersion::removeVersion()
 */
void PortagePackage::removeVersion( const QString& version )
{
	m_cachedDescription = QString::null; // clear cache
	Package::removeVersion( version );
}

/**
 * Overloaded for caching support and returning a PortagePackageVersion
 * instead of a standard PackageVersion.
 *
 * @see PackageVersion::insertVersion(const QString&)
 */
PortagePackageVersion* PortagePackage::insertVersion(
	const QString& versionString )
{
	m_cachedDescription = QString::null; // clear cache
	return (PortagePackageVersion*) Package::insertVersion( versionString );
}

/**
 * Overloaded for returning a PortagePackageVersion
 * instead of a standard PackageVersion.
 *
 * @see PackageVersion::version(const QString&)
 */
PortagePackageVersion* PortagePackage::version( const QString& versionString )
{
	return (PortagePackageVersion*) Package::version( versionString );
}

/**
 * Returns a description of this package.
 * For Portage, this is the same as shortDescription().
 */
QString PortagePackage::description()
{
	// only determine the description if it's not cached
	if( m_cachedDescription == QString::null )
	{
		PackageVersion* version = latestVersionAvailable();
		if( version == NULL )
			version = latestVersion();
		if( version == NULL )
			return "";

		m_cachedDescription = ((PortagePackageVersion*)version)->description();
	}

	return m_cachedDescription;
}

/**
 * Returns a short description of this package.
 * For Portage, this is the same as description().
 */
QString PortagePackage::shortDescription()
{
	return description();
}

/**
 * Reimplemented for slot-awareness.
 * This implementation assumes that the given PackageVersion object
 * is in fact a PortagePackageVersion - never handle over another one,
 * or it will crash.
 *
 * @param genericVersion  An installed version which will be checked on updates.
 * @returns  true if there is an update for version on arch, false otherwise.
 * @see Package::canUpdate(PackageVersion*)
 */
bool PortagePackage::canUpdate( PackageVersion* genericVersion )
{
	PortagePackageVersion* version = (PortagePackageVersion*) genericVersion;
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
			if( version->slot() ==
			        ((PortagePackageVersion*)(*versionIterator))->slot()
			    && (*versionIterator)->isAvailable() )
			{
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

/**
 * Retrieves the list of slots that this package's versions use.
 */
QStringList PortagePackage::slotList()
{
	QValueList<QString> slotList;
	PackageVersionMap::iterator versionIterator;

	for( versionIterator = m_versions.begin();
	     versionIterator != m_versions.end(); versionIterator++ )
	{
		if( slotList.contains(
				((PortagePackageVersion*)(*versionIterator))->slot() ) == 0 )
		{
			slotList.append(
				((PortagePackageVersion*)(*versionIterator))->slot() );
		}
	}
	return slotList;
}

/**
 * Return a list of PackageVersion objects sorted by their version numbers,
 * with the oldest version at the beginning and the latest version at the end
 * of the list. Only versions whose slot property equals the given one
 * are included in the returned list.
 */
QValueList<PackageVersion*> PortagePackage::sortedVersionListInSlot(
	const QString& slot )
{
	QValueList<PackageVersion*> sortedVersionsInSlot;

	QValueList<PackageVersion*> sortedVersions = this->sortedVersionList();
	QValueList<PackageVersion*>::iterator versionIterator;

	// go through each version and check if it's newer than the given one
	for( versionIterator = sortedVersions.begin();
	     versionIterator != sortedVersions.end(); versionIterator++ )
	{
		if( ((PortagePackageVersion*)(*versionIterator))->slot() == slot )
			sortedVersionsInSlot.append( *versionIterator );
	}
	return sortedVersionsInSlot;
}

} // namespace
