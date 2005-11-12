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

#ifndef PAKOOJOBVIEW_H
#define PAKOOJOBVIEW_H

#include <core/ijob.h>
#include <qwidget.h>

class QVBox;
class QLabel;
class KProgress;
class KTextEdit;
class KPushButton;

using libpakt::IJob;


/**
 * @short A class displaying the progress (and more) of an IJob.
 *
 * An ActionView acts as a front end to classes derived from IJob,
 * visualizing output like the current task, progress or status messages.
 */
class JobView : public QWidget
{
Q_OBJECT

public:
	JobView( libpakt::IJob* job, const QString& title,
	         QWidget* parent = 0, const char* name = 0 );
	libpakt::IJob* job();

signals:
	/**
	 * Emitted when the job has finished. The argument specifies
	 * how successful job execution has been.
	 */
	void finished( libpakt::IJob::JobResult result );

public slots:
	virtual void start();
	void setAutoClose( bool autoClose );

private slots:
	void setConsoleOutputVisible( bool showConsoleOutput );
	void jobFinished( IJob::JobResult result );
	void close();

private:
	libpakt::IJob* m_job;

	// child widgets
	QVBox* m_defaultView;
	KProgress* m_progress;
	QLabel* m_currentTaskLabel;
	KTextEdit* m_statusMessages;
	KTextEdit* m_consoleOutput;
	KPushButton* m_consoleOutputButton;
	KPushButton* m_closeButton;

	// other stuff
	bool m_autoClose;
	libpakt::IJob::JobResult m_result; // temporary result storage
};

#endif // PAKOOJOBVIEW_H
