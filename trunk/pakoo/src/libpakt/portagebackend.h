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

#ifndef LIBPAKTPORTAGEBACKEND_H
#define LIBPAKTPORTAGEBACKEND_H

#include <backendfactory.h>


namespace libpakt {

class PortageSettings;

/**
 * A concrete BackendFactory implementation returning objects
 * specific to Gentoo's Portage package management system.
 */
class PortageBackend : public BackendFactory
{
public:
	PortageBackend();
	~PortageBackend();
	//! Return the PortageSettings object containing the global configuration.
	PortageSettings* settings();

	bool hasLoaderClasses()    { return true; }
	bool hasInstallerClasses() { return false; }
	bool hasConfigClasses()    { return false; }

	PackageList* createPackageList();
	PackageCategory* createPackageCategory();
	InitialLoader* createInitialLoader();
	PackageLoader* createPackageLoader();

private:
	PortageSettings* portageSettings;
};

}

#endif
