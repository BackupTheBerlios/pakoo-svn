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

#ifndef LIBPAKTPACKAGELIST_H
#define LIBPAKTPACKAGELIST_H

#include <qstring.h>
#include <qmap.h>
#include <qvaluelist.h>

#include <ksharedptr.h>

#include "package.h"


namespace libpakt {

class PackageCategory;

/**
 * PackageList is a class for managing Package objects.
 * It can be used as a representation of the package tree,
 * or to store package search results, or whatever.
 *
 * @short  A list of Package objects.
 */
class PackageList
{
public:
	typedef QMapIterator<QString,KSharedPtr<Package> > iterator;
	typedef QMapConstIterator<QString,KSharedPtr<Package> > const_iterator;

	PackageList();

	int count();
	void clear();

	bool insert( Package* package );
	bool insert( PackageCategory* category, const QString& name );

	bool contains( PackageCategory* category, const QString& name );
	Package* package( PackageCategory* category, const QString& name );

	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;

private:
	typedef QMap<QString,KSharedPtr<Package> > PackageMap;

	//! The internal list of packages in the tree.
	PackageMap packages;
};

}

#endif // LIBPAKTPACKAGELIST_H
