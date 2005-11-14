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

#include "portagebackend.h"

#include "portage/core/portagepackage.h"
#include "core/packagelist.h"
#include "portage/core/portagesettings.h"
#include "portage/core/portagecategory.h"
#include "portage/loader/portagepackageloader.h"
#include "portage/loader/portageinitialloader.h"

namespace libpakt {

PortageBackend::PortageBackend() : BackendFactory()
{
	// The settings object is used internally
	// for all kinds of Portage settings
	portageSettings = new PortageSettings();

	// These settings can't be retrieved automatically
	
	// You can get the cache type from /etc/portage/modules
	// If the file don't exists use FlatCache
	// It it exists: read it to see what cache type to use.
	// If it isn't CDB then read the portage tree.
	// The others cache types are:
	//   AnyDBM (included in portage) and MySQL
	// There are one more (cpickle) in /usr/lib/portage/pym/
	// but I don't know anything about it.
	//portageSettings->setPreferredPackageSource( FlatCache );
	portageSettings->setPreferredPackageSource( PortageTree );
	//TODO: Read these from a configuration file of some kind.
	portageSettings->setInstalledPackagesDirectory("/var/db/pkg/");
	portageSettings->setCacheDirectory("/var/cache/edb/dep/");
}

PortageSettings* PortageBackend::settings()
{
	return portageSettings;
}

PackageList* PortageBackend::createPackageList() {
	return new TemplatedPackageList<PortagePackage>();
}

PackageCategory* PortageBackend::createPackageCategory()
{
	return new PortageCategory();
}

InitialLoader* PortageBackend::createInitialLoader()
{
	PortageInitialLoader* loader = new PortageInitialLoader();
	loader->setSettingsObject( portageSettings );
	return loader;
}

PackageLoader* PortageBackend::createPackageLoader()
{
	PortagePackageLoader* loader = new PortagePackageLoader();
	loader->setSettingsObject( portageSettings );
	return loader;
}

} // namespace
