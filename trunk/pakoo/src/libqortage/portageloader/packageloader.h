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

#ifndef LIBPAKTPACKAGELOADER_H
#define LIBPAKTPACKAGELOADER_H

#include "../core/threadedjob.h"


namespace libpakt {

class Package;
class PackageVersion;

/**
 * PackageLoader is a threaded job which is able to retrieve package
 * detail information. This class is responsible for filling in
 * package and version infos which were not loaded by @class InitialLoader
 * (because it would have been too resource- or time-expensive to load
 * all the info at once).
 *
 * After setting up the scanner (using at least the setPackage()
 * member function) you can call start() or perform() to begin scanning.
 *
 * @short A threaded class for retrieving detail info of a single package.
 */
class PackageLoader : public ThreadedJob
{
	Q_OBJECT

public:
	PackageLoader();
	void setPackage( Package* package );
	Package* package();

signals:
	/** Emitted every time when a package has successfully been scanned.
	 * The scanned package is given as argument. */
	void packageLoaded( Package* package );

protected:
	void emitPackageLoaded();
	void customEvent( QCustomEvent* event );

private:

	enum PackageLoaderEventType
	{
		PackageLoadedEventType = QEvent::User + 14346
	};

	//! The package that will be scanned.
	Package* m_package;


	//
	// nested event classes
	//

	class PackageLoadedEvent : public QCustomEvent
	{
	public:
		PackageLoadedEvent() : QCustomEvent( PackageLoadedEventType ) {};
		Package* package;
	};
};

}

#endif // LIBPAKTPACKAGELOADER_H
