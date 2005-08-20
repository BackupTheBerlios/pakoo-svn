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

#include "portagetree.h"

#include "packageversion.h"
#include "package.h"


/**
 * Initialize this object.
 */
PortageTree::PortageTree()
{
}

/**
 * Remove all packages from the tree.
 */
void PortageTree::clear()
{
	packages.clear();
}

/**
 * Add an existing Package object to the tree. If a Package object with
 * same name, category and subcategory already exists, it is replaced.
 *
 * @return  true if the package has been added to the tree.
 *          false if it has not been added because its name string was empty.
 */
bool PortageTree::setPackage( Package& package )
{
	if( package.name == "" )
		return false;

	this->packages[
		package.category + package.subcategory + package.name
	] = package;

	return true;
}

/**
 * Create a package object and add it to the tree. If a Package object with
 * same name, category and subcategory already exists, it is replaced.
 *
 * @param category     The package category, e.g. "sys".
 * @param subcategory  The package subcategory, e.g. "kernel".
 * @param package      The package name string.
 * @return  true if the package has been added to the tree.
 *          false if it has not been added because its name string was empty.
 */
bool PortageTree::setPackage( const QString& category,
                              const QString& subcategory,
                              const QString& package )
{
	Package pkg( category, subcategory, package );
	return this->setPackage( pkg );
}

/**
 * See if the tree contains a specific package.
 *
 * @param category     The package category, e.g. "sys".
 * @param subcategory  The package subcategory, e.g. "kernel".
 * @param package      The requested package name string.
 * @return  true if the tree contains the package, false otherwise.
 */
bool PortageTree::hasPackage( const QString& category,
                              const QString& subcategory,
                              const QString& package )
{
	if( this->packages.contains( category + subcategory + package ) )
		return true;
	else
		return false;
}

/**
 * Return a Package object for a given package name, category and subcategory.
 * If the Package object doesn't exist yet, it is created
 * (taking the arguments as default initialization values).
 *
 * @param category     The package category, e.g. "sys".
 * @param subcategory  The package subcategory, e.g. "kernel".
 * @param package      The requested package name string.
 * @return  A Package object. If you change its values,
 *          they are also changed in this PortageTree object.
 */
Package* PortageTree::package( const QString& category,
                               const QString& subcategory,
                               const QString& package )
{
	Package* pkg = &(this->packages[ category + subcategory + package ]);
	if( pkg->name == "" ) { // empty package
		pkg->category    = category;
		pkg->subcategory = subcategory;
		pkg->name        = package;
	}

	return pkg;
}

/**
 * Return the list of packages in the tree.
 */
PackageMap* PortageTree::packageMap()
{
	return &(this->packages);
}

/**
 * Return the number of packages in the tree.
 */
int PortageTree::packageCount()
{
	return this->packages.count();
}
