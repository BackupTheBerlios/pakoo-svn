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

#include "initialloader.h"

#include <qapplication.h>


namespace libpakt {

/**
 * Initialize this object. The package list receiving the loaded packages
 * still has to be set using setPackageList().
 *
 * @see setPackageList
 */
InitialLoader::InitialLoader() : ThreadedJob()
{
	m_packages = NULL;
}

/**
 * Set the PackageList object that will be filled with packages.
 */
void InitialLoader::setPackageList( PackageList* packages )
{
	m_packages = packages;
}

/**
 * From within the thread, emit a finishedLoading() signal to the main thread.
 */
void InitialLoader::emitFinishedLoading( PackageList* packages )
{
	FinishedLoadingEvent* event = new FinishedLoadingEvent();
	event->packages = packages;
	QApplication::postEvent( this, event );
}

/**
 * Translates QCustomEvents into signals. This function is called from Qt
 * in the main thread, which guarantees safety for emitting signals.
 */
void InitialLoader::customEvent( QCustomEvent* event )
{
	switch( event->type() )
	{
	case (int) FinishedLoadingEventType:
		emit finishedLoading( ((FinishedLoadingEvent*)event)->packages );

	default:
		ThreadedJob::customEvent( event );
		break;
	}
}

} // namespace
