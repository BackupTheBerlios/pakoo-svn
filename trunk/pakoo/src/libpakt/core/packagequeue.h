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

#ifndef LIBPAKTPACKAGEQUEUE_H
#define LIBPAKTPACKAGEQUEUE_H

#include <qvaluelist.h>

#include <ksharedptr.h>


namespace libpakt {

class Package;
class PackageVersion;

enum QueuedItemType {
	PackageType,
	PackageVersionType,
	WorldClassType,
	SystemClassType
};
typedef struct QueuedItem {
	QueuedItemType type;
	KSharedPtr<KShared> data;

	QueuedItem()
		: type( WorldClassType )
	{} // KSharedPtr initializes with an empty pointer

	QueuedItem( QueuedItemType _type, KSharedPtr<KShared> _data )
		: type(_type), data(_data) {};
} QueuedItem;

/**
 * @short An ordered list of packages with or without specified version.
 *
 * Like PackageList, PackageQueue can manage packages.
 * Unlike PackageList it is designed to act as an install/uninstall queue,
 * so it is possible to store package versions instead of just general
 * packages. Also, the list is ordered so the packages can be retrieved
 * in the right order.
 */
class PackageQueue : public QValueList<QueuedItem>
{
public:
	PackageQueue();
	PackageQueue( const PackageQueue& otherList );

	PackageQueue::iterator appendWorldClass();
	PackageQueue::iterator appendSystemClass();
	PackageQueue::iterator appendPackage( Package* package );
	PackageQueue::iterator appendPackageVersion( PackageVersion* version );
};

}

#endif // LIBPAKTPACKAGEQUEUE_H
