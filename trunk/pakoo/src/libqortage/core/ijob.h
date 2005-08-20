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

#ifndef LIBPAKTIJOB_H
#define LIBPAKTIJOB_H

#include <qobject.h>
#include <qstring.h>


namespace libpakt {

/**
 * An interface for a job. It uses signals to communicate with the caller.
 *
 * @short  An interface for asynchronous background jobs.
 */
class IJob : public QObject
{
	Q_OBJECT

public:
	enum JobResult {
		Success, // in case of, well, success
		Failure  // on errors, or when aborting the thread
	};

	IJob() {};

	/**
	 * The function that does the actual work and executes
	 * synchronously until the job is done.
	 */
	virtual JobResult perform() = 0;

signals:

	/**
	 * Emitted for untranslated textual output of this job.
	 * This will most likely be the console output of the process
	 * in behind, or maybe some debug messages, or may not be emitted at all.
	 */
	void debugOutput( const QString& output );
};

}

#endif // LIBPAKTIJOB_H
