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

#include "pakooview.h"

#include "portagewidgets/portagetreeview.h"
#include "portagewidgets/packageview.h"
#include "portagewidgets/packageinfoview.h"

#include "pakooconfig.h"
#include "i18n.h"

#include <qpainter.h>
#include <qlayout.h>

#include <klibloader.h>
#include <kmessagebox.h>
#include <krun.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <khtmlview.h>


/**
 * Initialize this object.
 */
PakooView::PakooView( QWidget *parent )
: DCOPObject("pakooIface"), QWidget(parent)
{
	QHBoxLayout* topLayout = new QHBoxLayout(this);
	topLayout->setAutoAdd( true );

	vSplitter = new QSplitter(this);
	vSplitter->setOrientation(QSplitter::Vertical);
	vSplitter->setOpaqueResize( true );

	hSplitter = new QSplitter( vSplitter );
	hSplitter->setOpaqueResize( true );
	treeView = new PortageTreeView( hSplitter, "treeView" );
	packageView = new PackageView(
		hSplitter, "packageView", portageTreeScanner.packageScanner()
	);
	hSplitter->setResizeMode( treeView, QSplitter::KeepSize );

	packageInfoView = new PackageInfoView( vSplitter, "packageInfoView" );
	vSplitter->setResizeMode( packageInfoView->view(), QSplitter::KeepSize );

	vSplitter->setSizes( PakooConfig::vSplitterSizes() );
	hSplitter->setSizes( PakooConfig::hSplitterSizes() );


	//
	// Here comes the big connection creator
	//

	// Connect the package displaying widgets to work together
	connect(
		treeView,
		SIGNAL(selectionChanged(PortageTree*, PortageSettings*, const QString&, const QString&)),
		packageView->listView,
		SLOT(displayPackages(PortageTree*, PortageSettings*, const QString&, const QString&))
	);
	connect(
		packageView->listView, SIGNAL(selectionChanged(Package*)),
		packageInfoView, SLOT(displayPackage(Package*))
	);
	connect(
		packageView->listView, SIGNAL(selectionChanged(Package*, PackageVersion*)),
		packageInfoView, SLOT(displayPackage(Package*, PackageVersion*))
	);

	// Connect the package list view with the status bar,
	// so the latter one is updated properly
	connect(
		packageView->listView, SIGNAL(loadingPackageInfo(int,int)),
		this, SLOT(handleLoadingPackageInfo(int,int))
	);
	connect(
		packageView->listView, SIGNAL(finishedLoadingPackageInfo(int)),
		this, SLOT(handleFinishedLoadingPackageInfo(int))
	);
}

/**
 * Initialize the tree structure, and stuff.
 */
void PakooView::initData()
{
	packageCount  = 0;
	mainlineCount = 0;
	overlayCount  = 0;
	installedPackageCount = 0;

	loadPortageTree();
}

/**
 * Size hint for the central view.
 */
QSize PakooView::sizeHint() const
{
	return QSize( 750, 700 );
}

/**
 * Initiate Portage tree scanning.
 */
void PakooView::loadPortageTree()
{
	ProfileLoader profileLoader;
	profileLoader.loadProfile( &settings );

	packageInfoView->setArchitecture( settings.acceptedKeyword() );
	portageTreeScanner.setMainlineTreeDirectory(
		settings.mainlineTreeDirectory() );
	portageTreeScanner.setOverlayTreeDirectories(
		settings.overlayTreeDirectories() );
	portageTreeScanner.setInstalledPackagesDirectory(
		PakooConfig::installedPackagesDir() );
	portageTreeScanner.setEdbDepDirectory(
		PakooConfig::edbDir() );

	// if not using /var/cache/edb/dep, try to load the packages from a cache file
	if( PakooConfig::preferEdb() == false )
	{
		portageML.startLoadingFile(
			this, &portageTree, KGlobalSettings::documentPath() + "/portagetree.xml"
		);
	}
	else if( !portageTreeScanner.running() )
	{
		packageCount  = 0;
		mainlineCount = 0;
		overlayCount  = 0;
		installedPackageCount = 0;

		portageTree.clear();
		portageTreeScanner.startScanningTrees(
			this, &portageTree, PortageTree::All, PakooConfig::preferEdb()
		);
		QString message = i18n("Scanning the portage tree...");
		kdDebug() << message << endl;
		this->setStatusbarText(message);
	}
}

/**
 * Receiver for portage tree loading events (and others, if needed).
 */
void PakooView::customEvent( QCustomEvent* event )
{
	message = "";
	if( event->type() == (int) LoadingTreeProgress )
	{
		LoadingTreeProgressEvent* portageEvent =
			(LoadingTreeProgressEvent*) event;

		if( portageEvent->searchedTree == PortageTree::Mainline )
		{
			mainlineCount = portageEvent->packageCount;
			packageCount = mainlineCount + overlayCount;
			message = LOADINGTREEPACKAGESTEXT
				.arg( packageCount ).arg( installedPackageCount );
		}
		else if( portageEvent->searchedTree == PortageTree::Overlay )
		{
			overlayCount = portageEvent->packageCount;
			packageCount = mainlineCount + overlayCount;
			message = LOADINGTREEPACKAGESTEXT
				.arg( packageCount ).arg( installedPackageCount );
		}
		else if( portageEvent->searchedTree == PortageTree::Installed )
		{
			installedPackageCount = portageEvent->packageCount;
			message = LOADINGINSTALLEDPACKAGESTEXT
				.arg( packageCount ).arg( installedPackageCount );
		}
		this->setStatusbarText( message );
	}
	if( event->type() == (int) LoadingTreePartiallyComplete )
	{
		LoadingTreePartiallyCompleteEvent* portageEvent =
			(LoadingTreePartiallyCompleteEvent*) event;

		if( portageEvent->searchedTree == PortageTree::Mainline )
		{
			mainlineCount = portageEvent->packageCount;
			packageCount = mainlineCount + overlayCount;
			kdDebug() << i18n("Finished scanning mainline tree...")
				+ SECONDSTEXT.arg(portageEvent->secondsElapsed) << endl;
		}
		else if( ((LoadingTreePartiallyCompleteEvent*)event)->searchedTree
		         == PortageTree::Overlay )
		{
			overlayCount = portageEvent->packageCount;
			packageCount = mainlineCount + overlayCount;
			kdDebug() << i18n("Finished scanning overlay... ")
				+ SECONDSTEXT.arg(portageEvent->secondsElapsed) << endl;
		}
		else if( ((LoadingTreePartiallyCompleteEvent*)event)->searchedTree
		         == PortageTree::Installed )
		{
			installedPackageCount = portageEvent->packageCount;
			kdDebug() << i18n("Finished scanning installed packages... ")
				+ SECONDSTEXT.arg(portageEvent->secondsElapsed) << endl;
		}
	}
	if( event->type() == (int) LoadingTreeComplete )
	{
		handleLoadingTreeComplete( (LoadingTreeCompleteEvent*) event );
	}
}

/**
 * To be called if a LoadingTreeCompleteEvent has been received.
 */
void PakooView::handleLoadingTreeComplete( LoadingTreeCompleteEvent* event )
{
	if( event->error == PortageLoaderBase::NoError )
	{
		if( PakooConfig::preferEdb() == false
		    && event->method == LoadingTreeEvent::ScanPortageTree )
		{
			// save scanned packages to disk
			portageML.wait(); // whatever portageML is doing at the moment
			portageML.startSavingFile( this, &portageTree,
				KGlobalSettings::documentPath() + "/portagetree.xml"
			);
		}

		// Add custom masking information
		FilePackageMaskLoader maskLoader;
		maskLoader.loadFile( &portageTree,
			"/usr/portage/" + PakooConfig::profilesPackageMaskFile() );
		maskLoader.loadFile( &portageTree, PakooConfig::packageMaskFile() );
		maskLoader.setMode( FilePackageMaskLoader::Unmask );
		maskLoader.loadFile( &portageTree, PakooConfig::packageUnmaskFile() );
		FilePackageKeywordsLoader keywordsLoader;
		keywordsLoader.loadFile( &portageTree, PakooConfig::packageKeywordsFile() );

		packageCount = event->packageCount;
		installedPackageCount = event->packageCountInstalled;
		message = PACKAGESINCATEGORYTEXT
			.arg( packageCount )
			.arg( THEPORTAGETREETEXT )
			.arg( installedPackageCount )
			.arg( "" );
	}
	else // there was an error
	{
		if( event->method == LoadingTreeEvent::LoadFile )
		{
			// Doesn't happen if PakooConfig::preferEdb() == true
			kdDebug() << "Couldn't load portagetree.xml, "
				"so try to load the real tree..." << endl;
			this->loadPortageTree();
			return;
		}
		else {
			message = ERRORLOADINGTEXT;
		}
	}

	this->setStatusbarText( message );
	kdDebug() << message << endl;

	treeView->displayTree( &portageTree, &settings );
}

/**
 * Emitting signalSetStatusbarText(text).
 */
void PakooView::setStatusbarText( const QString& text )
{
	emit signalSetStatusbarText( text );
}

/**
 * Emitting signalSetStatusbarProgress(text).
 */
void PakooView::setStatusbarProgress( int progress, int totalSteps )
{
	emit signalSetStatusbarProgress( progress, totalSteps );
}

/**
 * Emitting signalHideStatusbarProgress(text).
 */
void PakooView::hideStatusbarProgress( bool hide )
{
	emit signalHideStatusbarProgress( hide );
}

/**
 * Emitting signalChangeCaption(title).
 */
void PakooView::setTitle( const QString& title )
{
	emit signalSetCaption( title );
}

/**
 * Update the status bar if the package list view has loaded all
 * package details.
 *
 * @param totalPackageCount  The number of packages in the package list view.
 */
void PakooView::handleFinishedLoadingPackageInfo( int totalPackageCount )
{
	QString category = packageView->listView->currentCategory();
	QString subcategory = packageView->listView->currentSubcategory();
	if( category == QString::null )
		category = THEPORTAGETREETEXT;
	else if( subcategory != QString::null )
		category += "-" + subcategory;

	emit hideStatusbarProgress();
	emit setStatusbarText(
		PACKAGESINCATEGORYTEXT
			.arg( totalPackageCount )
			.arg( category )
			.arg( packageView->listView->installedPackageCount() )
			.arg( "" )
	);
}

/**
 * Update the status bar while the package list view is loading
 * detailed package info.
 *
 * @param loadedPackageCount  The number of packages loaded so far.
 * @param totalPackageCount   The total number of packages in the list view.
 */
void PakooView::handleLoadingPackageInfo( int loadedPackageCount,
                                          int totalPackageCount )
{
	QString category = packageView->listView->currentCategory();
	QString subcategory = packageView->listView->currentSubcategory();
	if( category == QString::null )
		category = THEPORTAGETREETEXT;
	else if( subcategory != QString::null )
		category += "-" + subcategory;

	emit setStatusbarProgress( loadedPackageCount, totalPackageCount );
	emit setStatusbarText(
		PACKAGESINCATEGORYTEXT
			.arg( totalPackageCount )
			.arg( category )
			.arg( packageView->listView->installedPackageCount() )
			.arg( LOADINGPACKAGEDETAILSTEXT )
	);
}

/**
 * Prepare for quitting (closing the window).
 */
void PakooView::quit()
{
	// store UI configuration
	PakooConfig::setHSplitterSizes(hSplitter->sizes());
	PakooConfig::setVSplitterSizes(vSplitter->sizes());
	PakooConfig::writeConfig();

	if( portageML.running() ) {
		portageML.abort();
		portageML.wait();
	}

	if( portageTreeScanner.running() ) {
		portageTreeScanner.abort();
		portageTreeScanner.wait();
	}

	packageView->quit();
}

#include "pakooview.moc"
