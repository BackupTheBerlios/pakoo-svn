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

#include "processjob.h"

#include <qstringlist.h>

#include <kuser.h>


namespace libpakt {

ProcessJob::ProcessJob( QObject *parent, const char *name )
	: IJob( parent, name )
{
	m_process = new KProcess( this );
}

/**
 * Destructor, killing the process.
 */
ProcessJob::~ProcessJob()
{
	abortAndWait();

	if( m_process != NULL )
		delete m_process;
}

/**
 * Execute synchronously, waiting for the process to end.
 *
 * @return  Tells if the process finished successfully (IJob::Success) or not
 *          (IJob::Failure). The finished() signal is not emitted.
 */
IJob::JobResult ProcessJob::perform()
{
	startProcess( KProcess::Block );
	return jobResult();
}

/**
 * Begins execution of the process.
 * When execution ends, the finished() signal is emitted.
 */
void ProcessJob::start()
{
	connect( m_process, SIGNAL( processExited(KProcess*) ),
	         this,        SLOT( processExited() ) );
	startProcess( KProcess::NotifyOnExit );
}

/**
 * Actually starts the progress, without connecting the finished()
 * signal. This is actually the shared part between perform()
 * and start().
 */
void ProcessJob::startProcess( KProcess::RunMode runMode )
{
	// write the name and argument of the process
	m_process->clearArguments();
	switch( processType() )
	{
	case SuProcess:
		if( !KUser().isSuperUser() ) {
			*m_process << "kdesu";
		}
		// and continue here:
	case LocalProcess:
		*m_process << processNameAndArguments();
		break;
	default:
		break;
	}

	// let's start the process!
	init(); // letting the derived classes do their initialization
	connect( m_process, SIGNAL( receivedStdout(KProcess*,char*,int) ),
	         this,        SLOT( handleOutput(char*,int) ) );
	connect( m_process, SIGNAL( receivedStderr(KProcess*,char*,int) ),
	         this,        SLOT( handleOutput(char*,int) ) );
	emit receievedOutput( processNameAndArguments().join(" ") );
	m_process->start( runMode, processCommunication() );
}

/**
 * Determine if the process is running.
 * Returns the result of KProcess::isRunning().
 */
bool ProcessJob::running()
{
	return m_process->isRunning();
}

// Take documentation from IJob doxygen.
void ProcessJob::abort()
{
	if( !running() )
		return;

	// try to terminate the process
	m_process->kill(SIGTERM);
	m_process->wait( 3 ); // meaning wait max. 10 secs for shutting down

	if( m_process->isRunning() ) {
		m_process->kill( SIGKILL ); // still not shut down, so kill it
	}
}

/**
 * Tell the process to abort (using the abort() function)
 * and wait for it to end before this function returns.
 * If the process is not currently running, it returns immediately.
 * This function is provided for convenience.
 */
void ProcessJob::abortAndWait()
{
	if( running() ) {
		abort();
		wait();
	}
}

/**
 * Wait for the process to end before this function returns.
 * If the process is not currently running, it returns immediately.
 * The return value is the one from KProcess::wait() and is always true
 * (since no timeout is being used in this implementation).
 */
bool ProcessJob::wait()
{
	return m_process->wait();
}

/**
 * Not (yet?) supported by ProcessJob.
 * In this version, it doesn't do anything, and specifically doesn't
 * emit a paused() signal.
 *
 * Theoretically, it just has to be possible to suspend a job -
 * after all, bash can do it too!
 */
void ProcessJob::pause()
{}

/**
 * This slot is called when the process exits and emits an appropriate
 * finished() signal.
 */
void ProcessJob::processExited()
{
	emit finished( jobResult() );

	disconnect( m_process, SIGNAL( processExited(KProcess*) ),
	            this,        SLOT( processExited() ) );
}

/**
 * This slot is called when output on stdout or stderr is received,
 * it transforms it into a QString and emits the output() signal.
 */
void ProcessJob::handleOutput( char* buffer, int buflen )
{
	const QString output = QString::fromLatin1( buffer, buflen );
	emit receievedOutput( output );
}


/**
 * This function determines if the process has done its job successfully.
 * It defaults to IJob::Success if the process has exited normally and with
 * an exit status of 0, and IJob::Failure in all other cases.
 *
 * You can replace this function with your own "result handler".
 * When it is called, you can assume that the process has already exited.
 */
IJob::JobResult ProcessJob::jobResult()
{
	if( m_process->normalExit() == true && m_process->exitStatus() == 0)
		return Success;
	else
		return Failure;
}

/**
 * Determines which communication links are opened to the process.
 * By default this is KProcess::NoCommunication, so if you want
 * to read stdout or stderr or read stdin, you want to overload
 * this function.
 */
KProcess::Communication ProcessJob::processCommunication()
{
	return KProcess::AllOutput;
}

} // namespace
