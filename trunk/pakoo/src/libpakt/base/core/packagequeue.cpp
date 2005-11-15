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

#include "packagequeue.h"

#include "package.h"
#include "packageversion.h"


namespace libpakt {

/**
 * Initialize this object with an empty package list.
 */
PackageQueue::PackageQueue()
	: QValueList<QueuedItem>()
{
}

/**
 * Constructs a copy of the otherList.
 * This operation takes O(1) time because QValueList is implicitly shared.
 * The first modification to a list will take O(n) time.
 */
PackageQueue::PackageQueue( const PackageQueue& otherList )
	: QValueList<QueuedItem>( otherList )
{
}

/**
 * Appends the package class "world" to the list, which is a Portage synonym
 * for all packages that have been explicitly installed on your system or
 * are part of the "system" package class.
 */
PackageQueue::iterator PackageQueue::appendWorldClass()
{
	return append( QueuedItem( WorldClassType,
	                           KSharedPtr<KShared>() ) );
}

/**
 * Appends the package class "system" to the list, which is a Portage synonym
 * for predefined packages that make up an essential minimum set of packages
 * needed for running a proper system.
 */
PackageQueue::iterator PackageQueue::appendSystemClass()
{
	return append( QueuedItem( SystemClassType,
	                           KSharedPtr<KShared>() ) );
}

/**
 * Appends a package without version information to the list
 * (so when merging, the latest stable version will be used).
 */
PackageQueue::iterator PackageQueue::appendPackage( Package* package )
{
	return append( QueuedItem( PackageType,
	                           KSharedPtr<KShared>(package) ) );
}

/**
 * Appends a package with version information to the list
 * (so this specific version instead of the latest stable version
 * will be used).
 */
PackageQueue::iterator PackageQueue::appendPackageVersion(
	PackageVersion* version )
{
	return append( QueuedItem( PackageType,
	                           KSharedPtr<KShared>(version) ) );
}

} // namespace
