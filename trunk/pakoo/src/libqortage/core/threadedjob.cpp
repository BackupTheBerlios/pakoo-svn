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

#include <qapplication.h>

#include "threadedjob.h"


namespace libpakt {

ThreadedJob::ThreadedJob() : IAsyncJob()
{}

/**
 * Destructor, aborts and waits for the thread if it's still running.
 */
ThreadedJob::~ThreadedJob()
{
	if( running() ) {
		abort();
		wait();
	}
	qApp->processEvents();
	this->disconnect();
}

/**
 * Start execution of the thread and emit a started() signal.
 * If the thread is already running, this function waits for the running
 * thread to end (without aborting) before starting a new one.
 * (If you want multiple threads of the same class, you'll have to
 * create multiple instances.)
 */
void ThreadedJob::start()
{
	if( running() )
		wait();

	isAborting = false;

	QThread::start();
	emit started();
}

/**
 * Execute synchronously without starting a new thread.
 * If the thread is already running, this function waits for the running
 * thread to end (without aborting) before going at it again.
 *
 * @return  If the thread finished successfully (IJob::Success) or not
 *          (IJob::Failure). The finished() signal is not emitted.
 */
IJob::JobResult ThreadedJob::perform()
{
	if( running() )
		wait();

	isAborting = false;

	return performThread();
}

/**
 * Calls the derived class's performThread() member function and
 * emits its return value with emitFinished().
 */
void ThreadedJob::run()
{
	// execute the code and emit its result as success indicator
	emitFinished( performThread() );
}

/**
 * Determine if the thread is running.
 * Returns the result of QThread::running().
 */
bool ThreadedJob::running()
{
	return QThread::running();
}

// Take documentation from IJob doxygen.
void ThreadedJob::abort()
{
	isAborting = true;
}

/**
 * Tell the thread to abort (using the abort() function)
 * and wait for it to end before this function returns.
 * If the thread is not currently running, it returns immediately.
 * This function is provided for convenience.
 */
void ThreadedJob::abortAndWait()
{
	if( running() ) {
		abort();
		wait();
	}
}

/**
 * Determine if the thread should be aborted.
 * Running threads should check regularly on this
 * and stop as soon as it makes sense if true is returned.
 */
bool ThreadedJob::aborting()
{
	return isAborting;
}


/**
 * From within the thread, emit a finished() signal to the main thread.
 * This has to be called exactly once in thread execution.
 */
void ThreadedJob::emitFinished( IJob::JobResult result )
{
	FinishedEvent* event = new FinishedEvent();
	event->result = result;
	QApplication::postEvent( this, event );
}

/**
 * From within the thread, emit a progressChanged() signal to the main thread.
 */
void ThreadedJob::emitProgressChanged( int value, int maximum )
{
	ProgressChangedEvent* event = new ProgressChangedEvent();
	event->value = value;
	event->maximum = maximum;
	QApplication::postEvent( this, event );
}

/**
 * From within the thread, emit a debugOutput() signal to the main thread.
 */
void ThreadedJob::emitDebugOutput( const QString& output )
{
	DebugOutputEvent* event = new DebugOutputEvent();
	event->output = output;
	QApplication::postEvent( this, event );
}

/**
 * From within the thread, emit a currentTaskChanged() signal
 * to the main thread.
 */
void ThreadedJob::emitCurrentTaskChanged( const QString& description )
{
	CurrentTaskChangedEvent* event = new CurrentTaskChangedEvent();
	event->description = description;
	QApplication::postEvent( this, event );
}


/**
 * Translates QCustomEvents into signals. This function is called from Qt
 * in the main thread, which guarantees safety for emitting signals.
 */
void ThreadedJob::customEvent( QCustomEvent* event )
{
	switch( event->type() )
	{
	case (int) FinishedEventType:
		emit IAsyncJob::finished( ((FinishedEvent*)event)->result );
		break;

	case (int) ProgressChangedEventType:
		ProgressChangedEvent* pcEvent = (ProgressChangedEvent*) event;
		emit progressChanged( pcEvent->value, pcEvent->maximum );
		break;

	case (int) DebugOutputEventType:
		emit debugOutput( ((DebugOutputEvent*)event)->output );
		break;

	case (int) CurrentTaskChangedEventType:
		emit currentTaskChanged(
			((CurrentTaskChangedEvent*)event)->description );
		break;

	default:
		break;
	}
}



/**
 * Not supported by ThreadedJob. Derived classes are free to implement
 * pausing the thread, but by default this function doesn't do anything.
 * No pausing, no signals, no setting flags.
 */
void ThreadedJob::pause()
{}

} // namespace
