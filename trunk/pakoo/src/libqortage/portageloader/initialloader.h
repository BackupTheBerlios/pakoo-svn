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

#ifndef LIBPAKTINITIALLOADER_H
#define LIBPAKTINITIALLOADER_H

#include "../core/threadedjob.h"


namespace libpakt {

class PackageList;

/**
 * This is a job that initializes the package list with packages in the
 * package tree, loads global settings from config files, and/or does other
 * initialization work (depending on the specific backend). Please perform()
 * or start() this job before attempting to work with the back end,
 * because otherwise there won't be any packages that you can access.
 */
class InitialLoader : public ThreadedJob
{
	Q_OBJECT

public:
	InitialLoader();

	void setPackageList( PackageList* packages );

signals:
	/**
	 * Emitted if the package tree has successfully been loaded from disk.
	 * The PackageList object now contains all packages and package versions,
	 * but without detailed package information.
	 */
	void finishedLoading( PackageList* packages );

protected slots:
	void emitFinishedLoading( PackageList* packages );

protected:
	void customEvent(QCustomEvent* event);

	//! The PackageList object that will be filled with packages.
	PackageList* packages;

private:
	enum InitialLoaderEventType
	{
		FinishedLoadingEventType = QEvent::User + 14344
	};


	//
	// nested event classes
	//

	class FinishedLoadingEvent : public QCustomEvent
	{
	public:
		FinishedLoadingEvent() : QCustomEvent( FinishedLoadingEventType ) {};
		PackageList* packages;
	};

};

}

#endif // LIBPAKTINITIALLOADER_H
