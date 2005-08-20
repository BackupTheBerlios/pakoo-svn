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

#ifndef LIBPAKTBACKENDFACTORY_H
#define LIBPAKTBACKENDFACTORY_H


namespace libpakt {

class PackageList;
class PackageSelector;
class MultiplePackageLoader;
class PackageCategory;
class InitialLoader;
class PackageLoader;

/**
 * This is the abstract factory that's supposed to be derived
 * by concrete factories for the specific back ends.
 *
 * The create* Member functions of derived classes create the backend-specific
 * objects using the new operator, which means you'll have to care for
 * deleting it properly.
 */
class BackendFactory
{
public:
	BackendFactory();

	/**
	 * Determine if the backend supports loading package structures.
	 */
	virtual bool hasLoaderClasses() = 0;

	/**
	 * Determine if the backend supports package management actions
	 * like installing, uninstalling, and the likes.
	 */
	virtual bool hasInstallerClasses() = 0;

	//! Determine if the backend supports configuration widgets.
	virtual bool hasConfigClasses() = 0;


	//
	// Core classes.
	//

	/**
	 * Creates a PackageList. With this list you can manage Package objects.
	 * A PackageList object can create, retrieve and remove such objects,
	 * but the proposed method is to fill the list using the InitialLoader,
	 * retrieve package details using the [Multiple]PackageLoader,
	 * and select sets of packages using the PackageSelector.
	 *
	 * @see PackageList
	 */
	virtual PackageList* createPackageList();

	/**
	 * Creates a PackageCategory. Category objects are able to return their
	 * full name and can be compared to other categories. They can also be
	 * used for filtering packages, with the help of a PackageSelector object.
	 *
	 * @see PackageCategory
	 * @see PackageSelector
	 */
	virtual PackageCategory* createPackageCategory() = 0;

	/**
	 * Creates a PackageSelector object.
	 * @see PackageSelector
	 */
	virtual PackageSelector* createPackageSelector();


	//
	// Loader classes.
	//

	/**
	 * Creates an InitialLoader object which should be run before
	 * using any of the other objects.
	 *
	 * @see InitialLoader
	 */
	virtual InitialLoader* createInitialLoader() = 0;

	/**
	 * Creates a PackageLoader object.
	 * @see PackageLoader
	 */
	virtual PackageLoader* createPackageLoader() = 0;

	/**
	 * Creates a MultiplePackageLoader object.
	 * @see MultiplePackageLoader
	 */
	virtual MultiplePackageLoader* createMultiplePackageLoader(
		PackageLoader* packageLoader );

};

}

#endif // LIBPAKTBACKENDFACTORY_H
