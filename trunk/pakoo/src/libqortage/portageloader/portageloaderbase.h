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

#ifndef LIBPAKTPORTAGELOADERBASE_H
#define LIBPAKTPORTAGELOADERBASE_H

#include <qthread.h>

/**
 * PortageLoaderBase is an abstract base class for threaded classes which
 * load, modify and/or save core objects like PortageTree, PortageSettings,
 * or similar. It's derived from QThread with protected access permissions
 * and provides a few of the QThread functions as public ones again.
 *
 * Each derived class should at least provide one public startSomething(...)
 * member function that replaces QThread::start() by setting configuration
 * and calling start() itself (it can't be called from outside, because
 * QThread is derived protected). The run() function should also be kept
 * protected in order to prevent wrong or uninitialized settings.
 */
class PortageLoaderBase : protected QThread
{
public:
	//! Return values for load/save functions.
	enum Error
	{
		// no error - great, success!
		NoError,

		// Thread errors
		AbortedError,        // when the thread is being aborted before it finishes.
		AlreadyRunningError, // when the thread is already running.
		                     // (Only one function should be running at the same time.)

		// General errors
		NullObjectError, // when the given object (PortageTree, PortageSettings, ...) is NULL.
		OpenFileError,   // when the file can't be opened

		// PortageML specific errors
		FileTypeError,   // when the doctype header isn't "portageML"
		RootElementError // when the root element is named anything but "portagetree"
	};

	PortageLoaderBase();

	bool running();
	bool wait();
	void abort();

protected:
	void initProcessing();
	void finishProcessing();

	/**
	 * This object receives custom events which can be caught
	 * by overloading QObject's customEvent(...) function.
	 */
	QObject* receiver;

	//! Set to true if the thread should should abort.
	bool stop;

	/**
	 * Set to true when processing starts and to false when it ends.
	 * That way, it can be used to prevent the processing function from
	 * being called twice the same time.
	 */
	bool working;
};

#endif // LIBPAKTPORTAGELOADERBASE_H
