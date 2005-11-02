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

#include "actionview.h"

#include "installeractionview.h"

#include <qlayout.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qlabel.h>

#include <kprogress.h>
#include <ktextedit.h>
#include <kpushbutton.h>
#include <kdialog.h>
#include <klocale.h>


/**
 * Initialize this object. The IJob object given as argument is expected
 * to be set up already and will be deleted together with this widget.
 */
JobView::JobView( IJob* job, const QString& title,
                  QWidget* parent, const char* name )
	: QWidget( parent, name )
{
	m_job = job;
	m_result = IJob::Failure;

	QVBoxLayout* topLayout = new QVBoxLayout( this );
	topLayout->setSpacing( KDialog::spacingHint() );
	topLayout->addItem(
		new QSpacerItem( 1, KDialog::marginHint(),
		                 QSizePolicy::Expanding, QSizePolicy::Fixed )
	);

	//
	// creating the widgets
	//

	topLayout->addWidget(
		new QLabel( "<b>" + title + "</b>", this, "captionLabel" )
	);

	// default view (progress bar, current task and status messages)
	m_defaultView = new QVBox( this, "defaultView" );
	m_progress = new KProgress( m_defaultView, "currentProgress" );
	m_currentTaskLabel = new QLabel( m_defaultView, "currentTaskLabel" );
	m_statusMessages = new KTextEdit( m_defaultView );
	m_statusMessages->setTextFormat( Qt::LogText );
	topLayout->addWidget( m_defaultView );

	// the process log, which is hidden by default
	// (and can be shown instead of the defaultView)
	m_consoleOutput = new KTextEdit( this );
	m_consoleOutput->setTextFormat( Qt::LogText );
	m_consoleOutput->hide();
	topLayout->addWidget( m_consoleOutput );

	// buttons at the bottom
	m_consoleOutputButton = new KPushButton(
		KGuiItem( i18n("JobView Button Caption", "View Console Output"),
		          "terminal", // icon name
		          QString::null, // tool tip
		          i18n("JobView Button Whatsthis (\"View Console Output\")",
		               "Toggles between the standard view and the raw "
		               "console output of the underlying program.")
		        ),
		this, "consoleOutputButton"
	);
	m_consoleOutputButton->setToggleButton( true );
	m_consoleOutputButton->setSizePolicy( QSizePolicy::Fixed,
	                                      QSizePolicy::Fixed );

	m_closeButton = new KPushButton(
		KGuiItem( i18n("JobView Button Caption", "Abort"),
		          "stop", // icon name
		          i18n("JobView Button Tooltip (\"Abort\")",
		               "Stops executing the action, "
		               "which will not finish successfully."), // tool tip
		          i18n("JobView Button Whatsthis (\"Abort\")",
		               "Toggles between the standard view and the raw "
		               "console output of the underlying program.")
		        ),
		this, "closeButton"
	);
	m_closeButton->setSizePolicy( QSizePolicy::Fixed,
	                              QSizePolicy::Fixed );

	QHBoxLayout* bottomButtons = new QHBoxLayout( topLayout );
	bottomButtons->addWidget( m_consoleOutputButton );
	bottomButtons->addItem( new QSpacerItem(1, 1) );
	bottomButtons->addWidget( m_closeButton );

	// end of widget creation
	topLayout->addItem(
		new QSpacerItem( 1, KDialog::marginHint(),
		                 QSizePolicy::Expanding, QSizePolicy::Fixed )
	);


	//
	// avoiding errors
	//

	// turn off in case we got a NULL job, which would be bad
	if( m_job == NULL ) {
		m_consoleOutputButton->setEnabled( false );
		m_closeButton->setGuiItem( KStdGuiItem::Close );
		return;
	}

	// make sure that the job won't be deleted by another parent QObject
	QObject* jobParent = m_job->parent();
	if( jobParent != 0 )
		jobParent->removeChild( m_job );
	insertChild( m_job );


	//
	// setting the widgets up
	//

	if( m_job->progressEnabled() == false ) {
		m_progress->hide();
		m_currentTaskLabel->setText(
			i18n("JobView progress message when no progress is available",
			     "Please wait while the job is being processed...")
		);
	}

	// don't show process outputs for jobs that are no processes
	if( m_job->inherits("libpakt::ProcessJob") == false ) {
		m_consoleOutputButton->hide();
	}
	else {
		if( m_job->progressEnabled() == false
		    && m_job->statusMessagesEnabled() == false )
		{
			// no progress or status, so rather drop the default view
			m_consoleOutputButton->hide();
			setConsoleOutputVisible( true );
		}
		else {
			// let the "View Console Output" button toggle the view
			connect( m_consoleOutputButton, SIGNAL( toggled(bool) ),
			         this,    SLOT( setConsoleOutputVisible(bool) ) );
		}
		connect( m_job,   SIGNAL( receivedOutput(const QString&) ),
		         m_consoleOutput,   SLOT( append(const QString&) ) );
	}

	// other connections
	connect( m_job,   SIGNAL( currentTaskChanged(const QString&) ),
	         m_currentTaskLabel,   SLOT( setText(const QString&) ) );
	connect( m_job,           SIGNAL( progressChanged(int,int) ),
	         m_progress, SLOT( setProgress(int,int) ) );
	connect( m_job,  SIGNAL(statusMessage(const QString&, IJob::MessageType)),
	         m_consoleOutput, SLOT(append(const QString&)) );
	connect( m_job, SIGNAL( finished(IJob::JobResult) ),
	         this,    SLOT( jobFinished(IJob::JobResult) ) );


	/* maybe use later for pretty printing:
	QStyleSheetItem * item =
		new QStyleSheetItem( m_statusMessages->styleSheet(), "mytag" );
	item->setColor( "red" );
	item->setFontWeight( QFont::Bold );
	item->setFontUnderline( TRUE );
	*/
}

/**
 * Returns the job that is visualized by this JobView.
 */
IJob* JobView::job()
{
	return m_job;
}

/**
 * Start this JobView's job. Afterwards, you get a finished() signal
 * telling if the job has been successful or not.
 *
 * @see finished(IJob::JobResult)
 * @see setAutoClose(bool)
 */
void JobView::start()
{
	if( m_job == NULL )
		return;

	m_job->start();
}

/**
 * When given true as argument, the console output is shown instead of
 * the default view. When the argument is false (which is also the default),
 * the console output will be hidden in favour of the standard view.
 */
void JobView::setConsoleOutputVisible( bool showConsoleOutput )
{
	if( showConsoleOutput == true ) {
		m_defaultView->hide();
		m_consoleOutput->show();
	}
	else {
		m_consoleOutput->hide();
		m_defaultView->show();
	}
}

/**
 * Set or unset autoClose. If it's set true, the JobView will
 * emit the finished() signal right after the job has completed,
 * otherwise it waits until the user has confirmed that he is done.
 *
 * By default, autoClose is turned off (and the user's got the power).
 */
void JobView::setAutoClose( bool autoClose )
{
	m_autoClose = autoClose;
}

/**
 * Called when the IJob object has finished.
 * If autoClose is set true, the JobView quits immediately,
 * emitting the same finished() signal into the big bad world.
 * If autoClose is set false (which is the default) the JobView
 * waits for the user to click on the close button before the
 * signal is emitted.
 */
void JobView::jobFinished( IJob::JobResult result )
{
	m_result = result;
	if( result == IJob::Success ) {
		m_statusMessages->append( i18n("JobView status message output",
		                               "Finished successfully.") );
	}
	else {
		m_statusMessages->append( i18n("JobView status message output",
		                               "Could not finish successfully.") );
	}

	if( m_autoClose == true )
		close();
	else {
		m_closeButton->setEnabled( true );
		m_closeButton->setGuiItem( KStdGuiItem::Close );
	}
}

/**
 * Called when the JobView is exiting.
 */
void JobView::close()
{
	if( m_job == NULL )
		return;

	if( m_job->running() ) {
		m_closeButton->setEnabled( false );
		m_job->abort();
		//TODO: wait cursor
	}
	else {
		emit finished( m_result );
	}
}
