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

#include "packageselector.h"

#include "package.h"
#include "packagelist.h"

#include <klocale.h>
#include <kdebug.h>

// Allocates an object if it's NULL
#define ENSURE_EXISTANCE(pointer,type) \
	if( pointer == NULL ) { \
		pointer = new type; \
	}

#define SAFEDELETE(pointer) \
	if( pointer != NULL ) { \
		delete pointer; \
		pointer = NULL; \
	}

// Replace when porting to Qt4.
#define FOREACH(iterName,list,itemtype) \
	for( QValueList<itemtype>::iterator iterName = list->begin(); \
	     iterName != list->end(); iterName++ )

// Deep copy of the filter values (which are only saved as pointers)
// from another PackageSelector.
#define DEEPCOPY(otherSelector,pointer,type) \
	if( this->pointer != NULL ) { \
		delete this->pointer; \
		this->pointer = NULL; \
	} \
	if( otherSelector.pointer == NULL ) { \
		this->pointer = NULL; \
	} \
	else { \
		this->pointer = new type; \
		*(this->pointer) = *(otherSelector.pointer); \
	}


namespace libpakt {

/**
 * Initialize this object without setting filters or package lists.
 */
PackageSelector::PackageSelector() : ThreadedJob()
{
	m_includedCategories = NULL;
	m_excludedCategories = NULL;
	m_includeInstalledPackages = NULL;
	m_excludeInstalledPackages = NULL;
	clearFilters();
}

/**
 * Copy constructor, achieving the same as the assignment operator.
 * @see operator=( const PackageSelector& )
 */
PackageSelector::PackageSelector( const PackageSelector& otherSelector )
	: ThreadedJob()
{
	copyFrom( otherSelector );
}

/**
 * Copies the the filter settings from another PackageSelector.
 */
PackageSelector& PackageSelector::operator=(
	const PackageSelector& otherSelector )
{
	copyFrom( otherSelector );
	return *this;
}

/**
 * Set the source package list. All packages from this list are tested
 * and the ones matching the package filter go into the destination list.
 */
void PackageSelector::setSourceList( PackageList* sourceList )
{
	m_sourceList = sourceList;
}

/**
 * Set the destination package list. All packages from the source list
 * matching the package filter go into this list.
 *
 * The destination list is cleared at the beginning of the job.
 */
void PackageSelector::setDestinationList( PackageList* destList )
{
	m_destList = destList;
}

/**
 * Delete all filters that have been previously set,
 * and call setAllPackagesFilter( Exclude ).
 *
 * @see  PackageSelector::setAllPackagesFilter
 */
void PackageSelector::clearFilters()
{
	setAllPackagesFilter( Exclude );
	clearCategoryFilters();
	clearIsInstalledFilters();
}

/**
 * Define if all packages should normally be included or excluded.
 * Other filters may increment or decrement the number of matching
 * packages. This filter is the one with least priority, meaning
 * that it is overridden by all other filters.
 *
 * By default, this filter is set to PackageSelector::Exclude.
 */
void PackageSelector::setAllPackagesFilter( FilterType allPackagesFilter )
{
	m_allPackagesFilter = allPackagesFilter;
}


/**
 * Add a filter for category matching. The filter matches a package if its
 * category is the same or a subset of the category given as argument.
 * (For comparing, PortageCategory::containedIn(category) is used.)
 *
 * @see FilterType
 * @see PortageCategory::containedIn
 */
void PackageSelector::addCategoryFilter( FilterType filterType,
                                         const PackageCategory& category )
{
	if( filterType == Include ) {
		ENSURE_EXISTANCE( m_includedCategories, QValueList<PackageCategory> );
		m_includedCategories->append( category );
	}
	else if( filterType == Exclude ) {
		ENSURE_EXISTANCE( m_excludedCategories, QValueList<PackageCategory> );
		m_excludedCategories->append( category );
	}
}

/**
 * Delete all category filters that have been previously set.
 */
void PackageSelector::clearCategoryFilters()
{
	SAFEDELETE( m_includedCategories );
	SAFEDELETE( m_excludedCategories );
}


/**
 * Add a filter for checking on a package's status of installation.
 * The filter matches if the 'installed' argument is the same as the
 * return value of Package::hasInstalledVersion().
 *
 * @see FilterType
 * @see Package::hasInstalledVersion()
 */
void PackageSelector::addIsInstalledFilter( FilterType filterType,
                                            bool installed )
{
	if( filterType == Include ) {
		ENSURE_EXISTANCE( m_includeInstalledPackages, bool );
		*m_includeInstalledPackages = installed;
	}
	else if( filterType == Exclude ) {
		ENSURE_EXISTANCE( m_excludeInstalledPackages, bool );
		*m_excludeInstalledPackages = installed;
	}
}

/**
 * Delete all filters checking for a package's status of installation.
 */
void PackageSelector::clearIsInstalledFilters()
{
	SAFEDELETE( m_includeInstalledPackages );
	SAFEDELETE( m_excludeInstalledPackages );
}


/**
 * Private function which is used from both the copy constructor and
 * the assignment operator. Copies the filter settings from another
 * PackageSelector.
 */
void PackageSelector::copyFrom( const PackageSelector& otherSelector )
{
	m_allPackagesFilter = otherSelector.m_allPackagesFilter;
	DEEPCOPY( otherSelector, m_includeInstalledPackages, bool );
	DEEPCOPY( otherSelector, m_excludeInstalledPackages, bool );
	DEEPCOPY( otherSelector,
	          m_includedCategories, QValueList<PackageCategory> );
	DEEPCOPY( otherSelector,
	          m_excludedCategories, QValueList<PackageCategory> );
}


/**
 * This function is called when the job is executed.
 * It should be called using start() or perform() and selects a subset
 * of the packages in the source list which is put into the destination
 * list.
 *
 * @see ThreadedJob::start()
 * @see ThreadedJob::perform()
 */
IJob::JobResult PackageSelector::performThread()
{
	if( m_sourceList == NULL )
	{
		kdDebug() << i18n( "PackageSelector debug output",
			"PackageSelector::performThread(): "
			"Didn't start scanning because "
			"the source PackageList is NULL." )
			<< endl;
		return Failure;
	}
	if( m_destList == NULL )
	{
		kdDebug() << i18n( "PackageSelector debug output",
			"PackageSelector::performThread(): "
			"Didn't start scanning because "
			"the destination PackageList is NULL." )
			<< endl;
		return Failure;
	}
	m_destList->clear();

	PackageList::iterator packageIteratorEnd = m_sourceList->end();

	// Iterate through all packages
	for( PackageList::iterator packageIterator = m_sourceList->begin();
	     packageIterator != packageIteratorEnd; ++packageIterator )
	{
		// test the current package
		if( includePackage( *packageIterator ) == true )
			m_destList->insert( *packageIterator );

		if( aborting() )
		{
			kdDebug() << i18n( "PackageSelector debug output",
				"PackageSelector::performThread(): "
				"Aborting on user request" )
				<< endl;
			return Failure;
		}
	}

	return Success;
}

/**
 * Checks inclusion and exclusion filters to determine if a specific package
 * will be included in the result package list. Do this check for each package
 * and you know which ones to take. true means the package should be included,
 * false means it shouldn't be.
 */
bool PackageSelector::includePackage( Package* package )
{
	// exclusion filters first, they have higher priority
	if( exclusionFilterMatches(package) )
		return false;

	// all exclusions are done (and don't apply for this package,
	// because otherwise we wouldn't come here): so, if any inclusion filter
	// matches, then the package is in.
	if( m_allPackagesFilter == Include || inclusionFilterMatches(package) )
		return true;
	else
		return false;
}

/**
 * Returns true if there is at least one inclusion filter that matches
 * the given package. The All Packages Filter is an exception which is
 * not checked in this function. If an inclusion filter matches (and
 * no exclusion filter does) then a package will be included in the
 * result package list. Returns false if there is no inclusion filter
 * matching this package.
 */
bool PackageSelector::inclusionFilterMatches( Package* package )
{
	// included categories filter
	if( m_includedCategories != NULL )
	{
		FOREACH( categoryIterator, m_includedCategories, PackageCategory )
		{
			if( !package->category()->isContainedIn(*categoryIterator) )
				return false;
		}
	}
	// include packages with 'package.installed == given value' filter
	if( m_includeInstalledPackages != NULL )
	{
		if( !package->containsInstalledVersion() == *m_includeInstalledPackages )
			return false;
	}
	return true;
}

/**
 * Returns true there is at least one exclusion filter that matches
 * the given package. The All Packages Filter is an exception which is
 * not checked in this function. If an exclusion filter matches,
 * the package will not be included in the result package list.
 * Returns false if no exclusion filter matches, which would mean
 * that the package still has chances to be included in the result list.
 */
bool PackageSelector::exclusionFilterMatches( Package* package )
{
	// excluded categories filter
	if( m_excludedCategories != NULL )
	{
		FOREACH( categoryIterator, m_excludedCategories, PackageCategory )
		{
			if( package->category()->isContainedIn(*categoryIterator) )
				return true;
		}
	}
	// exclude packages with 'package.installed == given value' filter
	if( m_excludeInstalledPackages != NULL )
	{
		if( package->containsInstalledVersion() == *m_excludeInstalledPackages )
			return true;
	}
	return false;
}

} // namespace
