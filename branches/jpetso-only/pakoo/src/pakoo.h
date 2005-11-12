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


#ifndef PAKOO_H
#define PAKOO_H

//#ifdef HAVE_CONFIG_H
#include <config.h>
//#endif

// comment out when doing a release
#define DEVELOPMENT_VERSION

#define APPNAME I18N_NOOP("Pakoo")

#include <qhbox.h>

#include <kapplication.h>
#include <kmainwindow.h>
#include <kprogress.h> // progress indicator for the status bar
#include <kpushbutton.h>
#include <kactionclasses.h>

#include "pakooview.h"


/**
 * This class serves as the main window for pakoo.  It handles the
 * menus, toolbars, and status bars.
 *
 * @short Main window class
 * @author Jakob Petsovits <jpetso@gmx.at>
 * @version 0.1
 */
class Pakoo : public KMainWindow
{
	Q_OBJECT
public:
	/**
		* Default Constructor
		*/
	Pakoo();

	/**
		* Default Destructor
		*/
	virtual ~Pakoo();


public slots:
	void slotQuit();

	void setCaption(const QString& text);
	void setStatusbarText( const QString& text );
	void setStatusbarProgress( int progress, int totalSteps );
	void showStatusbarProgress( bool show = true );
	void hideStatusbarProgress() {
		showStatusbarProgress(false);
	}

	void showStatusbarProgressButton( bool showProgressButton = true );

	void hideStatusbarProgressButton() {
		showStatusbarProgressButton(false);
	}

protected:
	//! Reimplemented to call slotQuit().
	bool queryExit();


private slots:
    void actionsSync();
    void actionsUpdate();
    void actionsInstall();
    void actionsUninstall();
    void actionsCleanUp();
    void optionsShowToolbar();
    void optionsShowStatusbar();
    void optionsConfigureKeys();
    void optionsConfigureToolbars();
    void optionsPreferences();
    void newToolbarConfig();

private:
    void setupAccel();
    void setupActions();

    PakooView* m_view;
	QHBox* statusBarProgress;
	KProgress* statusBarProgressBar;
	KPushButton* statusBarProgressButton;
	QLabel* statusBarLabel;

    KToggleAction *m_toolbarAction;
    KToggleAction *m_statusbarAction;
};

#endif // PAKOO_H
