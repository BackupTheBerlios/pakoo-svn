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

#ifndef PORTAGETREEVIEW_H
#define PORTAGETREEVIEW_H

#include <klistview.h>

#include <qmap.h>
#include <qstring.h>
#include <qpixmap.h>

#include "../libqortage/libqortage.h"

/**
 * A KListView with additional functions to handle portage tree categories.
 *
 * @short Widget to display categories and subcategories of the portage tree.
 */
class PortageTreeView : public KListView
{
	Q_OBJECT
public:
	PortageTreeView( QWidget* parent, const char* name );

	void setPortageTree( PortageTree* portageTree );

signals:
	void selectionChanged( PortageTree* tree,
		const QString& categoryName, const QString& subcategoryName
	);

protected slots:
	void emitSelectionChanged( QListViewItem* item );

protected:
	struct TreeViewCategory {
		QListViewItem* item;
		QMap<QString,QListViewItem*> subcategories;
	};

	QListViewItem* rootItem;
	QMap<QString,TreeViewCategory> categories;
	PortageTree* portageTree;

	QPixmap pxRootItem, pxCategoryItem;
	int width;
};

#endif // PORTAGETREEVIEW_H
