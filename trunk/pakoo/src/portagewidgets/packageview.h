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

#ifndef PAKOOPACKAGEVIEW_H
#define PAKOOPACKAGEVIEW_H

#include <qvbox.h>
#include <qcombobox.h>

#include <kactioncollection.h>

#include "packagelistview.h"
#include "packagesearchline.h"


namespace libpakt {

/**
 * PackageView is a combination of a PackageListView and an
 * associated PakooPackageSearchLine.
 *
 * @short A KListViewSearchLine customized to work with a PackageListView.
 */
class PackageView : public QVBox
{
	Q_OBJECT
public:
	PackageView( QWidget* parent, const char* name, BackendFactory* backend );
	~PackageView();

	PackageListView* listView;
	PackageSearchLine* searchLine;

public slots:
	void resetSearchLine();
	void updateFilter( int comboIndex );

protected:
	QComboBox* filterCombo;
};

}

#endif // PAKOOPACKAGEVIEW_H
