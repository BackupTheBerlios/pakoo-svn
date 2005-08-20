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

#ifndef PACKAGETREEVIEW_H
#define PACKAGETREEVIEW_H

#include <klistview.h>
#include <ksharedptr.h>

#include <qmap.h>
#include <qstring.h>
#include <qpixmap.h>


namespace libpakt {

class BackendFactory;
class PackageList;
class PackageSelector;

/**
 * A KListView with additional functions to handle portage tree categories.
 *
 * @short Widget to display categories and subcategories of the portage tree.
 */
class PackageTreeView : public KListView
{
	Q_OBJECT

public:
	PackageTreeView( QWidget* parent, const char* name,
	                 BackendFactory* backend );

public slots:
	void setPackageList( PackageList* packages );
	void clear();

signals:
	/**
	 * Emitted when setPackageList is called.
	 * @see setPackageList
	 */
	void packageListChanged( PackageList& packages );

	/**
	 * Emitted when a category item is selected.
	 * The given PackageSelector object is set up so that it can
	 * filter the packages of the selected category.
	 */
	void selectionChanged( PackageSelector& packageSelector );

private slots:
	void emitSelectionChanged( QListViewItem* item );

private:
	class ParentItem : public KShared {
	public:
		QListViewItem* item;
		QMap<QString,KSharedPtr<ParentItem> > children;
	};

	QListViewItem* rootItem( QListViewItem* item );

	/**
	 * The backend that creates PackageCategories
	 * and PackageSelectors for this object.
	 */
	BackendFactory* backend;

	/** The top-level category item ("All Packages") and its children. */
	ParentItem categoryRootItem;

	// Icons for the QListViewItems
	QPixmap pxCategoryRootItem, pxCategoryItem;
};

}

#endif // PACKAGETREEVIEW_H
