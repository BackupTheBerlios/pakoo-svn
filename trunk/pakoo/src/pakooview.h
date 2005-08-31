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


#ifndef PAKOOVIEW_H
#define PAKOOVIEW_H

#include <qwidget.h>
#include <qvaluelist.h>
#include <qmap.h>

#include <khtml_part.h>
#include <pakooiface.h>


namespace libpakt {

// our interface to libpakt
class BackendFactory;

// libpakt classes
class PackageList;

// widgets (will maybe go into libpakt too)
class PackageTreeView;
class PackageView;
class PackageInfoView;

} // end of libpakt declarations

class ActionView;
//TODO: remove and replace with real, working widgets
class QLabel;


/**
 * This is the main view class for Pakoo.  Most of the non-menu,
 * non-toolbar, and non-statusbar (e.g., non frame) GUI code goes here.
 *
 * @short Main view, containing the ListViews that display portage packages.
 */
class PakooView : public QWidget, public pakooIface
{
	Q_OBJECT
public:
	PakooView(QWidget *parent);

	void quit();

	QSize sizeHint() const;

public slots:
	void initData();
	void abortProgress();

signals:
	/** Use this signal to change the content of the window caption. */
	void windowCaptionChanged( const QString& text );
	/** Use this signal to change the content of the statusbar. */
	void statusbarTextChanged( const QString& text );
	/** Use this signal to set the progress of the main window's status bar.
	 * If it's hidden, it will be set visible. */
	void statusbarProgressChanged( int progress, int totalSteps );
	/** Use this signal to show the main window's status bar. */
	void statusbarProgressShown();
	/** Use this signal to hide the main window's status bar. */
	void statusbarProgressHidden();
	/** Use this signal to show the button next to the main window status bar. */
	void statusbarProgressButtonShown();
	/** Use this signal to hide the button next to the main window status bar. */
	void statusbarProgressButtonHidden();

protected:
	libpakt::PackageTreeView* m_treeView;
	libpakt::PackageView*     m_packageView;
	libpakt::PackageInfoView* m_packageInfoView;
	ActionView* m_actionView;
	QLabel* m_configView;
	QSplitter* m_hSplitter;
	QSplitter* m_vSplitter;

private slots:
	void showSection( int sectionIndex );

private:
	enum SectionType {
		BrowseSection,
		ActionSection,
		ConfigSection
	};

	void scanPortageTree();

	/** The factory creating all backend specific objects. */
	libpakt::BackendFactory* m_backend;

	/**
	 * The complete list of packages in the tree.
	 * It is filled in loadPortageTree().
	 */
	libpakt::PackageList* m_packages;

	QMap<int,SectionType> m_sectionIndexes;
};

#endif // PAKOOVIEW_H
