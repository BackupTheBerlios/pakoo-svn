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

#include "../libqortage/libqortage.h"


/**
 * A KListView with additional functions to handle portage tree packages.
 *
 * @short Widget to display a list of packages and versions.
 */
class PakooPackageListView : public KListView
{
	Q_OBJECT
public:
	PakooPackageListView( QWidget* parent, const char* name, PackageScanner* packageScanner );
	~PakooPackageListView();
	void quit();

	const QString& currentCategory();
	const QString& currentSubcategory();
	int installedPackageCount();
	int totalPackageCount();

	bool hasInstalledVersion( const QListViewItem* item );

signals:
	//! Emitted if the list view is cleared, so it doesn't contain any items anymore.
	void cleared();
	//! Emitted if a package item is selected.
	void selectionChanged( Package* package );
	//! Emitted if a package version item is selected.
	void selectionChanged( Package* package, PackageVersion* version );
	//! Emitted if a category item is selected.
	void selectionChanged( const QString& category, const QString& subcategory );
	//! Emitted if the list of packages has changed.
	void contentsChanged();
	//! Emitted if the details (description, hasUpdates) of a package have been loaded.
	void loadingPackageInfo( int loadedPackageCount, int totalPackageCount );
	//! Emitted if all package details of the displayed packages have been loaded.
	void finishedLoadingPackageInfo( int totalPackageCount );
	/**
	 * Emitted if all installed packages of the current category have been loaded.
	 * This also means that the list view knows if there are updates for the
	 * installed packages or not.
	 */
	void finishedLoadingInstalledPackageInfo();
	/**
	 * Emitted if an upgradable package has been found.
	 * This happens while scanning the packages for detailed info
	 * (exactly when the description text is displayed, too).
	 */
	void foundUpgradablePackage( Package* package );

public slots:
	void displayPackages( PortageTree* tree,
		const QString& categoryName = QString::null,
		const QString& subcategoryName = QString::null
	);
	void emitSelectionChanged( QListViewItem* item );

protected:

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

	//! A map of category structs, containing additional info about the item.
	QMap<QString,PackageViewCategory> categories;
	//! A pointer to the portage tree where the displayed packages come from.
	PortageTree* portageTree;
	//! A PackageScanner object that retrieves missing package information for the whole package list.
	PackageScanner* packageCategoryScanner;
	//! A PackageScanner object that retrieves missing package information for all installed packages.
	PackageScanner* packageInstalledScanner;
	//! A PackageScanner object that retrieves missing package information for one single package.
	PackageScanner* packageScanner;

	QPixmap pxCategoryItem,
	        pxPackageItem, pxPackageItemInstalled, pxPackageItemUpdatable,
	        pxVersionItem, pxVersionItemInstalled, pxVersionItemNotAvailable;

protected slots:
	void insertVersionItems( QListViewItem* packageItem );
	void displayPackageDetails( Package* package );

private:
	void insertPackageItem( QListViewItem* parent, Package& package );

	void customEvent( QCustomEvent* event );

	//! The main category that's currently displayed.
	QString category;
	//! The subcategory that's currently displayed.
	QString subcategory;
	//! The currently selected package.
	Package* currentPackage;

	//! A counter used for detailed package info progress.
	int loadedPackages;
	//! The number of installed packages in the list view.
	int installedPackages;
	//! The overall number of packages in the list view.
	int totalPackages;

	//! Enables or disables parallel scanning (installed and not-installed at the same time)
	bool parallelScanning;
};

#endif // PACKAGELISTVIEW_H
