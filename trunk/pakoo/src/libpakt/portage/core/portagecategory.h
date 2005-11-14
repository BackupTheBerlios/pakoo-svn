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

#ifndef LIBPAKTPORTAGECATEGORY_H
#define LIBPAKTPORTAGECATEGORY_H

#include "../../core/packagecategory.h"


namespace libpakt {

/**
 * This is a QStringList with a few convenience functions regarding
 * package categories. A category is viewed as part of the package tree,
 * and follows a hierarchical model. An empty list means all packages
 * (or: no category), a list containing one element (e.g. "app") is
 * a subset of the "all packages" category, and a list containing more
 * elements (e.g. "app"->"portage") is an even smaller subset and
 * shows the tree structure of the package tree quite well.
 */
class PortageCategory : public PackageCategory
{
public:
	enum PortageCategoryIndex {
		MainCategory = 0,
		SubCategory = 1
	};

	PortageCategory() : PackageCategory() {};
	PortageCategory( const QStringList& list ) : PackageCategory(list) {};
	PortageCategory( const QString& mainCategory, const QString& subCategory );

	void setCategory( const QString& mainCategory, const QString& subCategory );

	QString userVisibleName() const;
	QString uniqueName() const;
	bool loadFromUniqueName( const QString& uniqueName );
};

}

#endif // LIBPAKTPORTAGECATEGORY_H
