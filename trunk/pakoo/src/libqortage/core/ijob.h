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
 * A job mainly uses signals to talk back to the caller and optionally
 * provides progress numbers and status messages.
 * To execute a job, you can either use start() (which starts the job
 * asynchronously in the background) or perform() (which waits for the
 * job to finish until it returns).
 *
 * Derived classes can, but don't have to support aborting or
 * pausing/resuming the job. They can also implement progress reports
 * which can be processed by connecting to the appropriate signals.
 *
 * @short  An interface for asynchronous or blocking jobs.
 */
class IJob : public QObject
{
	Q_OBJECT

public:
	enum JobResult {
		Success, // in case of, well, success
		Failure  // on errors, or when aborting the job
	};

	IJob( QObject *parent = 0, const char *name = 0 )
		: QObject( parent, name ) {};

	/**
	 * The function that does the actual work and executes
	 * synchronously until the job is done.
	 */
	virtual IJob::JobResult perform() = 0;

	/**
	 * Determine if the job is currently running (in this case the function
	 * returns true) or not (returning false).
	 */
	virtual bool running() = 0;

	/**
	 * Wait for the job to end before this function returns.
	 * If the job is not currently running, it returns immediately.
	 */
	virtual bool wait() = 0;

	/**
	 * Tell the job to abort and wait for it to end before this function
	 * returns. If the job is not currently running, it returns immediately.
	 * If it doesn't abort even if told to, this function
	 * will block until it has finished normally.
	 */
	virtual void abortAndWait() = 0;


	/**
	 * True if the job reports its current progress status
	 * by emitting progressChanged().
	 */
	virtual bool progressEnabled() = 0;

	/*
	 * Returns true if this job is split up in one or several tasks,
	 * with a description string for each of them. In this case,
	 * the job will emit currentTaskProgressChanged().
	 * If the job is only to be viewed as a whole, these signals will not
	 * be emitted, and this function returns false.
	 *
	virtual bool currentTaskProgressEnabled() = 0;
	 */


public slots:

	/**
	 * Attempt to start or resume the job.
	 * If it has successfully been started then the started() signal
	 * will be emitted. If it has been previously paused and is now resumed,
	 * a resume() signal will be emitted.
	 * In case the job could neither be started nor resumed, it emits a
	 * finished() signal with result == Failure. This will happen if
	 * required variables have not been set, if the job is already
	 * running, or if it's already finished (or other sources of error).
	 *
	 * When the job is done it will also emit finished(), containing any
	 * of the possible JobResult values as argument.
	 */
	virtual void start() = 0;

	/**
	 * Attempt to abort the job. As job execution is asynchronous, you may not
	 * assume that the job has already been aborted when this function returns.
	 * (It's also possible that aborting the job is not supported.)
	 * Instead, you have to wait for the finished() signal if you want to
	 * make sure that the job has been aborted.
	 */
	virtual void abort() = 0;

	/**
	 * Attempt to pause the job. As job execution is asynchronous, you may not
	 * assume that the job has already been paused when this function returns.
	 * (It's also possible that pausing the job is not supported.)
	 * Instead, you have to wait for the paused() signal if you want to
	 * make sure that the job has been paused.
	 * A paused job can be resumed by doing another start() call.
	 */
	virtual void pause() = 0;


signals:

	/**
	 * Emitted if the job has successfully been started.
	 */
	void started();

	/**
	 * Emitted if the job has finished. The argument specifies
	 * how successful job execution has been.
	 */
	void finished( IJob::JobResult result );

	/**
	 * Emitted if the job has been paused (and can be resumed).
	 */
	void paused();

	/**
	 * Emitted if the job has been resumed from a previous paused state.
	 */
	void resumed();


	/**
	 * Emitted when the job progresses. The two argument values
	 * show the current progress and can be directly connected
	 * to a QProgressBar's setProgress() slot.
	 * When progressEnabled() == false, no such signals are emitted.
	 */
	void progressChanged( int value, int maximum );


	/**
	 * Emitted when the user visible task description changes. The argument
	 * contains a short, one-line description of what is currently done.
	 */
	void currentTaskChanged( const QString& description );

	/*
	 * Emitted when the current task progresses. The two argument values
	 * show the progress of the current task and can be directly connected
	 * to a QProgressBar's setProgress() slot.
	 *
	void currentTaskProgressChanged( int value, int maximum );

	 *
	 * Emitted if the state of the current task's current progress becomes
	 * enabled or disabled.
	 *
	void currentTaskProgressEnabled( bool enabled );
	 */
};

}

#endif // LIBPAKTIJOB_H
