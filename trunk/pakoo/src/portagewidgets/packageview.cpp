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

#include <qvbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qapplication.h>

#include <ktoolbar.h>


/**
 * Initialize this object.
 */
PackageView::PackageView( QWidget* parent, const char* name,
                                    PackageScanner* scanner )
: QVBox( parent, name )
{
	QString qname(name);

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
	searchLine = new PakooPackageSearchLine( searchToolBar, NULL, qname + "_searchLine" );
	searchToolBar->setStretchableWidget( searchLine );
	searchLabel->setBuddy( searchLine );

	// Combobox that can filter the package view, and its label
	QLabel* statusLabel = new QLabel( STATUSTEXT, searchToolBar, qname + "_statusLabel" );
	filterCombo = new QComboBox( searchToolBar, qname + "_filterCombo" );
	filterCombo->insertItem( ALLPACKAGESTEXT );
	filterCombo->insertItem( ONLYINSTALLEDTEXT );
	statusLabel->setBuddy( filterCombo );

	// The list view containing the packages of the current category
	listView = new PakooPackageListView( this, qname + "_listView", scanner );
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
	this->quit();
	searchLine->setListView( NULL );
}

/**
 * Prepare for deconstructing. Involves stopping threads and stuff.
 */
void PackageView::quit()
{
	listView->quit();
}

/**
 * Set the searchLine's filter to the new value of the filter combo box.
 */
void PackageView::updateFilter( int comboIndex )
{
	switch( comboIndex )
	{
	case 0: // "All Packages"
		searchLine->setFilter( PakooPackageSearchLine::All );
		break;
	case 1: // "Installed"
		searchLine->setFilter( PakooPackageSearchLine::Installed );
		break;
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

