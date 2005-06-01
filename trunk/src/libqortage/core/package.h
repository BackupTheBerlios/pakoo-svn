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

#ifndef PACKAGE_H
#define PACKAGE_H

#include <qstring.h>
#include <qmap.h>
#include <qvaluelist.h>

class PackageVersion;

typedef QMap<QString,PackageVersion> PackageVersionMap;


/**
 * Package is a class to store information about a package.
 * It contains the package's name and category
 * and a set of PackageVersion objects.
 *
 * @short Representation of a package in the portage tree (containing PackageVersion objects).
 */
class Package
{
public:
	QString category, subcategory, name;

	Package( QString category = "", QString subcategory = "", QString name = "" );

	// modifying functions
	void clear();
	void removeVersion( QString version );
	bool setVersion( PackageVersion& version );
	bool setVersion( QString versionString );
	PackageVersion* Package::version( QString version );

	// info functions
	bool hasVersions();
	bool hasVersion( QString version );
	bool hasInstalledVersion();
	bool hasUpdate( const QString& arch );
	bool hasUpdate( PackageVersion* version, const QString& arch );

	PackageVersionMap* versionMap();
	QValueList<PackageVersion> sortedVersionList();

protected:
	//! The internal list of package versions.
	PackageVersionMap versions;
};

#endif // PACKAGE_H
