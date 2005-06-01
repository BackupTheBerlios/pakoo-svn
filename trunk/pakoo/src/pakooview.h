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

#include <khtml_part.h>
#include <pakooiface.h>

class PortageTreeView;
class PackageView;
class PackageInfoView;

/* replace the member objects by pointers, then you can do:
class PortageTreeScanner;
class PackageScanner;
class PortageML;
instead of: */
#include "libqortage/libqortage.h"

class LoadingTreeCompleteEvent;


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

	void loadPortageTree();
	void customEvent( QCustomEvent *event );
	void quit();

	QSize sizeHint() const;

public slots:
	void initData();
	void setStatusbarText( const QString& text );
	void setTitle( const QString& title );
	void setStatusbarProgress( int progress, int totalSteps );
	void hideStatusbarProgress( bool hide = true );

signals:
	//! Use this signal to change the content of the statusbar.
	void signalSetStatusbarText(const QString& text);
	//! Use this signal to change the content of the caption.
	void signalSetCaption(const QString& text);
	/**
	 * Use this signal to set the progress of the main window's status bar.
	 * If it's hidden, it will be set visible.
	 */
	void signalSetStatusbarProgress( int progress, int totalSteps );
	//! Use this signal to hide or show the main window's status bar.
	void signalHideStatusbarProgress( bool hide );

protected:

	PortageTreeView* treeView;
	PackageView* packageView;
	PackageInfoView* packageInfoView;
	QSplitter* vSplitter;
	QSplitter* hSplitter;

	PortageTreeScanner portageTreeScanner;
	PortageML portageML;
	PortageTree portageTree;

private slots:
	void handleLoadingPackageInfo( int loadedPackageCount, int totalPackageCount );
	void handleFinishedLoadingPackageInfo( int totalPackageCount );

private:
	void handleLoadingTreeComplete( LoadingTreeCompleteEvent* event );

	QString message; // temp variable to store message texts
	int packageCount;
	int mainlineCount;
	int overlayCount;
	int installedPackageCount;
};

#endif // PAKOOVIEW_H
