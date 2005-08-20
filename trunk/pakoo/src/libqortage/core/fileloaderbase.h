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

#ifndef LIBPAKTFILELOADERBASE_H
#define LIBPAKTFILELOADERBASE_H

#include <qstring.h>

#include "threadedjob.h"


namespace libpakt {

/**
 * This is a base class for derived ones that read files line by line.
 *
 * To use it, set it up calling the setFileName member function
 * (and probably other ones from the derived classes), then call
 * perform() to process the file.
 *
 * @short A base class which simplifies reading from a file.
 */
class FileLoaderBase : public ThreadedJob
{
	Q_OBJECT

public:
	FileLoaderBase();

	void setFileName( const QString& filename );

	IJob::JobResult performThread();

protected:
	//! The file that will be read or written.
	QString filename;

	/**
	 * This purely virtual function is called by perform() for every line
	 * in the file if isLineProcessed(line) returns true.
	 *
	 * @param line  The current line that has been read from the file.
	 */
	virtual void processLine( const QString& line ) = 0;

	/**
	 * Called right at the beginning of the perform() member function.
	 * You can overload this function to check if variables have been
	 * initialized, or other consistency checks. Derived classes don't
	 * have to check if the file exists, this is done by perform()
	 * itself.
	 *
	 * If this function returns false, perform() exits with IJob::Failure
	 * as return value. (By default, it always returns true.)
	 */
	virtual bool check() { return true; }

	/**
	 * Called by perform() after the file has been opened successfully.
	 * You can overload this function to initialize variables that are
	 * needed all through the whole file loading process.
	 *
	 * If this function returns false, perform() exits with IJob::Failure
	 * as return value. (By default, it always returns true.)
	 */
	virtual bool init() { return true; }

	// documentation in the .cpp file
	virtual bool isLineProcessed( const QString& line );

	/**
	 * Called by perform() after each line that has been processed.
	 * If this function returns true, perform() immediately aborts
	 * reading the file and proceeds directly with finish().
	 *
	 * You can overload this function if you don't need to read
	 * the remaining lines. (By default, it always returns false.)
	 */
	virtual bool aborting() { return false; }

	/**
	 * Called by perform() after reading all lines and right before
	 * returning to the caller. You can overload this function to
	 * clean up and determine if job execution was successful or not.
	 *
	 * The result value of this function will be the one of perform().
	 * (By default, it always returns IJob::Success.)
	 */
	virtual IJob::JobResult finish() { return Success; }
};

}

#endif // LIBPAKTFILELOADERBASE_H
