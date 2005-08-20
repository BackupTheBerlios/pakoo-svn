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

#ifndef PORTAGETREESCANNER_H
#define PORTAGETREESCANNER_H

#include "portageloaderbase.h"
#include "../core/portagetree.h"

#include <qstringlist.h>
#include <qdir.h>
#include <qregexp.h>

class PackageVersion;
class Package;
class PortageTree;
class PackageScanner;

/**
 * PortageTreeScanner is an optionally threaded class for scanning the portage
 * tree for packages. In opposition to PackageScanner, this class only
 * scans the folder structure, so it will find packages without versions
 * in the mainline and overlay trees, and packages containing versions
 * in the installed packages database.
 *
 * @short  A threaded class for scanning the portage tree for packages.
 */
class PortageTreeScanner : public PortageLoaderBase
{
public:
	PortageTreeScanner( QString treeDir = "/usr/portage/",
	                    QStringList overlayDirs = QStringList(),
	                    QString installedDir = "/var/db/pkg/",
	                    QString edbDepDir = "/var/cache/edb/dep/" );

	~PortageTreeScanner();

	PortageLoaderBase::Error scanTrees(
		PortageTree* portageTree,
		PortageTree::Trees searchedTrees = PortageTree::All,
		bool preferEdb = true
	);

	bool startScanningTrees( QObject* receiver, PortageTree* portageTree,
	                         PortageTree::Trees searchedTrees = PortageTree::All,
	                         bool preferEdb = true );

	void setMainlineTreeDirectory( QString directory );
	void setOverlayTreeDirectories( QStringList directories );
	void setInstalledPackagesDirectory( QString directory );
	void setEdbDepDirectory( QString directory );

	PackageScanner* packageScanner();

protected:
	void run();

	PortageLoaderBase::Error scanMainlineTree();
	PortageLoaderBase::Error scanOverlayTrees();
	PortageLoaderBase::Error scanInstalledTree();

	PortageLoaderBase::Error scanTree(
		QString treeDir, PortageTree::Trees searchedTree
	);
	void scanTreePackage( QDir& d, bool overlay );
	void scanEdbCategory( QDir& d );
	void scanInstalledPackage( QDir& d );
	void postPartlyCompleteEvent( QDateTime& since, PortageTree::Trees searchedTrees );

	//! The PortageTree object that will be filled.
	PortageTree* tree;
	//! A PackageScanner object with settings synchronized to this PortageTreeScanner.
	PackageScanner* pkgScanner;

	//! The directory where PortageTreeScanner tries to find packages.
	QString portageTreeDir;
	//! The list of portage overlay directories.
	QStringList portageOverlayDirs;
	//! The directory where the database of installed packages resides.
	QString installedPackagesDir;
	//! The directory where the portage cache resides.
	QString edbDir;

	//! Set true if the edb/dep/ directory should be searched instead of the mainline tree.
	bool preferEdb;

	//! Defines which package directories are searched.
	PortageTree::Trees searchedTrees;

	/**
	 * A counter, incremented with each found package,
	 * but counting only the packages from the currently searched tree.
	 */
	int localTreeCount;
	//! A counter, incremented with each found installed package.
	int installedPackageCount;

private:
	//! The name of the current category
	QString currentCategory;
	//! The name of the current subcategory
	QString currentSubcategory;

	//! An object used for temporarily storing package information.
	Package* currentPackage;
	//! An object used for temporarily storing package version information.
	PackageVersion* currentVersion;

	//! Regexp for ebuild names (the part before the version string)
	QRegExp rxVersion;
};

#endif // PORTAGETREESCANNER_H
