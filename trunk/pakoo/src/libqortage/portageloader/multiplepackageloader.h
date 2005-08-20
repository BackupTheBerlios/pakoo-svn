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

#ifndef LIBPAKTMULTIPLEPACKAGELOADER_H
#define LIBPAKTMULTIPLEPACKAGELOADER_H

#include "../core/threadedjob.h"


namespace libpakt {

class PackageLoader;
class PackageList;

/**
 * MultiplePackageLoader is a threaded job which uses a PackageLoader
 * object to scan a list of packages for package details.
 * When scanning, the settings of the PackageLoader are used.
 *
 * After setting up the loader (using at least the setPackageLoader and
 * setPackageList() member functions) you can call start() or perform()
 * to begin loading the packages.
 *
 * You might want to connect to the PackageLoader's packageLoaded() signal
 * if you want to know which packages now contain detailed package info.
 *
 * @short A threaded class for retrieving detail info of multiple packages.
 */
class MultiplePackageLoader : public ThreadedJob
{
	// By the way, don't even think of deriving this from any PackageLoader,
	// because those loaders are backend-specific, and MultiplePackageLoader
	// is NOT.

	Q_OBJECT

public:
	MultiplePackageLoader( PackageLoader* loader );
	~MultiplePackageLoader();

	PackageLoader* packageLoader();
	void setPackageLoader( PackageLoader* loader );
	void setAutoDeletePackageLoader( bool autoDelete );

	void setPackageList( PackageList* packages );

signals:
	/**
	 * Emitted for untranslated debug output, like starting the scan
	 * or error messages.
	 */
	void debugOutput( QString output );

protected:
	JobResult performThread();

private:
	PackageLoader* loader;
	PackageList* packages;
	bool autoDeleteLoader;
};

}

#endif // LIBPAKTMULTIPLEPACKAGELOADER_H
