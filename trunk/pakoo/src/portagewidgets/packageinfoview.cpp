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

#include "packageinfoview.h"


/**
 * Initialize this object.
 * @param arch  The architecture for which the info view should be specialized
 *              on - something like "x86" or "~alpha"
 */
PackageInfoView::PackageInfoView(
	QWidget* parentWidget, const char* widgetname, const QString& arch )
: KHTMLPart(parentWidget, widgetname)
//: KTextBrowser(parentWidget, widgetname)
{
	htmlGenerator = new PortageHTMLGenerator(arch);
}

/**
 * Deconstruct this object.
 */
PackageInfoView::~PackageInfoView()
{
	delete htmlGenerator;
}

/**
 * Set the architecture that this info view specializes in.
 * Default is "x86".
 */
void PackageInfoView::setArchitecture( const QString& arch )
{
	htmlGenerator->setArchitecture( arch );
}

/**
 * Display information about a whole package that has already been loaded.
 */
void PackageInfoView::displayPackage( Package* package )
{
	this->package = package;
	this->version = NULL;

	displayScannedPackage();
}

/**
 * Display information about a specific package version.
 */
void PackageInfoView::displayPackage( Package* package, PackageVersion* version )
{
	// same as in displayPackage(Package*)

	this->package = package;
	this->version = version;

	displayScannedPackage();
}


/**
 * Display information about a whole package which has already been scanned.
 */
void PackageInfoView::displayScannedPackage()
{
	if( package == NULL )
		return;

	QString contents;

	if( version == NULL )
		contents = htmlGenerator->fromPackage( package, NULL );
	else
		contents = htmlGenerator->fromPackage( package, version );

	//* KHTMLPart version
	this->begin();
	this->write( contents );
	this->end();

	/*/
	// KTextBrowser version
	this->clear();
	this->append( contents );
	//*/
}
