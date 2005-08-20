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

// libpakt
#include <portagebackend.h>
#include <portageloader/portageinitialloader.h>


// widgets
#include "portagewidgets/packagetreeview.h"
#include "portagewidgets/packageview.h"
#include "portagewidgets/packageinfoview.h"

// stuff
#include "pakooconfig.h"
#include "i18n.h"

// Qt includes
#include <qpainter.h>
#include <qlayout.h>
#include <qtoolbox.h>

// KDE includes
#include <klibloader.h>
#include <kmessagebox.h>
#include <krun.h>
#include <kdebug.h>
#include <khtmlview.h>

// TODO: remove (used to make stubs in the QToolBox)
#include <qlabel.h>

using namespace libpakt;


/**
 * Initialize this object.
 */
PakooView::PakooView( QWidget *parent )
: DCOPObject("pakooIface"), QWidget(parent)
{
	m_backend = new PortageBackend();

	QHBoxLayout* topLayout = new QHBoxLayout( this );
	topLayout->setAutoAdd( true );

	hSplitter = new QSplitter( this );
	hSplitter->setOpaqueResize( true );

	QToolBox* toolBox = new QToolBox( hSplitter, "toolBox" );

	treeView = new PackageTreeView( 0, "treeView", m_backend );
	toolBox->addItem( treeView, TREEVIEWTEXT );
	toolBox->addItem( new QLabel("Action View", 0, "tempactionlabel"), ACTIONVIEWTEXT );
	toolBox->addItem( new QLabel("Config View", 0, "tempconfiglabel"), CONFIGVIEWTEXT );

	vSplitter = new QSplitter( hSplitter );
	vSplitter->setOrientation(QSplitter::Vertical);
	vSplitter->setOpaqueResize( true );

	packageView = new PackageView( vSplitter, "packageView", m_backend );

	packageInfoView = new PackageInfoView( vSplitter, "packageInfoView" );

	hSplitter->setResizeMode( toolBox, QSplitter::KeepSize );
	vSplitter->setResizeMode( packageInfoView->view(), QSplitter::KeepSize );

	vSplitter->setSizes( PakooConfig::vSplitterSizes() );
	hSplitter->setSizes( PakooConfig::hSplitterSizes() );


	//
	// Here comes the big connection creator
	//

	// Connect the package displaying widgets to work together
	connect(
		treeView,            SIGNAL( packageListChanged(PackageList&) ),
		packageView->listView, SLOT( setPackageList(PackageList&) )
	);
	connect(
		treeView,            SIGNAL( selectionChanged(PackageSelector&) ),
		packageView->listView, SLOT( setPackageSelector(PackageSelector&) )
	);
	connect(
		packageView->listView, SIGNAL( selectionChanged(Package*) ),
		packageInfoView, SLOT( displayPackage(Package*) )
	);
	connect(
		packageView->listView, SIGNAL(selectionChanged(Package*, PackageVersion*)),
		packageInfoView, SLOT(displayPackage(Package*, PackageVersion*))
	);

	// Connect the package list view with the status bar,
	// so the latter one is updated properly
	//TODO: adapt
	/*connect(
		packageView->listView, SIGNAL(loadingPackageInfo(int,int)),
		this, SLOT(handleLoadingPackageInfo(int,int))
	);
	connect(
		packageView->listView, SIGNAL(finishedLoadingPackageDetails(PackageList&)),
		this, SLOT(handleFinishedLoadingPackageDetails(PackageList&))
	);*/
}

/**
 * Initialize the backend, tree structure, and stuff.
 */
void PakooView::initData()
{
	InitialLoader* initialLoader = m_backend->createInitialLoader();
	m_packages = m_backend->createPackageList();
	initialLoader->setPackageList( m_packages );

	// display the packages when loaded
	connect( initialLoader, SIGNAL( finishedLoading(PackageList*) ),
	         treeView,        SLOT( setPackageList(PackageList*) )
	);
	// hide the progress bar when it's done
	connect( initialLoader, SIGNAL( finishedLoading(PackageList*) ),
	         this,          SIGNAL( statusbarProgressHidden() )
	);
	// update the status bar message when needed
	connect( initialLoader, SIGNAL( currentTaskChanged(const QString&) ),
	         this,          SIGNAL( statusbarTextChanged(const QString&) )
	);
	// update the progress bar when needed
	connect( initialLoader, SIGNAL( progressChanged(int,int) ),
	         this,          SIGNAL( statusbarProgressChanged(int,int) )
	);
	// delete the initialLoader when it's done
	connect( initialLoader, SIGNAL( finished(IJob::JobResult) ),
	         initialLoader,   SLOT( deleteLater() )
	);

	initialLoader->start();
}

/**
 * Size hint for the central view.
 */
QSize PakooView::sizeHint() const
{
	return QSize( 750, 700 );
}

/**
 * Tell the package list view to stop loading package details.
 */
void PakooView::abortProgress()
{
	packageView->listView->abortLoadingPackageDetails();
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
}

#include "pakooview.moc"
