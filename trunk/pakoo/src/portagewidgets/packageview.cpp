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

#include "packageview.h"
#include "pixmapnames.h"
#include "i18n.h"

#include <core/packageselector.h>

#include <qvbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qapplication.h>

#include <ktoolbar.h>


namespace libpakt {

/**
 * Initialize this object.
 *
 * @param backend  The package management backend which can create
 *                 backend specific objects.
 */
PackageView::PackageView( QWidget* parent, const char* name,
                          BackendFactory* backend )
: QVBox( parent, name )
{
	QString qname = name;

	// We need a KToolBar to have that fancy look for the clear button
	KToolBar* searchToolBar = new KToolBar( this, "searchBar" );
	searchToolBar->boxLayout()->setSpacing( 3 );

	searchToolBar->insertButton(
		QApplication::reverseLayout() ? CLEARICON_RTL : CLEARICON, 1/*id*/,
		SIGNAL( clicked() ), this, SLOT( resetSearchLine() ), true/*enabled*/,
		CLEARTEXT, 0/*index*/
	);

	// Label and SearchLine
	QLabel* searchLabel = new QLabel( SEARCHTEXT, searchToolBar, qname + "_searchLabel" );
	searchLine = new PackageSearchLine( searchToolBar, NULL, qname + "_searchLine" );
	searchToolBar->setStretchableWidget( searchLine );
	searchLabel->setBuddy( searchLine );

	// Combobox that can filter the package view, and its label
	QLabel* statusLabel = new QLabel( STATUSTEXT, searchToolBar, qname + "_statusLabel" );
	filterCombo = new QComboBox( searchToolBar, qname + "_filterCombo" );
	filterCombo->insertItem( ALLPACKAGESTEXT );
	filterCombo->insertItem( ONLYINSTALLEDTEXT );
	filterCombo->insertItem( ONLYNOTINSTALLEDTEXT );
	statusLabel->setBuddy( filterCombo );

	// The list view containing the packages of the current category
	listView = new PackageListView( this, qname + "_listView", backend );
	searchLine->setListView( listView );

	QValueList<int> searchColumns;
	searchColumns.append( 0 );
	searchLine->setSearchColumns( searchColumns );

	connect( filterCombo, SIGNAL(activated(int)), this, SLOT(updateFilter(int)) );
	connect( listView, SIGNAL(cleared()), searchLine, SLOT(clear())  );
	connect( listView, SIGNAL(contentsChanged()),
	         searchLine, SLOT(updateSearch())   );
	/*
	connect( listView, SIGNAL(cleared()),
	         this, SLOT(enableFilterUpdatable(false)) );
	connect( listView, SIGNAL(finishedLoadingInstalledPackageInfo()),
	         this, SLOT(enableFilterUpdatable(true))                );
	*/
}

/**
 * Deconstruct this object.
 */
PackageView::~PackageView()
{
	searchLine->setListView( NULL );
}

/**
 * Set the searchLine's filter to the new value of the filter combo box.
 */
void PackageView::updateFilter( int comboIndex )
{
	switch( comboIndex )
	{
	case 0: // "All Packages"
		listView->packageSelector()->clearIsInstalledFilters();
		listView->refreshView();
		//searchLine->setFilter( PackageSearchLine::All );
		break;
	case 1: {// "Installed"
		PackageSelector* selector = listView->packageSelector();
		selector->clearIsInstalledFilters();
		selector->addIsInstalledFilter( PackageSelector::Exclude, false );
		selector->addIsInstalledFilter( PackageSelector::Include, true );
		listView->refreshView();
		//searchLine->setFilter( PackageSearchLine::Installed );
		break;
	}
	case 2: {
		PackageSelector* selector = listView->packageSelector();
		selector->clearIsInstalledFilters();
		selector->addIsInstalledFilter( PackageSelector::Exclude, true );
		selector->addIsInstalledFilter( PackageSelector::Include, false );
		listView->refreshView();
	}
	}
	searchLine->updateSearch();
}

/**
 * Clear the search line and have the list view updated.
 */
void PackageView::resetSearchLine()
{
	searchLine->setText("");
	searchLine->updateSearch();
}

} // namespace
