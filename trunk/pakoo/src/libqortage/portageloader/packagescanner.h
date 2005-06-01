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

#ifndef PACKAGESCANNER_H
#define PACKAGESCANNER_H

#include "portageloaderbase.h"

#include <qdir.h>
#include <qregexp.h>

class Package;
class PackageVersion;
class PortageTree;


/**
 * PackageScanner is an optionally threaded class able to retrieve
 * package detail information. In opposition to PortageTreeScanner, this class
 * is responsible for scanning files (namely: ebuilds and digests) to
 * fill versions of a package with detailed info.
 *
 * @short A threaded class for retrieving package detail info.
 */
class PackageScanner : public PortageLoaderBase
{
public:
	enum Action {
		ScanPackage,
		ScanCategory
	};

	PackageScanner( QString treeDir = "/usr/portage/",
	                QString overlayDir = "",
	                QString installedDir = "/var/db/pkg/",
	                QString edbDepDir = "/var/cache/edb/dep/" );
	PackageScanner( PackageScanner* anotherScanner );

	PortageLoaderBase::Error scanPackage( Package* package,
	                                           bool preferEdb = true );
	PortageLoaderBase::Error scanCategory(
		PortageTree* tree, QString category,
		QString subcategory, bool preferEdb = true
	);

	bool startScanningPackage( QObject* receiver,
	                           Package* package, bool preferEdb = true );
	bool startScanningCategory( QObject* receiver, PortageTree* tree,
	                            QString category, QString subcategory,
	                            bool preferEdb = true );

	void setFilterInstalled( bool doFilter, bool scanInstalled = true );

	void setPortageTreeDirectory( QString directory );
	void setOverlayDirectory( QString directory );
	void setInstalledPackagesDirectory( QString directory );
	void setEdbDepDirectory( QString directory );

protected:
	void run();

	bool scanEbuild( PackageVersion* version, const QString& filename );
	bool scanEdbFile( PackageVersion* version, const QString& filename );
	bool scanDigest( PackageVersion* version, const QString& filename );

	//! The currently processed package.
	Package* package;

	//! The directory where PackageScanner tries to find packages.
	QString  portageTreeDir;
	//! The portage overlay directory for finding additional packages.
	QString  portageOverlayDir;
	//! The directory where the database of installed packages resides.
	QString  installedPackagesDir;
	//! The directory where the portage cache resides.
	QString edbDir;

	//! Set true if the edb/dep/ directory should be searched instead of the mainline tree.
	bool preferEdb;

	//! An object used for temporarily storing package version information.
	PackageVersion* version;
	//! The portage tree which is currently scanned by category.
	PortageTree* tree;
	//! The currently processed category.
	QString category;
	//! The currently processed subcategory.
	QString subcategory;

	//! Specifies if a single package or a whole category is searched.
	Action action;
	//! true if filtering installed packages out or in is enabled.
	bool doFilterInstalled;
	/**
	 * true if only packages with installed versions should be scanned,
	 * false to scan only packages without installed versions.
	 * This is only used if doFilter == true.
	 */
	bool scanInstalled;

private:
	//! Helper function for extracting tokens from a string
	bool extractStringList( const QString& string, QRegExp* rx, QStringList* targetList );

	// Regexps for various line strings inside an ebuild.
	QRegExp rxDescription, rxHomepage, rxSlot;
	QRegExp rxLicenses, rxKeywords, rxUseflags;
};

#endif // PACKAGESCANNER_H
