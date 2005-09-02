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

#include "packagelistview.h"
#include "pixmapnames.h"

#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>

#include <backendfactory.h>
#include <core/packagelist.h>
#include <core/package.h>
#include <core/packageversion.h>
#include <core/packagecategory.h>
#include <core/packageselector.h>
#include <portageloader/packageloader.h>
#include <portageloader/multiplepackageloader.h>


namespace libpakt {

/**
 * Initialize this object.
 *
 * @param backend  The package management backend which can create
 *                 backend specific objects.
 */
PackageListView::PackageListView( QWidget* parent, const char* name,
                                  BackendFactory* backend )
: KListView( parent, name )
{
	m_backend = backend;
	// if backend is NULL or illegal, let it crash now in the constructor

	//TODO: get arch outta here
	m_arch = "x86";

	m_packageLoader = m_backend->createPackageLoader();
	m_multiplePackageLoader = m_backend->createMultiplePackageLoader(
		m_backend->createPackageLoader() );
	m_multiplePackageLoader->setAutoDeletePackageLoader( true );

	// Display the details of newly loaded packages.
	connect(
		m_packageLoader, SIGNAL( packageLoaded(Package*) ),
		this,            SLOT( displayPackageDetails(Package*) )
	);
	connect(
		m_multiplePackageLoader->packageLoader(),
			  SIGNAL( packageLoaded(Package*) ),
		this, SLOT( displayPackageDetails(Package*) )
	);

	// Emit selectionChanged(Package*) when a package is loaded
	// by packageLoader. (packageLoader is started only and every time
	// when a package item has been selected.)
	connect(
		m_packageLoader, SIGNAL( packageLoaded(Package*) ),
		this,            SIGNAL( selectionChanged(Package*) )
	);

	//m_parallelScanning = true; // you may set this to false when debugging
	//if( m_parallelScanning == true ) {
		//TODO: bring back?
		//packageCategoryScanner->setFilterInstalled( true, false );
		//packageInstalledScanner->setFilterInstalled( true, true );
	//}

	m_allPackages = m_backend->createPackageList();
	m_shownPackages = m_backend->createPackageList();
	m_currentPackage = NULL;

	m_packageSelector = m_backend->createPackageSelector();

	m_loadedPackageCount = 0;
	m_installedPackageCount = 0;
	m_totalPackageCount = 0;

	// ListView stuff
	this->addColumn( i18n("PackageListView column #1", "Package") );
	this->addColumn( i18n("PackageListView column #2", "Description") );
	this->setRootIsDecorated( true );
	this->setFullWidth( true );

	// Load root and category item icons
	KIconLoader* iconLoader = KGlobal::iconLoader();
	pxCategoryItem = iconLoader->loadIcon( CATEGORYICON, KIcon::Small );
	pxPackageItem = iconLoader->loadIcon( PACKAGEICON, KIcon::Small );
	pxPackageItemInstalled = iconLoader->loadIcon( PACKAGEICON_INSTALLED, KIcon::Small );
	pxPackageItemUpdatable = iconLoader->loadIcon( PACKAGEICON_UPDATABLE, KIcon::Small );
	pxVersionItem = iconLoader->loadIcon( VERSIONICON, KIcon::Small );
	pxVersionItemInstalled = iconLoader->loadIcon( VERSIONICON_INSTALLED, KIcon::Small );
	pxVersionItemNotAvailable = iconLoader->loadIcon( VERSIONICON_NOTAVAILABLE, KIcon::Small );

	// Translate item selections into package[,version] selection signal.
	connect(
		this, SIGNAL( selectionChanged(QListViewItem*)   ),
		this, SLOT( emitSelectionChanged(QListViewItem*) )
	);

	// Add the versions only when the user wants to see them, which brings
	// a) slightly better performance, and b) better column auto-resizing.
	connect(
		this, SIGNAL( expanded(QListViewItem*)         ),
		this, SLOT( insertVersionItems(QListViewItem*) )
	);
}

/**
 * The deconstructor aborts and waits for the PackageScanner,
 * if it's running, so that it can safely be deleted.
 */
PackageListView::~PackageListView()
{
	if( m_packageSelector != NULL ) {
		delete m_packageSelector;
	}
	if( m_packageLoader != NULL ) {
		delete m_packageLoader;
	}
	if( m_multiplePackageLoader != NULL ) {
		delete m_multiplePackageLoader;
	}
	if( m_allPackages != NULL ) {
		delete m_allPackages;
	}
	if( m_shownPackages != NULL ) {
		delete m_shownPackages;
	}
}

/**
 * Retrieve the current PackageSelector which determines the
 * shown packages in the ListView.
 */
PackageSelector* PackageListView::packageSelector()
{
	return m_packageSelector;
}

/**
 * Get the number of installed packages which are currently shown
 * in the list view.
 */
int PackageListView::installedPackageCount()
{
	return m_installedPackageCount;
}

/**
 * Get the total number of packages which are currently shown
 * in the list view.
 */
int PackageListView::totalPackageCount()
{
	return m_totalPackageCount;
}

/**
 * Set the list of all packages in the package tree and an appropriate
 * PackageSelector object. The PackageSelector is used to determine
 * the packages that are actually shown in this ListView.
 *
 * This function usually has to be called only once, when the package
 * list has been initialized. To switch to another selection within
 * the same package list (e.g. when another category is selected),
 * you can call setPackageSelector.
 *
 * Calling this function will not cause an update of the view.
 * If you want that, please use an additional call of refreshView().
 *
 * @see setPackageSelector
 * @see refreshView
 */
void PackageListView::setPackageList( PackageList& allPackages )
{
	*m_allPackages = allPackages;
}

/**
 * Set a new selection of packages in the list view. The given PackageSelector
 * object should already have its filters set up so that it can determine
 * which packages should be shown. The shown package items will be a subset
 * of the previously set package list.
 *
 * Calling this function also causes the ListView to clear and refill
 * its package items.
 *
 * @see setPackageList
 */
void PackageListView::setPackageSelector( PackageSelector& packageSelector )
{
	*m_packageSelector = packageSelector;
	emit packageSelectorChanged( m_packageSelector );
	refreshView();
}


/**
 * Determine if a package (item) in this list view has at least one
 * installed version. Returns false for version and category items.
 */
bool PackageListView::hasInstalledVersion( const QListViewItem* packageItem )
{
	if( packageItem->depth() != 1 ) { // we got a version or category item
		return false;
	}
	else // we got a package item, depth 1
	{
		// retrieve the package and category strings from the ListView
		const QString& categoryString = packageItem->parent()->text(0);

		// see if the category exists
		if( m_categories.contains(categoryString) == false )
			return false;

		// the category exists, see if the package exists
		const QString& packageString = packageItem->text(0);
		PackageViewCategory& cat = m_categories[categoryString];
		if( cat.packageItems.contains(packageString) == false )
			return false;

		// the package exists, return if it's installed
		return cat.packageItems[packageString].installed;
	}
}

/**
 * Translate item selections into package[,version] selection signal.
 * @param item  The item that has been selected.
 */
void PackageListView::emitSelectionChanged( QListViewItem* item )
{
	QListViewItem* categoryItem;

	if( item->depth() == 0 ) { // we got a category item
		categoryItem = item;
	}
	else if( item->depth() == 1 ) { // we got a package item
		categoryItem = item->parent();
	}
	else { // nothing of the previous ones, so it's a version item
		categoryItem = item->parent()->parent();
	}

	PackageCategory* packageCategory = m_backend->createPackageCategory();
	//TODO: We want user visible names, and not this:
	packageCategory->loadFromUniqueName( categoryItem->text(0) );
	PackageCategory* category; // which will be a copy of the above

	// emit the right signal
	if( item->depth() == 0 )  // we got a category item
	{
		emit selectionChanged( category );
	}
	else if( item->depth() == 1 ) // we got a package item
	{
		const QString& packageName = item->text(0);
		category = m_backend->createPackageCategory();
		*category = *packageCategory;
		Package* package = m_shownPackages->package( category, packageName );

		// Retrieve the package's detail info (description and hasUpdates).
		// The signal is emitted when it's done - mind the connection which
		// has been set up in the constructor.
		m_packageLoader->setPackage( package );
		m_packageLoader->start();
	}
	else // nothing of the previous ones, so it's a version item
	{
		const QString& packageName = item->parent()->text(0);
		category = m_backend->createPackageCategory();
		*category = *packageCategory;
		Package* package = m_shownPackages->package( category, packageName );

		const QString& versionString = item->text(0);
		if( package->containsVersion(versionString) )
		{
			PackageVersion* version = package->version( versionString );
			emit selectionChanged( package, version );
		}
		else { // should not happen, but just to make sure
			emit selectionChanged( package );
		}
	}
}

/**
 * Stop scanning the appropriate files for package details.
 * This stops harddisk reading activity, with the effect
 * that remaining package descriptions are not loaded
 * until displayPackages() is called again.
 */
void PackageListView::abortLoadingPackageDetails()
{
	m_packageLoader->abortAndWait();
	m_multiplePackageLoader->abortAndWait();
}

/**
 * Clear and refill the ListView with those package items that
 * are defined by the list of all packages and the PackageSelector.
 */
void PackageListView::refreshView()
{
	abortLoadingPackageDetails();

	//if( m_parallelScanning == true )
	//{
		//TODO: bring back?
		// also start a thread for scanning the installed packages,
		// because they are more important and need hasUpdates first.
		//packageInstalledScanner->startScanningCategory(
		//	this, tree, categoryName, subcategoryName
		//);
	//}

	// reset everything
	m_categories.clear();
	this->clear(); emit cleared();
	m_loadedPackageCount = 0;
	m_installedPackageCount = 0;
	m_totalPackageCount = 0;

	// Get the list of shown packages
	m_packageSelector->setSourceList( m_allPackages );
	m_packageSelector->setDestinationList( m_shownPackages );
	if( m_packageSelector->perform() == IJob::Failure ) {
		kdDebug() << i18n( "PackageListView debug output",
			"PackageListView::refreshView(): "
			"Failed to select shown packages" )
			<< endl;
		return;
	}

	// scan the package descriptions (in an extra thread)
	m_multiplePackageLoader->setPackageList( m_shownPackages );
	m_multiplePackageLoader->start();


	// Insert packages under their right category in the ListView.
	// If the category doesn't exist yet, then a check ensures that
	// it is created before insertPackageItem() creates the package item.

	QListViewItem *catItem;
	QString categoryName, uniqueCategoryName;
	PackageList::iterator packageIteratorEnd = m_shownPackages->end();

	for( PackageList::iterator packageIterator = m_shownPackages->begin();
	     packageIterator != packageIteratorEnd; ++packageIterator )
	{
		//TODO: We want user visible names in categoryName.
		//      See emitSelectionChanged().
		categoryName = (*packageIterator)->category()->uniqueName();
		uniqueCategoryName = (*packageIterator)->category()->uniqueName();

		if( m_categories.contains(uniqueCategoryName) == false )
		{
			catItem = new KListViewItem( this, categoryName );
			catItem->setExpandable( true );
			catItem->setOpen( true );
			catItem->setPixmap( 0, pxCategoryItem );
			m_categories[uniqueCategoryName].item = catItem;
		}
		insertPackageItem(
			m_categories[uniqueCategoryName].item, *(*packageIterator)
		);
	}

	emit contentsChanged();

} // end of refreshView(...)

/**
 * Insert a package item into the view.
 * This function does not insert version child items.
 *
 * @param parent  Parent item of the package one. This will most likely be
 *                a category item.
 * @param package  The package whose data is used for creating the item.
 */
void PackageListView::insertPackageItem( QListViewItem* parent,
                                         Package& package )
{
	// create the package item
	QListViewItem* packageItem = new KListViewItem( parent, package.name() );
	packageItem->setExpandable( true );

	PackageViewPackage& pkg =
		m_categories[parent->text(0)].packageItems[package.name()];
	pkg.item = packageItem;
	pkg.containsVersions = false;
	pkg.hasDetails = false;

	if( package.containsInstalledVersion() ) {
		packageItem->setPixmap( 0, pxPackageItemInstalled );
		pkg.installed = true;
		m_installedPackageCount++;
	}
	else {
		packageItem->setPixmap( 0, pxPackageItem );
	}
	m_totalPackageCount++;
}

/**
 * Insert package version items into the view (being children of a package
 * item).
 *
 * @param packageItem  The parent item whose version items should be created.
 */
void PackageListView::insertVersionItems( QListViewItem* packageItem )
{
	if( packageItem->depth() != 1 ) // not a package item
		return;

	QListViewItem* categoryItem = packageItem->parent();
	PackageCategory* category = m_backend->createPackageCategory();
	category->loadFromUniqueName( categoryItem->text(0) );

	Package* package = m_shownPackages->package(
		category,
		packageItem->text(0) // package name
	);

	// get description, maskedness and Co.
	m_packageLoader->setPackage( package );
	m_packageLoader->perform();
	this->displayPackageDetails( package );

	PackageViewCategory& pvcategory =
		m_categories[package->category()->uniqueName()];
	PackageViewPackage& pkg =
		pvcategory.packageItems[package->name()];

	if( pkg.containsVersions == true )
		return;
	else
		pkg.containsVersions = true;


	QListViewItem* versionItem;

	// create version subnodes
	Package::versioniterator versionIteratorEnd = package->versionEnd();

	for( Package::versioniterator versionIterator = package->versionBegin();
	     versionIterator != versionIteratorEnd; ++versionIterator )
	{
		versionItem = new KListViewItem(
			packageItem, (*versionIterator)->version() );

		if( (*versionIterator)->isInstalled() ) {
			versionItem->setPixmap( 0, pxVersionItemInstalled );
		}
		else if( (*versionIterator)->isAvailable() == false ){
			versionItem->setPixmap( 0, pxVersionItemNotAvailable );
		}
		else {
			versionItem->setPixmap( 0, pxVersionItem );
		}
	}
}

/**
 * Called when ebuild info of the package has been loaded.
 * Description texts and other delayed-loading stuff are handled in here.
 *
 * @param package  The package for which detailed info should be displayed.
 */
void PackageListView::displayPackageDetails( Package* package )
{
	if( package == NULL || !package->containsVersions() )
		return;

	PackageViewPackage& pkg =
		m_categories[ package->category()->uniqueName() ]
			.packageItems[ package->name() ];

	if( pkg.hasDetails == true || pkg.item == NULL )
		return;
	else
		pkg.hasDetails = true;

	pkg.item->setText( 1, package->shortDescription() );

	if( package->canUpdate() ) {
		pkg.item->setPixmap( 0, pxPackageItemUpdatable );
		emit foundUpgradablePackage(package);
	}

	m_loadedPackageCount++;
	//emit contentsChanged(); // NOT. try it out, if you want.
}

/**
 * Get the name of the current category filter of the list view.
 * Returns QString::null if all packages from the portage tree are shown.
 */
/*void PackageListView::updateSettings( PortageSettings* settings )
{
	if( settings == NULL ) {
		kdDebug() << "PackageListView: Got a NULL settings object." << endl;
		arch = "x86";
	}
	else {
		arch = settings->acceptedKeyword();
	}
}*/

/**
 * Receiver for package loading events (and others, if needed).
 */
/*void PackageListView::customEvent( QCustomEvent* event )
{
	if( event->type() == (int) LoadingPackageComplete )
	{
		LoadingPackageCompleteEvent* packageEvent =
			(LoadingPackageCompleteEvent*) event;

		if( packageEvent->action == PackageScanner::ScanPackage )
		{
			if( packageEvent->error != PortageLoaderBase::NoError )
				return;

			displayPackageDetails( packageEvent->package );

			if( packageEvent->package == currentPackage ) {
				emit selectionChanged( currentPackage );
				currentPackage = NULL;
			}

			if( packageEvent->packageScanner == packageCategoryScanner
			    || packageEvent->packageScanner == packageInstalledScanner )
			{
				emit loadingPackageInfo( loadedPackages, totalPackages );
			}
		}
		else if( packageEvent->action == PackageScanner::ScanCategory )
		{
			emit finishedLoadingPackageDetails( *m_shownPackages );

			if( packageEvent->packageScanner == packageInstalledScanner ) {
				emit finishedLoadingInstalledPackageInfo();
			}
		}
	}
}*/

} // namespace
