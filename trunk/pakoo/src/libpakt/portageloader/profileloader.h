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

#ifndef LIBPAKTPROFILELOADER_H
#define LIBPAKTPROFILELOADER_H

#include "../core/threadedjob.h"

class QString;
class QDir;


namespace libpakt {

class PortageSettings;

/**
 * ProfileLoader is responsible for reading the current profile configuration.
 * It supports cascading profiles and gets important settings like
 * the ACCEPT_KEYWORDS value (e.g. x86 or ~alpha) or Portage directories.
 *
 * Like all jobs derived from ThreadedJob, you can call start() or perform()
 * to execute it.
 */
class ProfileLoader : public ThreadedJob
{
	Q_OBJECT

public:
	ProfileLoader();

	void setSettingsObject( PortageSettings* settings );

	IJob::JobResult performThread();

private:
	bool goToStartDirectory( QDir& dir );
	bool goToParentDirectory( QDir& currentDir );

	//! The PortageSettings object that will be filled with configuration values.
	PortageSettings* m_settings;
};

}

#endif // LIBPAKTPROFILELOADER_H
