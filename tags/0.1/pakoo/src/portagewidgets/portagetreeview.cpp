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

#include <qheader.h>

#include <kiconloader.h>

#include "portagetreeview.h"
#include "pixmapnames.h"
#include "i18n.h"


/**
 * Initialize this object.
 */
PortageTreeView::PortageTreeView( QWidget* parent, const char* name )
 : KListView( parent, name )
{
	portageTree = NULL;

	// load root and category item icons
	KIconLoader* iconLoader = KGlobal::iconLoader();
	pxRootItem = iconLoader->loadIcon( ALLPACKAGESICON, KIcon::Small );
	pxCategoryItem = iconLoader->loadIcon( CATEGORYICON, KIcon::Small );

	this->header()->hide();
	this->addColumn( "Categories" );
	this->setColumnWidthMode( 0, QListView::Maximum );
	this->setRootIsDecorated( true );

	rootItem = new KListViewItem( this, ALLPACKAGESTEXT );
	rootItem->setExpandable( true );
	rootItem->setOpen( true );
	rootItem->setPixmap( 0, pxRootItem );

	this->setFullWidth( true );

	connect( this, SIGNAL(selectionChanged(QListViewItem*)),
	         this, SLOT(emitSelectionChanged(QListViewItem*)) );
}

/**
 * This function quickly translates a QListViewItem to its category and
 * subcategory. It then emits selectionChanged( category, subcategory ).
 */
void PortageTreeView::emitSelectionChanged( QListViewItem* item )
{
	if( portageTree == NULL )
		return;

	if( item->depth() == 0 ) // the root item
	{
		// this is easy, no filter at all.
		emit selectionChanged( portageTree, portageSettings,
		                       QString::null, QString::null );
		return;
	}
	else if( item->depth() == 1 ) // category match
	{
		// emit signal only with category filter
		emit selectionChanged( portageTree, portageSettings,
		                       item->text(0), QString::null );
		return;
	}
	else // It must be one of the subcategory items then. So, strict filter.
	{
		emit selectionChanged( portageTree, portageSettings,
		                       item->parent()->text(0), item->text(0) );
		return;
	}
}

/**
 * Assign a PortageTree object (along with its settings) to this widget
 * and add category and subcategory items to the list view.
 */
void PortageTreeView::displayTree( PortageTree* tree,
                                   PortageSettings* settings )
{
	this->portageTree = tree;
	this->portageSettings = settings;
	QListViewItem *catItem, *subcatItem;
	QString categoryName, subcategoryName;

	categories.clear();
	this->clear();
	rootItem = new KListViewItem( this, ALLPACKAGESTEXT );
	rootItem->setExpandable( true );
	rootItem->setOpen( true );
	rootItem->setPixmap( 0, pxRootItem );

	PackageMap* packages = portageTree->packageMap();
	PackageMap::iterator packageIteratorEnd = packages->end();

	// Insert packages under their right categoryName in the listview
	for( PackageMap::iterator packageIterator = packages->begin();
	     packageIterator != packageIteratorEnd; ++packageIterator )
	{
		categoryName = (*packageIterator).category;

		if( categories.contains(categoryName) == false )
		{
			catItem = new KListViewItem( rootItem, categoryName );
			catItem->setExpandable( true );
			catItem->setPixmap( 0, pxCategoryItem );
			categories[categoryName].item = catItem;
		}

		subcategoryName = (*packageIterator).subcategory;

		if( categories[categoryName].subcategories.contains(subcategoryName)
		    == false )
		{
			subcatItem = new KListViewItem(
				categories[categoryName].item, subcategoryName );
			subcatItem->setPixmap( 0, pxCategoryItem );
			categories[categoryName].subcategories[subcategoryName] = subcatItem;
		}
	}
}
