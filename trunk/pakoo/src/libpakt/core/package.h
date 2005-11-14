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

#ifndef LIBPAKTPACKAGE_H
#define LIBPAKTPACKAGE_H

#include <qstring.h>
#include <qmap.h>
#include <qvaluelist.h>
#include <qstringlist.h>

#include <ksharedptr.h>

//FIXME: Shouldn't have portagedependecy here!
#include "../portage/core/portagecategory.h"


namespace libpakt {

class PackageVersion;

/**
 * Package is a class to store information about a package.
 * It contains the package's name and category
 * and a set of PackageVersion objects.
 *
 * @short Representation of a package in the portage tree (containing PackageVersion objects).
 */
class Package : public KShared
{
public:
	typedef QMapIterator<QString,PackageVersion*> versioniterator;
	typedef QMapConstIterator<QString,PackageVersion*> const_versioniterator;

	Package( PackageCategory* category, const QString& name );
	~Package();

	// core functionality
	const QString& name() const;
	PackageCategory* category();

	// version clearing functions
	void clear();
	void removeVersion( const QString& version );

	// info functions
	bool containsVersions();
	bool containsVersion( const QString& version );
	bool containsInstalledVersion();
	bool containsAvailableVersion();

	virtual bool canUpdate();
	virtual bool canUpdate( PackageVersion* version );

	// version retrieval functions
	PackageVersion* version( const QString& version );
	PackageVersion* insertVersion( const QString& versionString );
	PackageVersion* latestVersion();
	PackageVersion* latestVersionAvailable();
	QValueList<PackageVersion*> sortedVersionList();

	versioniterator versionBegin();
	versioniterator versionEnd();
	const_versioniterator versionBegin() const;
	const_versioniterator versionEnd() const;

	// other retrieval functions
	QString uniqueName() const;

	/**
	 * A rather detailed description of the package.
	 * If there is no distinction between detailed and short descriptions,
	 * this will be the same as shortDescription().
	 */
	virtual QString description() = 0;

	/**
	 * A short description of the package, perferably just one short sentence.
	 * If there is no distinction between detailed and short descriptions,
	 * this will be the same as description().
	 */
	virtual QString shortDescription() = 0;

protected:
	/**
	 * Required to allocate new PackageVersion objects.
	 * You can either derive a class to overload this function or use
	 * TemplatedPackage to define the PackageVersion type without subclassing.
	 */
	virtual PackageVersion* createPackageVersion( const QString& versionString ) = 0;

	typedef QMap<QString,PackageVersion*> PackageVersionMap;

	//! The name of the package, e.g. "pakoo"
	const QString m_name;
	//! The package category, for example, "app-portage" in Gentoo
	PackageCategory* m_category;
	//! The internal list of package versions.
	PackageVersionMap m_versions;
};

}

#endif // LIBPAKTPACKAGE_H
