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

#ifndef PACKAGESEARCHLINE_H
#define PACKAGESEARCHLINE_H

#include <klistviewsearchline.h>

#include "packagelistview.h"


namespace libpakt {

/**
 * A KListViewSearchLine customized to work with a PackageListView.
 * It mainly overloads the item matching function to display the right
 * items (always show version items, filtering capabilities).
 *
 * @short A KListViewSearchLine customized to work with a PackageListView.
 */
class PackageSearchLine : public KListViewSearchLine
{
	Q_OBJECT
public:
	enum Filter {
		All,
		Installed
	};
	PackageSearchLine( QWidget* parent, PackageListView* listView, const char* name );

public slots:
	void setFilter( PackageSearchLine::Filter packageFilter );
	void setListView( PackageListView* lv );

protected:
	bool itemMatches(const QListViewItem *item, const QString &s) const;

private:
	Filter filter;
};

}

#endif // PACKAGESEARCHLINE_H
