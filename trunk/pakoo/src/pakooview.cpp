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
#include <portage/loader/portageinitialloader.h>
#include <portage/installer/emergeprocess.h>


// widgets
#include "actionview.h"
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
#include <qwidgetstack.h>

// KDE includes
#include <klibloader.h>
#include <kmessagebox.h>
#include <krun.h>
#include <kdebug.h>
#include <khtmlview.h>
#include <kapplication.h>

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

	// Overall layout

	QHBoxLayout* topLayout = new QHBoxLayout( this );
	topLayout->setAutoAdd( true );

	m_hSplitter = new QSplitter( this );
	m_hSplitter->setOpaqueResize( true );

	QToolBox* toolBox = new QToolBox( m_hSplitter, "toolBox" );

	m_vSplitter = new QSplitter( m_hSplitter );
	m_vSplitter->setOrientation( QSplitter::Vertical );
	m_vSplitter->setOpaqueResize( true );


	// Adding the individual widgets

	m_treeView = new PackageTreeView( this, "treeView", m_backend );

	int sectionIndex = toolBox->addItem( m_treeView, TREEVIEWTEXT );
	m_sectionIndexes[sectionIndex] = BrowseSection;

	sectionIndex = toolBox->addItem(
		new QLabel("Action View", 0, "tempactionlabel"), ACTIONVIEWTEXT );
	m_sectionIndexes[sectionIndex] = ActionSection;
	toolBox->setCurrentIndex( 0 );

	sectionIndex = toolBox->addItem(
		new QLabel("Config View", 0, "tempconfiglabel"), CONFIGVIEWTEXT );
	m_sectionIndexes[sectionIndex] = ConfigSection;

	m_viewAreas = new QWidgetStack( m_vSplitter, "viewArea" );
	m_packageView = new PackageView( m_viewAreas, "packageView", m_backend );
	m_actionArea = new QWidgetStack( m_viewAreas, "actionArea" );
	JobView* jobView = new JobView( new EmergeProcess(), "title", m_actionArea, "jobView" );
	jobView->start();
	m_configArea = new QWidgetStack( m_viewAreas, "configArea" );
	new QLabel( "Config View", m_viewAreas, "configView" );
	m_viewAreas->raiseWidget( m_actionArea );

	m_packageInfoView = new PackageInfoView( m_vSplitter, "packageInfoView" );

	m_hSplitter->setResizeMode( toolBox, QSplitter::KeepSize );
	m_vSplitter->setResizeMode( m_packageInfoView->view(), QSplitter::KeepSize );

	m_vSplitter->setSizes( PakooConfig::vSplitterSizes() );
	m_hSplitter->setSizes( PakooConfig::hSplitterSizes() );


	//
	// Here comes the big connection creator
	//

	// Connect the QToolBox signals so the right side follows the left one
	connect(
		toolBox, SIGNAL( currentChanged(int) ),
		this,      SLOT( showSection(int) )
	);

	// Connect the package displaying widgets to work together
	connect(
		m_treeView,            SIGNAL( packageListChanged(PackageList&) ),
		m_packageView->listView, SLOT( setPackageList(PackageList&) )
	);
	connect(
		m_treeView,            SIGNAL( selectionChanged(PackageSelector&) ),
		m_packageView->listView, SLOT( setPackageSelector(PackageSelector&) )
	);
	connect(
		m_packageView->listView, SIGNAL( selectionChanged(Package*) ),
		m_packageInfoView, SLOT( displayPackage(Package*) )
	);
	connect(
		m_packageView->listView, SIGNAL(selectionChanged(Package*, PackageVersion*)),
		m_packageInfoView, SLOT(displayPackage(Package*, PackageVersion*))
	);

	// Connect the convenience method for copying text,
	// like pointed out by Scott Wheeler on the kde-core mailing list
	KStdAction::copy( KApplication::kApplication(), SLOT(copy()), NULL );

	// Connect the package list view with the status bar,
	// so the latter one is updated properly
	//TODO: adapt
	/*connect(
		m_packageView->listView, SIGNAL(loadingPackageInfo(int,int)),
		this, SLOT(handleLoadingPackageInfo(int,int))
	);
	connect(
		m_packageView->listView, SIGNAL(finishedLoadingPackageDetails(PackageList&)),
		this, SLOT(handleFinishedLoadingPackageDetails(PackageList&))
	);*/
	m_viewAreas->raiseWidget( m_packageView );
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
	         m_treeView,      SLOT( setPackageList(PackageList*) )
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
 * Show the section belonging to the QToolBox index.
 */
void PakooView::showSection( int sectionIndex )
{
	if( !m_sectionIndexes.contains(sectionIndex) )
		return;

	switch( m_sectionIndexes[sectionIndex] )
	{
	case BrowseSection:
		m_viewAreas->raiseWidget( m_packageView );
		break;
	case ActionSection:
		m_viewAreas->raiseWidget( m_actionArea );
		break;
	case ConfigSection:
		m_viewAreas->raiseWidget( m_configArea );
		break;
	}
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
	m_packageView->listView->abortLoadingPackageDetails();
}

/**
 * Prepare for quitting (closing the window).
 */
void PakooView::quit()
{
	// store UI configuration
	PakooConfig::setHSplitterSizes( m_hSplitter->sizes() );
	PakooConfig::setVSplitterSizes( m_vSplitter->sizes() );
	PakooConfig::writeConfig();
}

#include "pakooview.moc"
