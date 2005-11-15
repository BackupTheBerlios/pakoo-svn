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

#include "packagetreeview.h"

#include <kiconloader.h>

#include <backendfactory.h>
#include <base/core/packagelist.h>
#include <base/core/packagecategory.h>
#include <base/core/packageselector.h>

#include "pixmapnames.h"
#include "i18n.h"


#define COLUMNHEADERSTRING \
	i18n("PackageTreeView column header string", "Categories")


namespace libpakt {

/**
 * Initialize this object.
 *
 * @param backend  The package management backend which can create
 *                 backend specific objects.
 */
PackageTreeView::PackageTreeView( QWidget* parent, const char* name,
                                  BackendFactory* backend )
 : KListView( parent, name )
{
	this->backend = backend;

	// load root and category item icons
	KIconLoader* iconLoader = KGlobal::iconLoader();
	pxCategoryRootItem = iconLoader->loadIcon( ALLPACKAGESICON, KIcon::Small );
	pxCategoryItem = iconLoader->loadIcon( CATEGORYICON, KIcon::Small );

	addColumn( COLUMNHEADERSTRING );
	setColumnWidthMode( 0, QListView::Maximum );
	setRootIsDecorated( true );
	setFullWidth( true );

	categoryRootItem.item = new KListViewItem( this, ALLPACKAGESTEXT );
	categoryRootItem.item->setExpandable( true );
	categoryRootItem.item->setOpen( true );
	categoryRootItem.item->setPixmap( 0, pxCategoryRootItem );

	connect( this, SIGNAL(selectionChanged(QListViewItem*)),
	         this, SLOT(emitSelectionChanged(QListViewItem*)) );
}

/**
 * This function quickly translates a QListViewItem to its category and
 * subcategory. It then emits selectionChanged( category, subcategory ).
 */
void PackageTreeView::emitSelectionChanged( QListViewItem* item )
{
	if( backend == NULL ) {
		//TODO: debug output needed
		return;
	}

	/*
	if( rootItem(item)->text(0) == searchRootItem->text(0) )
	{
		emit searchSelected( const QString& ? );
		// or
		emit newSearchSelected();
	}
	*/

	if( rootItem(item)->text(0) == categoryRootItem.item->text(0) )
	{
		PackageCategory* category = backend->createPackageCategory();

		for( QListViewItem* currentItem = item;
		     currentItem->depth() > 0; currentItem = currentItem->parent() )
		{
			category->prepend( currentItem->text(0) );
		};

		// set up a selector able to filter exactly this category's packages
		PackageSelector* selector = backend->createPackageSelector();
		selector->setAllPackagesFilter( PackageSelector::Exclude );
		selector->addCategoryFilter( PackageSelector::Include, *category );

		emit selectionChanged( *selector );

		delete selector;
		delete category;
	}
}

/**
 * Assign a PackageList object (along with its settings) to this widget
 * and add the appropriate category items to the list view.
 */
void PackageTreeView::setPackageList( PackageList* packages )
{
	if( backend == NULL ) {
		//TODO: debug output, like also needed further above
		return;
	}

	clear();

	// make the category root item
	categoryRootItem.item = new KListViewItem( this, ALLPACKAGESTEXT );
	categoryRootItem.item->setOpen( true );
	categoryRootItem.item->setPixmap( 0, pxCategoryRootItem );

	// Insert all existing category names. This iteration can dynamically
	// create hierarchical structures of any depth, according to which
	// categories the packages are in.

	PackageList::iterator packageIteratorEnd = packages->end();
	PackageCategory* cat;
	ParentItem* currentParentItem;
	QListViewItem* newItem;

	for( PackageList::iterator packageIterator = packages->begin();
	     packageIterator != packageIteratorEnd; ++packageIterator )
	{
		// get the current package's category
		cat = (*packageIterator)->category();

		// start off with the topmost category, which is all packages
		currentParentItem = &categoryRootItem;

		// iterate through all the category strings
		// e.g. first item is "app", next one is "portage"
		for( PackageCategory::const_iterator categoryIterator = cat->begin();
		     categoryIterator != cat->end(); categoryIterator++ )
		{
			// every existing child category gets an item with the same name
			if( !currentParentItem->children.contains(*categoryIterator) )
			{
				// if we get here, the category item has not yet been created

				// tell the parent that it has children
				currentParentItem->item->setExpandable( true );

				// make a new item with the current category text, like "app"
				newItem = new KListViewItem(
					currentParentItem->item, *categoryIterator );
				newItem->setPixmap( 0, pxCategoryItem );

				// auto-generate the new child structure
				// and add the new item to it
				currentParentItem->children.insert(
					*categoryIterator,
					KSharedPtr<ParentItem>( new ParentItem )
				);
				currentParentItem->children[*categoryIterator]->item =
					newItem;
			}
			// go down into the child category and repeat
			currentParentItem =
				currentParentItem->children[*categoryIterator];
		}
	}

	emit packageListChanged( *packages );
	// do this?:
	//this->setSelected( categoryRootItem.item, true );
}

/**
 * Reimplemented for also clearing internal data structures.
 */
void PackageTreeView::clear()
{
	KListView::clear();
	categoryRootItem.item = NULL;
	categoryRootItem.children.clear();
}

/**
 * Retrieves the topmost successor of an item.
 * That can be the item itself, or its parent, or its grandparent, or...
 *
 * The returned item has has depth() == 0.
 */
QListViewItem* PackageTreeView::rootItem( QListViewItem* item )
{
	QListViewItem* currentItem = item;

	while( currentItem->depth() != 0 ) {
		currentItem = currentItem->parent();
	}
	return currentItem;
}

} // namespace
