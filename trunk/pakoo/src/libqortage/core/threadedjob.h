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

#ifndef LIBPAKTTHREADEDJOB_H
#define LIBPAKTTHREADEDJOB_H

#include "iasyncjob.h"

#include <qthread.h>


namespace libpakt {

/**
 * A base class for jobs that need an extra thread.
 * It's derived from QThread and implements the IAsyncJob interface.
 * Its main feature, which will be obsolete when switching to Qt 4,
 * is convenient translation of QCustomEvents to signals,
 * which is very handy for the caller.
 *
 * Classes that are derived from ThreadedJob provide the start() and perform()
 * member functions for executing the job. With start(), the job is executed
 * in an own thread and emits finished( IJob::JobResult ) when it's done.
 * With perform(), the job is executed synchronously (in the current thread)
 * and returns the job result as result value of the function.
 * You may use whatever fits your current needs.
 *
 * @short  A base class for asynchronous jobs that are running as thread.
 */
class ThreadedJob : public IAsyncJob, public QThread
{
	Q_OBJECT

public:
	ThreadedJob();
	~ThreadedJob();

	IJob::JobResult perform();
	void abortAndWait();
	bool running();
	//bool wait();

	/**
	 * True if the job reports its current progress status
	 * by emitting progressChanged(). This standard implementation
	 * returns false, it should be overloaded by derived classes
	 * if they plan to emit progressChanged() signals.
	 */
	virtual bool progressEnabled() { return false; };

public slots:
	void start();
	void abort();
	void pause();

protected:
	/**
	 * This is the function that has to be implemented by every derived class.
	 * It fulfills the same purpose as QThread::run(), but has got a return
	 * value in addition, and can also be called synchronously without
	 * starting a thread. It's guaranteed that this function is only running
	 * once at a time per object.
	 *
	 * Do not emit signals from within performThread(). You have to assume
	 * that you are within a thread where emitting signals is not safe,
	 * so define emit*() functions that send custom events to the GUI thread
	 * and catch them in customEvent() from where you can emit the signals.
	 * (You can also use emitDebugOutput() and emitProgressChanged()
	 * which are already provided by the ThreadedJob class.)
	 *
	 * @return  If fulfilling the purpose successfully, this function should
	 *          return IJob::Success. On thread abortion (when aborting() ==
	 *          true) and on errors, it should emit an error message
	 *          and return IJob::Failure.
	 */
	virtual IJob::JobResult performThread() = 0;

	bool aborting();
	virtual void customEvent( QCustomEvent* event );

protected slots:

	void emitDebugOutput( const QString& output );
	void emitProgressChanged( int value, int maximum );
	void emitCurrentTaskChanged( const QString& description );

private:
	bool isAborting;

	void run();
	void emitFinished( IJob::JobResult result );

	enum ThreadedJobEventType
	{
		FinishedEventType = QEvent::User + 14330,
		ProgressChangedEventType = QEvent::User + 14331,
		DebugOutputEventType = QEvent::User + 14332,
		CurrentTaskChangedEventType = QEvent::User + 14333
	};


	//
	// nested event classes
	//

	class FinishedEvent : public QCustomEvent
	{
	public:
		FinishedEvent() : QCustomEvent( FinishedEventType ) {};
		IJob::JobResult result;
	};

	class ProgressChangedEvent : public QCustomEvent
	{
	public:
		ProgressChangedEvent() : QCustomEvent( ProgressChangedEventType ) {};
		int value, maximum;
	};

	class DebugOutputEvent : public QCustomEvent
	{
	public:
		DebugOutputEvent() : QCustomEvent( DebugOutputEventType ) {};
		QString output;
	};

	class CurrentTaskChangedEvent : public QCustomEvent
	{
	public:
		CurrentTaskChangedEvent()
			: QCustomEvent( CurrentTaskChangedEventType ) {};
		QString description;
	};

};

}

#endif // LIBPAKTTHREADEDJOB_H
