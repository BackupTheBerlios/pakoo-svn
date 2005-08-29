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

#ifndef PACKAGELISTVIEW_H
#define PACKAGELISTVIEW_H

#include <klistview.h>

#include <qmap.h>
#include <qstring.h>
#include <qpixmap.h>


namespace libpakt {

class BackendFactory;
class PackageList;
class Package;
class PackageVersion;
class PackageCategory;
class PackageSelector;
class PackageLoader;
class MultiplePackageLoader;

/**
 * A KListView with additional functions to handle portage tree packages.
 *
 * @short Widget to display a list of packages and versions.
 */
class PackageListView : public KListView
{
	Q_OBJECT
public:
	PackageListView( QWidget* parent, const char* name,
	                 BackendFactory* backend );
	~PackageListView();

	PackageSelector* packageSelector();

	int installedPackageCount();
	int totalPackageCount();

	bool hasInstalledVersion( const QListViewItem* item );

public slots:
	void setPackageList( PackageList& package );
	void setPackageSelector( PackageSelector& selector );
	void refreshView();

	void emitSelectionChanged( QListViewItem* item );

	void abortLoadingPackageDetails();


signals:
	/** Emitted if the list view is cleared,
	 * so it doesn't contain any items anymore. */
	void cleared();
	/** Emitted if a package item is selected. */
	void selectionChanged( Package* package );
	/** Emitted if a package version item is selected. */
	void selectionChanged( Package* package, PackageVersion* version );
	/** Emitted if a category item is selected. */
	void selectionChanged( PackageCategory* category );
	/** Emitted if a new package selector is set. */
	void packageSelectorChanged( PackageSelector* selector );
	/** Emitted if the list of packages has changed. */
	void contentsChanged();
	/** Emitted if the details (description, hasUpdates)
	 * of a package have been loaded. */
	void loadingPackageInfo( int loadedPackageCount, int totalPackageCount );
	/** Emitted if all package details of the displayed packages
	 * have been loaded. */
	void finishedLoadingPackageDetails( PackageList& packages );
	/**
	 * Emitted if all installed packages of the current category
	 * have been loaded. This also means that the list view knows
	 * if there are updates for the installed packages or not.
	 */
	void finishedLoadingInstalledPackageDetails();
	/**
	 * Emitted if an upgradable package has been found.
	 * This happens while scanning the packages for detailed info
	 * (exactly when the description text is displayed, too).
	 */
	void foundUpgradablePackage( Package* package );


private slots:
	void insertVersionItems( QListViewItem* packageItem );
	void displayPackageDetails( Package* package );

private:

	struct PackageViewPackage {
		QListViewItem* item;
		bool installed; // true if the package has at least one installed version
		bool hasVersions; // true if its version child items have already been added
		bool hasDetails;  // true if the package details have already been loaded
	};

	struct PackageViewCategory {
		QListViewItem* item;
		QMap<QString,PackageViewPackage> packageItems;
	};

	void insertPackageItem( QListViewItem* parent, Package& package );

	/**
	 * The backend factory that creates some objects that are used here.
	 */
	BackendFactory* m_backend;

	/** TODO: get arch outta here */
	QString m_arch;

	/** A map of category structs, containing
	 * additional info about the item. */
	QMap<QString,PackageViewCategory> m_categories;
	/** The list of all available packages. */
	PackageList* m_allPackages;
	/** The list of all packages that are shown in the ListView. */
	PackageList* m_shownPackages;
	/** The object that filters out the shown packages from the
	 * complete package list. */
	PackageSelector* m_packageSelector;

	/** A MultiplePackageLoader object that retrieves missing
	 * package information for all installed packages. */
	MultiplePackageLoader* m_multiplePackageLoader;
	/** A PackageScanner object that retrieves missing
	 * package information for one single package. */
	PackageLoader* m_packageLoader;

	/** The currently selected package. */
	Package* m_currentPackage;

	/** A counter used for detailed package info progress. */
	int m_loadedPackageCount;
	/** The number of installed packages in the list view. */
	int m_installedPackageCount;
	/** The overall number of packages in the list view. */
	int m_totalPackageCount;

	QPixmap pxCategoryItem,
		pxPackageItem, pxPackageItemInstalled, pxPackageItemUpdatable,
		pxVersionItem, pxVersionItemInstalled, pxVersionItemNotAvailable;
};

}

#endif // PACKAGELISTVIEW_H
