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

#ifndef PACKAGEINFOVIEW_H
#define PACKAGEINFOVIEW_H

#include <khtml_part.h>
//#include <ktextbrowser.h>

#include "../libqortage/libqortage.h"
#include "portagehtmlgenerator.h"


namespace libpakt {

/**
 * A KHTMLPart with additional functions to display package info.
 *
 * @short Widget to display description and other information about a package.
 */
class PackageInfoView : public KHTMLPart // public KTextBrowser
{
	Q_OBJECT

public:
	PackageInfoView( QWidget* parentWidget = 0, const char* widgetname = 0,
	                 const QString& arch = "x86" );
	~PackageInfoView();

public slots:
	void setArchitecture( const QString& arch );
	void displayPackage( Package* package );
	void displayPackage( Package* package, PackageVersion* version );

protected:
	void displayScannedPackage();

	//! The package that will be shown in the widget.
	Package* package;
	//! Not NULL if a specific version of the package will be shown in detail.
	PackageVersion* version;
	//! The object that produces the appropriate HTML that is displayed.
	PortageHTMLGenerator* htmlGenerator;

protected slots:
	void openURLRequest( const KURL& url, const KParts::URLArgs& args );
};

}

#endif // PACKAGEINFOVIEW_H
