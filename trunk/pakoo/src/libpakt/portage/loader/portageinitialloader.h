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

#ifndef LIBPAKTPORTAGEINITIALLOADER_H
#define LIBPAKTPORTAGEINITIALLOADER_H

#include "../../base/loader/initialloader.h"


namespace libpakt {

class PortageSettings;
class ProfileLoader;
class PortageTreeScanner;

/**
 * This is a job that initializes the package list with packages
 * in the Portage tree and loads global Portage settings from
 * config files. Please perform() or start() this job before
 * attempting to work with the Portage back end, because otherwise
 * there won't be any packages that you can access.
 *
 * The PortageInitialLoader itself makes use of the ProfileLoader
 * and the PortageTreeScanner.
 */
class PortageInitialLoader : public InitialLoader
{
	Q_OBJECT

public:
	PortageInitialLoader();

	void setSettingsObject( PortageSettings* settings ); // Portage specific

	bool progressEnabled() { return true; }

public slots:
	void abort();

signals:
	/** For internal use only. Emitted when abort() is called. */
	void aborted();

protected:
    IJob::JobResult performThread();
    void customEvent(QCustomEvent* event);

private slots:
	void emitFinishedLoading( PackageList* packages );
	void emitPackagesScanned( int packageCountAvailable,
	                          int packageCountInstalled );

private:
	enum PortageInitialLoaderEventType
	{
		PortageFinishedLoadingEventType = QEvent::User + 14345
	};

	//! The PortageTree object that will be filled with configuration values.
	PortageSettings* m_settings;


	//
	// nested event classes
	//

	class PortageFinishedLoadingEvent : public QCustomEvent
	{
	public:
		PortageFinishedLoadingEvent()
			: QCustomEvent( PortageFinishedLoadingEventType ) {};
	};

};

}

#endif
