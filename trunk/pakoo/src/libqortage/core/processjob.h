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

#ifndef LIBPAKTPROCESSJOB_H
#define LIBPAKTPROCESSJOB_H

#include "ijob.h"

#include <kprocess.h>


namespace libpakt {

/**
 * A base class for jobs that invoke another process.
 * It implements the IJob interface and simplifies usage of processes,
 * using KExtProcess.
 *
 * Classes that are derived from ProcessJob provide the start() and perform()
 * member functions for executing the job. With start(), the job is executed
 * asynchronously and emits finished( IJob::JobResult ) when it's done.
 * With perform(), the job is executed synchronously (in other words,
 * blocking) and returns the job result as result value of the function.
 * You may use whatever fits your current needs.
 *
 * @short  A base class for asynchronous jobs that require external processes.
 */
class ProcessJob : public IJob
{
	Q_OBJECT

public:
	ProcessJob( QObject *parent = 0, const char *name = 0 );
	~ProcessJob();

	IJob::JobResult perform();
	void abortAndWait();
	bool running();
	bool wait();

	/**
	 * True if the job reports its current progress status
	 * by emitting progressChanged(). This standard implementation
	 * returns false, it should be overloaded by derived classes
	 * if they plan to emit progressChanged() signals.
	 */
	bool progressEnabled() { return false; }

public slots:
	void start();
	void abort();
	void pause();

signals:
	/**
	 * Emitted when output from the child process has been received,
	 * like seen on terminal output. This is a combination of stdout
	 * and stderr.
	 *
	 * @param output  The data that has been received.
	 */
	void receivedOutput( const QString& output );

protected:
	enum ProcessType {
		LocalProcess,
		SuProcess
	};

	/**
	 * To be defined by any derived class.
	 * Determines if the process has to be run with the privileges of
	 * the current user (ProcessJob::LocalProcess) or if it needs root
	 * privileges to be successful (ProcessJob::SuProcess).
	 */
	virtual ProcessType processType() = 0;

	/**
	 * To be defined by any derived class.
	 * This function has to return a QStringList containing the name
	 * and the arguments of the process that will be started.
	 * If processType() is SuProcess, this function doesn't have to
	 * take care of privileges, because ProcessJob already does that
	 * for you.
	 *
	 * The implementation, for example, could look similar to this:
	 * \code
	 * QStringList list;
	 * list << "killall";
	 * list << "-9" << "pakoo";
	 * return list;
	 * \endcode
	 */
	virtual QStringList processNameAndArguments() = 0;

	/**
	 * This function is called right before the process is started.
	 * It can be overloaded to set up additional process properties,
	 * like the process priority or usage of the terminal.
	 * It's also very useful to connect signals from the process object
	 * in order to process them (track the status, or other gimmicks).
	 *
	 * This default implementation does absolutely nothing.
	 */
	virtual void init() {};

	virtual KProcess::Communication processCommunication();

	virtual JobResult jobResult();

private slots:
	void processExited();
	void handleOutput( KProcess*, char* buffer, int buflen );

private:
	void startProcess( KProcess::RunMode runMode );

	/** The process which is run by this job. */
	KProcess* m_process;
};

}

#endif // LIBPAKTPROCESSJOB_H
