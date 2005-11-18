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

#include "installeractionview.h"

#include "libpakt/portage/installer/emergeprocess.h"

#include <qlayout.h>
#include <qhbox.h>

#include <ktextedit.h>
#include <kpushbutton.h>

using namespace libpakt;


InstallerActionView::InstallerActionView( QWidget *parent, const char *name )
 : QWidget( parent, name )
{
	emergeProcess = new EmergeProcess( this );
	QVBoxLayout* layout = new QVBoxLayout( this );
	layout->setAutoAdd( true );

	m_log = new KTextEdit( this );
	m_log->setTextFormat( Qt::LogText );
	/* maybe use later for pretty printing:
	QStyleSheetItem * item =
		new QStyleSheetItem( m_log->styleSheet(), "mytag" );
	item->setColor( "red" );
	item->setFontWeight( QFont::Bold );
	item->setFontUnderline( TRUE );
	*/

	QHBox* hBox = new QHBox( this );
	KPushButton* startButton = new KPushButton("Start!", hBox);
	connect( startButton, SIGNAL( pressed() ),
	         emergeProcess, SLOT( start() ) );
	connect( emergeProcess, SIGNAL( receivedOutput(const QString&) ),
	         m_log,           SLOT( append(const QString&) ) );
}

InstallerActionView::~InstallerActionView()
{
}
