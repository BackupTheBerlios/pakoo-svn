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

#ifndef PROFILELOADER_H
#define PROFILELOADER_H

#include "portageloaderbase.h"

class PortageSettings;

class QString;
class QDir;

/**
 * ProfileLoader is responsible for reading the current profile configuration.
 * It supports cascading profiles and gets important settings like
 * the ACCEPT_KEYWORDS value (e.g. x86 or ~alpha) or Portage directories.
 */
class ProfileLoader : public PortageLoaderBase
{
public:
	ProfileLoader();

	PortageLoaderBase::Error loadProfile( PortageSettings* settings );

protected:
	bool goToStartDirectory( QDir& dir );
	bool goToParentDirectory( QDir& currentDir );

	//! The PortageTree object that will be filled with configuration values.
	PortageSettings* settings;

private:
	void run() {}; // inherited from PortageLoaderBase, but not used here
};

#endif // PROFILELOADER_H
