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

#include <qtimer.h>

#include <kglobal.h>
#include <kiconloader.h>
#include <kdeversion.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <kkeydialog.h>
#include <kaccel.h>
#include <kconfig.h>

#include <kedittoolbar.h>

#include <kstdaccel.h>
#include <kaction.h>
#include <kstdaction.h>

#include "pakoo.h"
#include "pref.h"
#include "i18n.h"
#include "pixmapnames.h"

Pakoo::Pakoo()
    : KMainWindow( 0, "Pakoo" ),
      m_view(new PakooView(this))
{
	// tell the KMainWindow that this is indeed the main widget
	setCentralWidget(m_view);

	// then, setup our actions
	setupActions();

	// and a status bar, with progress
	statusBar()->show();
	statusBarLabel = new QLabel(0, "statusBarLabel");

	statusBarProgress = new QHBox(0, "statusBarProgress");
	statusBarProgress->setSizePolicy(
		QSizePolicy::Maximum, QSizePolicy::Preferred, false );

	statusBarProgressButton = new KPushButton(
		KGuiItem( "", "stop",
		          TOOLTIP_ABORTLOADINGPACKAGEDETAILS,
		          WHATSTHIS_ABORTLOADINGPACKAGEDETAILS ),
		statusBarProgress, "statusBarProgressButton"
	);
	statusBarProgressBar = new KProgress(
		statusBarProgress, "statusBarProgressBar" );

	statusBar()->addWidget( statusBarLabel, 1 /* id */, 1 /* stretch factor */ );
	statusBar()->addWidget( statusBarProgress, 1 /* stretch factor */,
	                        true /* permanent (on the right side) */ );
	showStatusbarProgress( false );

	// apply the saved mainwindow settings, if any, and ask the mainwindow
	// to automatically save settings if changed: window size, toolbar
	// position, icon size, etc.
	setAutoSaveSettings();

	// allow the view to change the statusbar and caption
	connect( m_view, SIGNAL(signalSetStatusbarText(const QString&)),
	         this,     SLOT(setStatusbarText(const QString&))      );
	connect( m_view, SIGNAL(signalSetCaption(const QString&)),
	         this,     SLOT(setCaption(const QString&))      );
	connect( m_view, SIGNAL(signalSetStatusbarProgress(int,int,bool)),
	         this,     SLOT(setStatusbarProgress(int,int,bool))      );
	connect( m_view, SIGNAL(signalShowStatusbarProgress(bool,bool)),
	         this,     SLOT(showStatusbarProgress(bool,bool))      );
	connect( statusBarProgressButton, SIGNAL(clicked()),
	         m_view,                    SLOT(abortProgress()) );

	// Zack Rusin's delayed initialization technique
	QTimer::singleShot( 0, m_view, SLOT(initData()) );
}

Pakoo::~Pakoo()
{
}

void Pakoo::setupActions()
{
	// TODO (?): get the i18n texts into i18n.h
	KAction *actionsInstallEbuild =
		new KAction( i18n("&Install Ebuild File..."), "open",
		             0 /* KShortcut( CTRL + Key_E ) */, this,
		             SLOT(actionsSync()), actionCollection(),
		             "installebuild"
		           );

	KAction *actionsSync =
		new KAction( i18n("&Sync"), "reload",
		             KShortcut( CTRL + Key_S ), this,
		             SLOT(actionsSync()), actionCollection(), "sync"
		           );

	KAction *actionsUpdate =
		new KAction( i18n("&Update Packages..."), "up",
		             KShortcut( CTRL + Key_U ), this,
		             SLOT(actionsUpdate()), actionCollection(), "update"
		           );

	KAction *actionsCleanUp =
		new KAction( i18n("&Clean Up..."), "editdelete",
		             KShortcut( CTRL + Key_C ), this,
		             SLOT(actionsCleanUp()), actionCollection(), "cleanup"
		           );

	KAction *actionsAdvancedSearch =
		new KAction( i18n("Advanced &Search..."), "find",
		             KShortcut( CTRL + Key_F ), this,
		             SLOT(actionsCleanUp()), actionCollection(),
		             "advancedsearch"
		           );

	KAction *actionsInstall =
		new KAction( i18n("&Install Selected..."), "button_ok",
		             KShortcut( CTRL + Key_I ), this,
		             SLOT(actionsInstall()), actionCollection(), "install"
		           );

	KAction *actionsUninstall =
		new KAction( i18n("U&ninstall Selected..."), "button_cancel",
		             KShortcut( CTRL + Key_N ), this,
		             SLOT(actionsUninstall()), actionCollection(), "uninstall"
		           );

	// File menu standard items
	KStdAction::quit( this, SLOT(slotQuit()), actionCollection() );

	// Settings menu standard items
	KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
	KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());
	//KStdAction::preferences(this, SLOT(optionsPreferences()), actionCollection());

	// disable items until they are implemented
	//TODO: delete from here once they work
	actionsInstallEbuild->setEnabled( false );
	actionsSync->setEnabled( false );
	actionsUpdate->setEnabled( false );
	actionsCleanUp->setEnabled( false );
	actionsAdvancedSearch->setEnabled( false );
	actionsInstall->setEnabled( false );
	actionsUninstall->setEnabled( false );

	m_toolbarAction = KStdAction::showToolbar(
		this, SLOT(optionsShowToolbar()), actionCollection() );
	//m_statusbarAction = KStdAction::showStatusbar(
	//	this, SLOT(optionsShowStatusbar()), actionCollection() );

	#ifdef DEBUG
	// load the XMLGUI definition file even if it's not installed
	createGUI(
		KApplication::kApplication()->applicationDirPath().replace("debug/", "")
		+ "/pakooui.rc"
	);
	#else
	createGUI();
	#endif
}

void Pakoo::actionsSync()
{
    // Perform an update of the portage package database.
    // Also known as 'emerge sync'.
    // This action is supposed to open a modal progress dialog.
}

void Pakoo::actionsUpdate()
{
    // Update packages with new versions available.
    // Also known as 'emerge -u world' or 'emerge --oneshot [packages]'
    // (depending on whether the user wants all or some of them to install).
    // This action is supposed to open a dialog where the appropriate packages
    // are displayed (in order of install or in dependency tree form)
    // and where the deep (-D) option can be turned on or off.
}

void Pakoo::actionsInstall()
{
    // Install the selected package and its dependancies.
    // Also known as 'emerge [package]'.
    // This action is supposed to open a dialog (similar to the update dialog)
    // where the appropriate packages are displayed.

    // If the selected package is already installed, ask the user if he wants
    // to rebuild it, to add it to the world file (if it isn't there already)
    // to cancel the 'install' procedure.
}

void Pakoo::actionsUninstall()
{
    // Uninstall the selected package (and maybe its dependancies, afterwards).
    // Also known as 'emerge -C [package]' (and, in case, 'emerge depclean').
    // This action is supposed to open a dialog where the uninstall procedure
    // has to be confirmed. Afterwards, if 'emerge -p depclean' would at least
    // uninstall 1 package, open up a dialog where depclean can be performed.

}

void Pakoo::actionsCleanUp()
{
    // Uninstall packages that are neither in the world file nor a dependancy
    // of another package. Also known as 'emerge depclean'.
    // Like update, open a dialog to review and optionally deselect
    // packages that are going to be uninstalled.
}


void Pakoo::optionsShowToolbar()
{
    // this is all very cut and paste code for showing/hiding the
    // toolbar
    if (m_toolbarAction->isChecked())
        toolBar()->show();
    else
        toolBar()->hide();
}

void Pakoo::optionsShowStatusbar()
{
    // this is all very cut and paste code for showing/hiding the
    // statusbar
    if (m_statusbarAction->isChecked())
        statusBar()->show();
    else
        statusBar()->hide();
}

void Pakoo::optionsConfigureKeys()
{
    KKeyDialog::configure(actionCollection());
}

void Pakoo::optionsConfigureToolbars()
{
    // use the standard toolbar editor
#if defined(KDE_MAKE_VERSION)
# if KDE_VERSION >= KDE_MAKE_VERSION(3,1,0)
    saveMainWindowSettings(KGlobal::config(), autoSaveGroup());
# else
    saveMainWindowSettings(KGlobal::config());
# endif
#else
    saveMainWindowSettings(KGlobal::config());
#endif
}

void Pakoo::newToolbarConfig()
{
    // this slot is called when user clicks "Ok" or "Apply" in the toolbar editor.
    // recreate our GUI, and re-apply the settings (e.g. "text under icons", etc.)
    createGUI();

#if defined(KDE_MAKE_VERSION)
# if KDE_VERSION >= KDE_MAKE_VERSION(3,1,0)
    applyMainWindowSettings(KGlobal::config(), autoSaveGroup());
# else
    applyMainWindowSettings(KGlobal::config());
# endif
#else
    applyMainWindowSettings(KGlobal::config());
#endif
}

void Pakoo::optionsPreferences()
{
    // popup some sort of preference dialog, here
    pakooPreferences dlg;
    if (dlg.exec())
    {
        // redo your settings
    }
}

/**
 * Set the status message that's shown in the status bar.
 */
void Pakoo::setStatusbarText(const QString& text)
{
    // display the text on the statusbar
    statusBarLabel->setText(text);
}

/**
 * Set the progress that's shown in the status bar.
 * If the progress bar is hidden, it will be shown.
 *
 * @param progress    The current amount of progress.
 * @param totalSteps  The total number of steps.
 */
void Pakoo::setStatusbarProgress( int progress, int totalSteps,
                                  bool showProgressButton )
{
	if( statusBarProgress->isHidden() ) {
		showStatusbarProgress( true, showProgressButton );
	}

	statusBarProgressBar->setTotalSteps( totalSteps );
	statusBarProgressBar->setProgress( progress );
}

/**
 * Define if the progress bar should be shown or hidden.
 * @param hide  If true, the progress bar will be hidden.
 *              If false, it will be shown.
 * @param showProgressButton  If this and the "hide" parameter are both true,
 *                            the button right next to the status bar will be
 *                            shown. Otherwise it will be hidden.
 */
void Pakoo::showStatusbarProgress( bool show, bool showProgressButton )
{
	if( show == false ) {
		statusBarProgress->hide();
	}
	else { // show == true
		statusBarProgress->show();

		if( showProgressButton == true )
			statusBarProgressButton->show();
		else
			statusBarProgressButton->hide();
	}
}

/**
 * Set the window title.
 */
void Pakoo::setCaption(const QString& text)
{
    // display the text on the caption
    setCaption(text);
}

/**
 * Called before the window is actually destroyed.
 */
bool Pakoo::queryExit()
{
	slotQuit();
	return true;
}

/**
 * Prepare for window destruction.
 */
void Pakoo::slotQuit()
{
	m_view->quit();
	kapp->quit();
}

#include "pakoo.moc"
