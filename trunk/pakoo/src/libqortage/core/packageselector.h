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

#ifndef LIBPAKTPACKAGESELECTOR_H
#define LIBPAKTPACKAGESELECTOR_H

#include "threadedjob.h"
#include "../core/portagecategory.h"


namespace libpakt {

class Package;
class PackageList;
class PackageCategory;

/**
 * PackageSelector is a class that can handle one or more filter settings
 * that are used for filtering out a subset of packages in a package list.
 * After settings the source and destination package lists as well as the
 * requested filter rules, you can call start() or perform() to fill the
 * destination package list with packages matching the filters.
 *
 * By default, the filter rules are set in a way that no packages
 * go into the destination list. Filter rules that exclude packages
 * have higher priority than inclusion filters (one exception:
 * the All Packages Filter, which always has least priority).
 *
 * @short  Used for generating a package list containing a filtered subset of another list.
 */
class PackageSelector : public ThreadedJob
{
public:
	/**
	 * Possible filter type values. If a filter has the type
	 * PackageSelector::Include then matching packages will be contained
	 * in the destination package list, except when there is some
	 * other matching filter of type PackageSelector::Exclude.
	 *
	 * So, exclusion filters have higher priority than inclusion filters.
	 * The All Packages Filter is the exception from this ruls and doesn't
	 * exclude packages with a matching inclusion filter.
	 */
	enum FilterType {
		Include,
		Exclude
	};

	PackageSelector();
	PackageSelector( const PackageSelector& );
	PackageSelector& operator=( const PackageSelector& otherSelector );

	// setting up
	void setSourceList( PackageList* sourceList );
	void setDestinationList( PackageList* destList );

	// filters
	void clearFilters();
	void setAllPackagesFilter( PackageSelector::FilterType filterType );
	void addCategoryFilter( PackageSelector::FilterType filterType,
	                        const PackageCategory& category );
	void clearCategoryFilters();
	void addIsInstalledFilter( PackageSelector::FilterType filterType,
	                           bool isPackageInstalled );
	void clearIsInstalledFilters();

signals:
	/**
	 * Emitted for untranslated debug output, like starting the scan
	 * or error messages.
	 */
	void debugOutput( QString output );

protected:
	JobResult performThread();

private:
	void copyFrom( const PackageSelector& otherSelector );
	bool includePackage( Package* package );
	bool inclusionFilterMatches( Package* package );
	bool exclusionFilterMatches( Package* package );

	//! The list from where the packages are taken.
	PackageList* sourceList;
	//! The list where matching packages are inserted.
	PackageList* destList;

	//! Defines if all packages are normally included or excluded.
	FilterType allPackagesFilter;

	//
	// Filters.
	// If set to NULL, it means that this filter is not set.
	// Otherwise the value is used by the specific filter code.
	//
	QValueList<PackageCategory> *includedCategories, *excludedCategories;
	bool *includeInstalledPackages, *excludeInstalledPackages;
};

}

#endif // LIBPAKTPACKAGESELECTOR_H
