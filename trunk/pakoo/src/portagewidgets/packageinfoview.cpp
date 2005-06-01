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
 * @param scanner  A copy of this scanner will be kept
 *                 to retrieve detailed package info.
 */
PackageInfoView::PackageInfoView(
	QWidget* parentWidget, const char* widgetname, PackageScanner* scanner )
: KHTMLPart(parentWidget, widgetname)
//: KTextBrowser(parentWidget, widgetname)
{
	packageScanner = new PackageScanner(scanner);
}

/**
 * Deconstruct this object.
 */
PackageInfoView::~PackageInfoView()
{
	this->quit();
}

/**
 * Prepare for deconstructing. Involves stopping threads and stuff.
 */
void PackageInfoView::quit()
{
	if( packageScanner != NULL )
	{
		if( packageScanner->running() ) {
			packageScanner->abort();
			packageScanner->wait();
		}
		delete packageScanner;
		packageScanner = NULL;
	}
}

/**
 * Display information about a whole package.
 */
void PackageInfoView::displayPackage( Package* package )
{
	if( packageScanner == NULL )
		return;

	this->package = package;
	this->version = NULL;

	if( packageScanner->running() ) {
		packageScanner->abort();
		packageScanner->wait();
	}
	packageScanner->startScanningPackage( this, package );
}

/**
 * Display information about a specific package version.
 */
void PackageInfoView::displayPackage( Package* package, PackageVersion* version )
{
	if( packageScanner == NULL )
		return;

	this->package = package;
	this->version = version;

	if( packageScanner->running() ) {
		packageScanner->abort();
		packageScanner->wait();
	}
	packageScanner->startScanningPackage( this, package );
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
		contents = getHTML( package );
	else
		contents = getHTML( package, version );

	//* KHTMLPart version
	this->begin();
	this->write( contents );
	this->end();

	/*/ KTextBrowser version
	this->clear();
	this->append( contents );
	//*/
}


/**
 * Construct hypertext only from package information.
 * The package object argument is assumed not to be NULL.
 */
QString PackageInfoView::getHTML( Package* package )
{
	QString contents;

	if( !package->hasVersions() ) {
		contents =
			"<html><body><strong>" + package->name + "</strong><br/>"
			+ package->category + "-" + package->subcategory
			+ "</body></html>";
	}
	else
	{
		QValueList<PackageVersion> versions = package->sortedVersionList();
		PackageVersion& firstVersion = *(versions.begin());

		QString versionsString;
		for( uint i = 0; i < versions.count(); i++ )
		{
			versionsString += versions[i].version + " ";
		}

		contents =
			"<html><body><span style=\"font-size:x-small;margin:-10px;\"><p>"
			/*+ package->name + "</strong> ("*/ + package->category + "-"
			+ package->subcategory /*+ ")<br/>"*/ + " / <strong>" + package->name + "</strong><br/>"
			+ firstVersion.description + "</p><p>" + versionsString +
			+ "</p></span></body></html>";
	}
	return contents;
}

/**
 * Construct hypertext from package and version information.
 * Both argument objects are assumed not to be NULL.
 */
QString PackageInfoView::getHTML( Package* package,
                                       PackageVersion* version )
{
	QString contents =
		"<html><body><span style=\"font-size:small\"><strong>"
		+ package->name + "-" + version->version + "</strong> ("
		+ package->category + "-" + package->subcategory + ")<br/>"
		+ version->description + "</span></body></html>";

	return contents;
}


/**
 * Receiver for package loading events (and others, if needed).
 */
void PackageInfoView::customEvent( QCustomEvent* event )
{
	if( event->type() == (int) LoadingPackageComplete )
	{
		LoadingPackageCompleteEvent* packageEvent =
			(LoadingPackageCompleteEvent*) event;

		if( packageEvent->error != PortageLoaderBase::NoError )
			return;
		if( packageEvent->package != this->package )
			return; // this is not the package that should be shown at the moment

		displayScannedPackage();
	}
}
