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

#include "packagesearchline.h"

#include <qlistview.h>


namespace libpakt {

/**
 * Initialize this object.
 */
PackageSearchLine::PackageSearchLine(
	QWidget* parent, PackageListView* listView, const char* name )
: KListViewSearchLine(parent, listView, name)
{
	filter = All;
}

/**
 * Overloaded for stricter type checking.
 */
void PackageSearchLine::setListView( PackageListView* lv )
{
	KListViewSearchLine::setListView( lv );
}

/**
 * Set the filter for package items.
 */
void PackageSearchLine::setFilter(
	PackageSearchLine::Filter packageFilter )
{
	filter = packageFilter;
}

/**
 * Overloaded to include version items and to filter for package properties.
 */
bool PackageSearchLine::itemMatches(
	const QListViewItem* item, const QString& s ) const
{
	switch( item->depth() )
	{
	case 0: // never include category items
		return false;
	case 2: // include all wanted version items
		return itemMatches( item->parent(), s );

	default: // search in package items, optionally with filter
		if( filter == Installed )
		{
			PackageListView* view = (PackageListView*) this->listView();
			if( view->hasInstalledVersion(item) == false )
				return false;
		}
		return KListViewSearchLine::itemMatches( item, s );
	}
}

} // namespace
