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

#include "packageloader.h"

#include <qapplication.h>


namespace libpakt {

/**
 * Initialize this object. The package which will be loaded is not specified
 * yet, so you still have to call setPackage().
 *
 * @see setPackage
 */
PackageLoader::PackageLoader() : ThreadedJob()
{
	m_package = NULL;
}

/**
 * Specify the package that will be scanned and
 * filled with detailed package info.
 */
void PackageLoader::setPackage( Package* package )
{
	m_package = package;
}

/**
 * Retrieve the currently scanned package.
 */
Package* PackageLoader::package()
{
	return m_package;
}


/**
 * From within the thread, emit a packageLoaded() signal to the main thread.
 */
void PackageLoader::emitPackageLoaded()
{
	PackageLoadedEvent* event  = new PackageLoadedEvent();
	event->package = m_package;
	QApplication::postEvent( this, event );
}

/**
 * Translates QCustomEvents into signals. This function is called from Qt
 * in the main thread, which guarantees safety for emitting signals.
 */
void PackageLoader::customEvent( QCustomEvent* event )
{
	switch( event->type() )
	{
	case (int) PackageLoadedEventType:
		emit packageLoaded( ((PackageLoadedEvent*)event)->package );
		break;

	default:
		ThreadedJob::customEvent( event );
		break;
	}
}

} // namespace
