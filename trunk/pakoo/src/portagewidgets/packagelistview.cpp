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


/**
 * Initialize this object.
 * @param scanner  A copy of this scanner will be kept
 *                 to retrieve detailed package info.
 */
PakooPackageListView::PakooPackageListView( QWidget* parent, const char* name,
                                            PackageScanner* scanner )
: KListView( parent, name )
{
	this->addColumn( i18n("Package") );
	this->addColumn( i18n("Description") );
	this->setRootIsDecorated( true );
	this->setFullWidth( true );

	packageScanner = new PackageScanner( scanner );

	parallelScanning = true; // you may set this to false when debugging
	if( parallelScanning == true )
	{
		packageCategoryScanner = new PackageScanner( scanner );
		packageCategoryScanner->setFilterInstalled( true, false );
		packageInstalledScanner = new PackageScanner( scanner );
		packageInstalledScanner->setFilterInstalled( true, true );
	}
	else
	{
		packageCategoryScanner = new PackageScanner( scanner );
		packageInstalledScanner = new PackageScanner( scanner );
	}

	portageTree = NULL;

	loadedPackages = 0;
	installedPackages = 0;
	totalPackages = 0;

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
 * Get the name of the current category filter of the list view.
 * Returns QString::null if all packages from the portage tree are shown.
 */
const QString& PakooPackageListView::currentCategory()
{
	return category;
}

/**
 * Get the name of the current subcategory filter of the list view.
 * Returns QString::null if all packages from the main category are shown.
 */
const QString& PakooPackageListView::currentSubcategory()
{
	return subcategory;
}

/**
 * Get the number of installed packages which are currently shown
 * in the list view.
 */
int PakooPackageListView::installedPackageCount()
{
	return installedPackages;
}

/**
 * Get the total number of packages which are currently shown
 * in the list view.
 */
int PakooPackageListView::totalPackageCount()
{
	return totalPackages;
}

/**
 * Determine if a package (item) in this list view has at least one
 * installed version. Returns false for version and category items.
 */
bool PakooPackageListView::hasInstalledVersion( const QListViewItem* packageItem )
{
	if( packageItem->depth() != 1 ) { // we got a version or category item
		return false;
	}
	else // we got a package item, depth 1
	{
		const QString& categoryString = packageItem->parent()->text(0);
		const QString& packageString = packageItem->text(0);

		if( categories.contains(categoryString) == false )
			return false;

		PackageViewCategory& cat = categories[categoryString];
		if( cat.packageItems.contains(packageString) == false )
			return false;
		else
			return cat.packageItems[packageString].installed;
	}
}

/**
 * The deconstructor aborts and waits for the PackageScanner,
 * if it's running, so that it can safely be deleted.
 */
PakooPackageListView::~PakooPackageListView()
{
	this->quit();
}

/**
 * Prepare for deconstructing. Involves stopping threads and stuff.
 */
void PakooPackageListView::quit()
{
	if( packageCategoryScanner != NULL )
	{
		if( packageCategoryScanner->running() ) {
			packageCategoryScanner->abort();
			packageCategoryScanner->wait();
		}
		delete packageCategoryScanner;
		packageCategoryScanner = NULL;
	}
	if( packageInstalledScanner != NULL )
	{
		if( packageInstalledScanner->running() ) {
			packageInstalledScanner->abort();
			packageInstalledScanner->wait();
		}
		delete packageInstalledScanner;
		packageInstalledScanner = NULL;
	}
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
 * Translate item selections into package[,version] selection signal.
 * @param item  The item that has been selected.
 */
void PakooPackageListView::emitSelectionChanged( QListViewItem* item )
{
	if( portageTree == NULL )
		return;

	QListViewItem* categoryItem;
	QString categoryName, subcategoryName;
	int pos;

	if( item->depth() == 0 ) { // we got a category item
		categoryItem = item;
	}
	else if( item->depth() == 1 ) { // we got a package item
		categoryItem = item->parent();
	}
	else { // nothing of the previous ones, so it's a version item
		categoryItem = item->parent()->parent();
	}

	pos = categoryItem->text(0).find('-');
	categoryName = categoryItem->text(0).left(pos);
	subcategoryName = categoryItem->text(0).mid(pos+1);

	if( item->depth() == 0 )  // we got a category item
	{
		emit selectionChanged( categoryName, subcategoryName );
	}
	else if( item->depth() == 1 ) // we got a package item
	{
		QString packageName = item->text(0);
		Package* package = portageTree->package(
			categoryName, subcategoryName, packageName );

		emit selectionChanged( package );

		// retrieve the package's detail info (description and hasUpdates)
		if( packageScanner->running() ) {
			packageScanner->abort();
			packageScanner->wait();
		}
		packageScanner->startScanningPackage( this, package );

		return;
	}
	else // nothing of the previous ones, so it's a version item
	{
		const QString& packageName = item->parent()->text(0);
		Package* package = portageTree->package(
			categoryName, subcategoryName, packageName );
		const QString& versionString = item->text(0);
		if( package->hasVersion(versionString) )
		{
			PackageVersion* version = package->version( versionString );
			emit selectionChanged( package, version );
		}
		else {
			emit selectionChanged( package );
		}
		return;
	}
}

/**
 * Show packages from the given PortageTree object inside this ListView.
 * The packages can be filtered by category and subcategory, so that only
 * a subset of the packages in the tree are displayed.
 *
 * @param tree  The tree object containing the packages that will be shown.
 * @param categoryName  Only show packages from this category. QString::null
 *                      means all packages will be shown (without filter).
 * @param subcategoryName  Only show packages from this subcategory.
 *                         This is only used if the categoryName filter is
 *                         also set. QString::null means that the packages
 *                         won't be filtered by subcategory.
 */
void PakooPackageListView::displayPackages( PortageTree* tree,
	const QString& categoryName, const QString& subcategoryName )
{
	if( tree == NULL )
		return;
	else
		portageTree = tree;

	if( packageCategoryScanner->running() ) {
		packageCategoryScanner->abort();
		packageCategoryScanner->wait();
	}
	if( packageInstalledScanner->running() ) {
		packageInstalledScanner->abort();
		packageInstalledScanner->wait();
	}

	// scan the package descriptions (in an extra thread)
	packageCategoryScanner->startScanningCategory(
		this, tree, categoryName, subcategoryName
	);

	if( parallelScanning == true )
	{
		// also start a thread for scanning the installed packages,
		// because they are more important and need hasUpdates first.
		packageInstalledScanner->startScanningCategory(
			this, tree, categoryName, subcategoryName
		);
	}

	// reset everything
	categories.clear();
	this->clear(); emit cleared();
	loadedPackages = 0;
	installedPackages = 0;
	totalPackages = 0;

	category = categoryName;
	subcategory = subcategoryName;
	QString fullCategoryName;
	QListViewItem *catItem;

	// Insert packages under their right category in the ListView.
	PackageMap* packages = tree->packageMap();
	PackageMap::iterator packageIteratorEnd = packages->end();

	if( categoryName.isNull() ) // no category filter at all, process all
	{
		// Iterate through all packages
		for( PackageMap::iterator packageIterator = packages->begin();
		     packageIterator != packageIteratorEnd; ++packageIterator )
		{
			fullCategoryName = (*packageIterator).category + "-"
			                    + (*packageIterator).subcategory;

			if( categories.contains(fullCategoryName) == false )
			{
				catItem = new KListViewItem( this, fullCategoryName );
				catItem->setExpandable( true );
				catItem->setOpen( true );
				catItem->setPixmap( 0, pxCategoryItem );
				categories[fullCategoryName].item = catItem;
			}
			insertPackageItem(
				categories[fullCategoryName].item, *packageIterator
			);
		}
	}
	else // at least the main-category filter is set
	{
		// Iterate through all packages
		for( PackageMap::iterator packageIterator = packages->begin();
			 packageIterator != packageIteratorEnd; ++packageIterator )
		{
			// Only process categories that equal the given filter category.
			if( categoryName != (*packageIterator).category )
				continue;

			// Only process packages if no subcategory filter is set,
			// or if the current package's subcategory equals the given one.
			if( subcategoryName.isNull()
				|| (*packageIterator).subcategory == subcategoryName )
			{
				fullCategoryName = (*packageIterator).category + "-"
					+ (*packageIterator).subcategory;

				if( categories.contains(fullCategoryName) == false ) {
					catItem = new KListViewItem( this, fullCategoryName );
					catItem->setExpandable( true );
					catItem->setOpen( true );
					catItem->setPixmap( 0, pxCategoryItem );
					categories[fullCategoryName].item = catItem;
				}
				insertPackageItem(
					categories[fullCategoryName].item, *packageIterator
				);
			}
		}
	}
	emit contentsChanged();

} // end of displayPackages(...)

/**
 * Insert a package item into the view.
 * This function does not insert version child items.
 *
 * @param parent  Parent item of the package one. This will most likely be
 *                a category item.
 * @param package  The package whose data is used for creating the item.
 */
void PakooPackageListView::insertPackageItem( QListViewItem* parent,
                                              Package& package       )
{
	// create the package item
	QListViewItem* packageItem = new KListViewItem( parent, package.name );
	packageItem->setExpandable( true );

	PackageViewPackage& pkg =
		categories[parent->text(0)].packageItems[package.name];
	pkg.item = packageItem;
	pkg.hasVersions = false;
	pkg.hasDetails = false;

	if( package.hasInstalledVersion() ) {
		packageItem->setPixmap( 0, pxPackageItemInstalled );
		pkg.installed = true;
		installedPackages++;
	}
	else {
		packageItem->setPixmap( 0, pxPackageItem );
	}
	totalPackages++;
}

/**
 * Insert package version items into the view (being children of a package
 * item).
 *
 * @param packageItem  The parent item whose version items should be created.
 */
void PakooPackageListView::insertVersionItems( QListViewItem* packageItem )
{
	if( packageItem->depth() != 1 ) // not a package item
		return;

	QListViewItem* categoryItem = packageItem->parent();

	const QString& categoryString = categoryItem->text(0);
	int pos = categoryString.find('-');
	Package* package = portageTree->package(
		categoryString.left(pos),  // category
		categoryString.mid(pos+1), // subcategory
		packageItem->text(0)       // package name
	);

	// new PackageScanner that's definetely not running
	PackageScanner* tempScanner = new PackageScanner(packageCategoryScanner);
	tempScanner->scanPackage( package );
	this->displayPackageDetails( package );

	PackageViewPackage& pkg =
		categories[categoryString].packageItems[package->name];

	if( pkg.hasVersions == true )
		return;
	else
		pkg.hasVersions = true;


	QListViewItem* versionItem;

	// create version subnodes
	PackageVersionMap* versions = package->versionMap();
	PackageVersionMap::iterator versionIteratorEnd = versions->end();

	for( PackageVersionMap::iterator versionIterator = versions->begin();
	     versionIterator != versionIteratorEnd; ++versionIterator )
	{
		versionItem = new KListViewItem(
			packageItem, (*versionIterator).version );

		if( (*versionIterator).installed == true ) {
			versionItem->setPixmap( 0, pxVersionItemInstalled );
		}
		//TODO: generalize for all architectures
		else if( (*versionIterator).stability("x86") != PackageVersion::Stable ){
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
void PakooPackageListView::displayPackageDetails( Package* package )
{
	if( package == NULL || !package->hasVersions() )
		return;

	PackageViewPackage& pkg =
		categories[ package->category + "-"
		            + package->subcategory ].packageItems[ package->name ];

	if( pkg.hasDetails == true || pkg.item == NULL )
		return;
	else
		pkg.hasDetails = true;

	PackageVersion& firstVersion = *(package->versionMap()->begin());
	pkg.item->setText( 1, firstVersion.description );

	//TODO: generalize for all architectures
	if( package->hasUpdate("x86") ) {
		pkg.item->setPixmap( 0, pxPackageItemUpdatable );
		emit foundUpgradablePackage(package);
	}

	loadedPackages++;
	emit loadingPackageInfo( loadedPackages, totalPackages );
	if( loadedPackages == totalPackages ) {
		emit finishedLoadingPackageInfo( totalPackages );
	}
	//emit contentsChanged(); // NOT. try it out, if you want.
}

/**
 * Receiver for package loading events (and others, if needed).
 */
void PakooPackageListView::customEvent( QCustomEvent* event )
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
		}
		else if( packageEvent->action == PackageScanner::ScanCategory
		         && packageEvent->packageScanner == packageInstalledScanner )
		{
			emit finishedLoadingInstalledPackageInfo();
		}
	}
}
