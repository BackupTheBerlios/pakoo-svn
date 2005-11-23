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

#include "packagelist.h"

#include "packageversion.h"
#include "package.h"
#include "packagecategory.h"

#include <klocale.h>
#include <kdebug.h>


namespace libpakt {

/**
 * Initialize this object with an empty package list.
 */
PackageList::PackageList()
{
}

/**
 * Return the number of packages in the tree.
 */
int PackageList::count()
{
	return m_packages.count();
}

/**
 * Remove all packages from the tree.
 */
void PackageList::clear()
{
	m_packages.clear();
}

/**
 * Add an existing Package object to the list. If a Package object with
 * same name, category and subcategory already exists, it is replaced.
 *
 * @return  The pointer to the package if it has successfully been added.
 *          NULL if it has not been added because its name string was empty.
 */
Package* PackageList::insert( Package* package )
{
	if( package == NULL || package->name() == "" )
		return NULL;

	PackageMap::iterator packageIterator = m_packages.insert(
		package->category()->uniqueName() + package->name(),
		KSharedPtr<Package>( package )
	);

	if( packageIterator == m_packages.end() )
		return NULL; // could not be inserted into m_packages
	else // return the real pointer, not the KSharedPtr object
		return (*packageIterator).data();
}

/**
 * Create a package object and add it to the tree. If a Package object with
 * the same name and category already exists, it is replaced.
 *
 * @param category  The package category.
 * @param name      The package name string.
 * @return  The pointer to the package if it has successfully been added.
 *          NULL if it has not been added because its name string was empty.
 */
Package* PackageList::insert( PackageCategory* category,
                              const QString& name )
{
	if( category == NULL )
		return false;

	Package* pkg = createPackage( category, name );
	return this->insert( pkg );
}

/**
 * See if the tree contains a specific package.
 *
 * @param category  The category containing the requested package.
 * @param name      The requested package name string.
 * @return  true if the tree contains the package, false otherwise.
 */
bool PackageList::contains( PackageCategory* category,
                            const QString& name )
{
	if( category == NULL )
		return false;

	return m_packages.contains( category->uniqueName() + name );
}

/**
 * Return a Package object for a given package name and category.
 * If the Package object doesn't exist yet, it is created
 * (taking the arguments as default initialization values).
 *
 * By calling this function, you give up ownership and control of
 * the given category, which is now owned and held by the new
 * Package object. So, do not delete the category after calling this
 * function. You can get this category by calling the category()
 * function of the returned Package object.
 *
 * @param category  The category containing the requested package.
 * @param name      The requested package name string.
 * @return  A Package object. If you change its values,
 *          they are also changed in this PackageList object.
 *
 * @see Package::category()
 */
Package* PackageList::package( PackageCategory* category,
                               const QString& name )
{
	if( category == NULL ) {
		kdDebug() << i18n( "PackageList debug output",
			"PackageList::package(): "
			"Didn't retrieve a Package because category is NULL" )
			<< endl;
		return NULL;
	}

	PackageMap::iterator packageIterator =
		m_packages.find( category->uniqueName() + name );

	// if there is no such package, then create one and retrieve again:
	if( packageIterator == m_packages.end() ) {
		return insert( category, name ); // returns the new package pointer
	}
	else { // The package already exists, delete the superfluous category
		delete category;
		return (*packageIterator).data(); // the real pointer to the package
	}
}


PackageList::iterator PackageList::begin()
{
	return m_packages.begin();
}

PackageList::iterator PackageList::end()
{
	return m_packages.end();
}

PackageList::const_iterator PackageList::begin() const
{
	return m_packages.begin();
}

PackageList::const_iterator PackageList::end() const
{
	return m_packages.end();
}

} // namespace
