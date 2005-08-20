/***************************************************************************
 *   Copyright (C) 2004 by karye <karye@users.sourceforge.net>             *
 *   Copyright (C) 2005 by Jakob Petsovits <jpetso@gmx.at>                 *
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

#ifndef LIBPAKTPORTAGETREESCANNER_H
#define LIBPAKTPORTAGETREESCANNER_H

#include "../core/threadedjob.h"
#include "../core/portagecategory.h"

#include <qstringlist.h>
#include <qdir.h>
#include <qregexp.h>


namespace libpakt {

class PackageVersion;
class Package;
class PackageList;
class PortageSettings;

/**
 * PortageTreeScanner is an optionally threaded class for scanning the portage
 * tree for packages. After scanning, the given PackageList object will
 * contain all packages and package versions, with no extra info except
 * if the package is installed or not.
 *
 * After setting up the scanner (using the setPackageList member function)
 * you can call start() or perform() to begin scanning.
 *
 * @short  A threaded class for scanning the portage tree for packages.
 */
class PortageTreeScanner : public ThreadedJob
{
	Q_OBJECT

public:
	PortageTreeScanner();

	// settings
	void setSettingsObject( PortageSettings* settings );
	void setPackageList( PackageList* packages );

	// filters
	void setScanAvailablePackages( bool scanAvailablePackages );
	void setScanInstalledPackages( bool scanInstalledPackages );

signals:
	/**
	 * Emitted every once in a while when packages have been added to the
	 * PortageTree object. The arguments specify the number of available
	 * packages and the number of installed ones. (Note: The number of
	 * available packages is perhaps not the total number of packages in the
	 * resulting tree object, because it's possible that there are packages
	 * that are installed but not available in the tree anymore. For
	 * retrieving the total number of packages, please make use of the
	 * appropriate member function of the tree object, and use this signal
	 * just for displaying the current scan status.)
	 */
	void packagesScanned( int packageCountAvailable,
	                      int packageCountInstalled );

	/**
	 * Emitted if the package tree has successfully been loaded from disk.
	 * The PackageList object now contains all packages and package versions,
	 * but without detailed package information.
	 */
	void finishedLoading( PackageList* packages );

	/**
	 * Emitted for debug output, like starting the scan,
	 * finishing part of the job, or error messages.
	 */
	void debugOutput( QString output );

protected:
	JobResult performThread();
	void customEvent( QCustomEvent* event );

private:
	enum TreeType
	{
		Mainline,
		Overlay,
		Installed
	};
	enum PortageTreeScannerEventType
	{
		PackagesScannedEventType = QEvent::User + 14340,
		FinishedLoadingEventType = QEvent::User + 14341
	};

	bool scanTree( const QString& treeDir, PortageTreeScanner::TreeType searchedTree );
	void scanTreePackage( QDir& d, bool overlay );
	void scanCacheCategory( QDir& d );
	void scanInstalledPackage( QDir& d );

	void emitPackagesScanned();
	void emitFinishedLoading();

	//! The PackageList object that will be filled.
	PackageList* packages;
	//! The PortageSettings object used for retrieving directories and cache info.
	PortageSettings* settings;

	//! The directory where PortageTreeScanner tries to find packages.
	QString mainlineTreeDir;
	//! The list of portage overlay directories.
	QStringList overlayTreeDirs;
	//! The directory where the database of installed packages resides.
	QString installedPackagesDir;
	//! The directory where the portage cache resides.
	QString cacheDir;

	//! Set true if the Portage cache should be scanned instead of the mainline tree.
	bool preferCache;

	//! Defines if mainline and overlay trees are searched.
	bool scanAvailablePackages;
	//! Defines if the installed packages database is searched.
	bool scanInstalledPackages;

	//! A counter, incremented with each found available package.
	int packageCountAvailable;
	//! A counter, incremented with each found installed package.
	int packageCountInstalled;

	//! The name of the current category
	PortageCategory currentCategory;

	//! An object used for temporarily storing package information.
	Package* currentPackage;
	//! An object used for temporarily storing package version information.
	PackageVersion* currentVersion;

	//! Regexp for ebuild names (the part before the version string)
	QRegExp rxVersion;


	//
	// nested event classes
	//

	class PackagesScannedEvent : public QCustomEvent
	{
	public:
		PackagesScannedEvent() : QCustomEvent( PackagesScannedEventType ) {};
		int packageCountAvailable;
		int packageCountInstalled;
	};

	class FinishedLoadingEvent : public QCustomEvent
	{
	public:
		FinishedLoadingEvent() : QCustomEvent( FinishedLoadingEventType ) {};
		PackageList* packages;
	};
};

}

#endif // LIBPAKTPORTAGETREESCANNER_H
