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

#include "portagecategory.h"


namespace libpakt {

/**
 * Initialize the string list with main and sub category values.
 */
PortageCategory::PortageCategory( const QString& mainCategory,
                                  const QString& subCategory )
: PackageCategory()
{
	setCategory( mainCategory, subCategory );
}

/**
 * Set the category to a Portage-like combination of main
 * and sub category.
 */
void PortageCategory::setCategory( const QString& mainCategory,
                                   const QString& subCategory )
{
	this->clear();
	this->append( mainCategory );
	this->append( subCategory );
}

/**
 * Retrieve a user visible string of the category name.
 * This string shouldn't look too cryptic and should make use of
 * internationalization, if needed.
 */
QString PortageCategory::userVisibleName() const
{
	if( this->count() == 0 )
		return PackageCategory::userVisibleName();
	else
		return this->join("-");
}

/**
 * Retrieve the unique name of the category. For the Portage backend,
 * this will be the list of strings in here seperated by a comma.
 * (Example: A category containing {"app","portage"} will return
 * "app-portage" with this function.)
 *
 * An empty list will return something like "All Packages" here.
 */
QString PortageCategory::uniqueName() const
{
	if( this->count() == 0 )
		return "[no category]";
	else
		return this->join("-");
}

/**
 * Get the category name parts from its unique name string.
 * The current name is discarded.
 *
 * @return  true if the category could be loaded from the string.
 *          false if it could not be loaded. If false was returned,
 *          the current category name parts remain.
 */
bool PortageCategory::loadFromUniqueName( const QString& uniqueName )
{
	if( uniqueName == "[no category]" ) {
		this->clear();
	}
	else {
		PortageCategory category( QStringList::split("-", uniqueName) );
		*this = category;
	}
	return true;
}

} // namespace
