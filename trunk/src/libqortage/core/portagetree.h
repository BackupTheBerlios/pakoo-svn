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

#ifndef PORTAGETREE_H
#define PORTAGETREE_H

#include <qstring.h>
#include <qmap.h>
#include <qvaluelist.h>

class Package;

typedef QMap<QString,Package> PackageMap;


/**
 * PortageTree is a class for managing Package objects.
 * Changes in here don't affect the real portage tree, of course.
 *
 * @short Representation of the portage tree (containing Package objects).
 */
class PortageTree
{
public:
	//! Portage trees and reasonable combinations.
	enum Trees {
		All,
		Installed,
		MainlineAndOverlay,
		Mainline,
		Overlay
	};

	PortageTree();

	void clear();

	bool setPackage( Package& package );
	bool setPackage( QString category, QString subcategory, QString package );

	bool hasPackage( QString category, QString subcategory, QString package );
	Package* package( QString category, QString subcategory, QString package );

	PackageMap* packageMap();
	int packageCount();

protected:
	//! The internal list of packages in the tree.
	PackageMap packages;
};

#endif // PORTAGETREE_H
