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

#ifndef LIBPAKTPORTAGEPACKAGELOADER_H
#define LIBPAKTPORTAGEPACKAGELOADER_H

#include "packageloader.h"

#include <qstringlist.h>
#include <qregexp.h>


namespace libpakt {

class PortageSettings;

/**
 * PortagePackageLoader is a threaded job which is able to retrieve package
 * detail information. In this implementation for Portage, this class
 * loads file contents (for example, of ebuilds or digests) to fill in
 * PackageVersion info that has not been read yet.
 *
 * After setting up the scanner (using at least the setPackage()
 * member function) you can call start() or perform() to begin scanning.
 *
 * @short A threaded class for retrieving detail info of a single package.
 */
class PortagePackageLoader : public PackageLoader
{
	Q_OBJECT

public:
	PortagePackageLoader();

	// settings
	void setSettingsObject( PortageSettings* settings );

protected:
	JobResult performThread();

private:

	bool scanPackage();

	bool scanEbuild( PackageVersion* version, const QString& filename );
	bool scanEdbFile( PackageVersion* version, const QString& filename );
	bool scanOverlayPackage( PackageVersion* version );
	bool scanDigest( PackageVersion* version, const QString& filename );

	bool extractStringList( const QString& string, QRegExp* rx, QStringList* targetList );

	//! The PortageSettings object used for retrieving directories and cache info.
	PortageSettings* settings;

	//! An object used for temporarily storing package version information.
	PackageVersion* version;

	//! The directory where PortagePackageLoader tries to find packages.
	QString mainlineTreeDir;
	//! The overlay directories for finding additional packages.
	QStringList overlayTreeDirs;
	//! The directory where the database of installed packages resides.
	QString installedPackagesDir;
	//! The directory where the portage cache resides.
	QString cacheDir;

	//! Set true if the Portage cache should be scanned instead of the mainline tree.
	bool preferCache;

	// Regexps for various line strings inside an ebuild.
	QRegExp rxDescription, rxHomepage, rxSlot;
	QRegExp rxLicenses, rxKeywords, rxUseflags;
};

}

#endif // LIBPAKTPORTAGEPACKAGELOADER_H
