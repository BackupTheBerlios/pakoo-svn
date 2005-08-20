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

#include "packagecategory.h"

#include <klocale.h>

#define ALLPACKAGESCATEGORY \
	i18n("Name for the 'All Packages' category", "All Packages")

#define min(a,b) (a<b ? a : b)


namespace libpakt {

/**
 * The empty constructor just calls the QStringList constructor.
 */
PackageCategory::PackageCategory() : QStringList()
{}

/**
 * A copy constructor for getting the category name parts
 * from QStringList values (which can also be another PackageCategory).
 */
PackageCategory::PackageCategory( const QStringList& list )
	: QStringList( list )
{}

/**
 * The assignment operator copies the category name parts
 * from QStringList values (which can also be another PackageCategory).
 */
PackageCategory& PackageCategory::operator=( const QStringList& list )
{
	*this = list;
	return *this;
}

/**
 * Determine if this category is contained in another one,
 * which means it's either the same or a subset of the other category.
 *
 * For example, if this category is {"app", "portage"}, this would return true
 * if the other one is the same, or just {"app"}, or an empty list.
 * It would return false if the other one is {"sys"}, or {"app","office"}.
 */
bool PackageCategory::isContainedIn( PackageCategory& otherCategory )
{
	if( otherCategory.count() > this->count() )
	{
		// e.g., otherCategory == app-portage, and this == app.
		// As the other category is more specific, this one can't be
		// contained in it.
		return false;
	}
	for( unsigned int i = 0; i < otherCategory.count(); i++ )
	{
		if( otherCategory[i] != (*this)[i] )
			return false; // that's clear, isn't it
	}
	// if all comparisons until here were equal,
	// then this category is contained in the other one, so:
	return true;
}

/**
 * Retrieve a user visible string of the category name.
 * This string shouldn't look too cryptic and should make use of
 * internationalization, if needed.
 */
QString PackageCategory::userVisibleName() const
{
	if( this->count() == 0 )
		return ALLPACKAGESCATEGORY;
	else
		return this->join(" / ");
}

/**
 * Get the unique name of this category.
 * Most likely this will be a string that's easy to parse, because it must
 * be possible to reconstruct the category in loadFromUniqueName().
 *
 * This string should be relatively short and doesn't have to be pretty
 * or internationalized to please the user. For user visible strings,
 * please use userVisibleName() instead.
 *
 * @see loadFromUniqueName
 * @see userVisibleName
 */
QString PackageCategory::uniqueName() const
{
	if( this->count() == 0 )
		return "[no category]";
	else
		return this->join("/");
}

/**
 * Get the category name parts from its unique name string.
 * The current name is discarded.
 *
 * @return  true if the category could be loaded from the string.
 *          false if it could not be loaded. If false was returned,
 *          the current category name parts remain.
 */
bool PackageCategory::loadFromUniqueName( const QString& uniqueName )
{
	if( uniqueName == "[no category]" ) {
		this->clear();
	}
	else {
		PackageCategory category( QStringList::split("/", uniqueName) );
		*this = category;
	}
	return true;
}

} // namespace
