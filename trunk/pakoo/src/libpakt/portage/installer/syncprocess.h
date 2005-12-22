/***************************************************************************
 *   Copyright (C) 2005 by Arvid Norlander                                 *
 *   anmaster@users.berlios.de                                             *
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

#ifndef LIBPAKTSYNCPROCESS_H
#define LIBPAKTSYNCPROCESS_H

#include "../../base/core/processjob.h"


namespace libpakt {

/**
 * @short A ProcessJob to sync the portage tree.
 *
 * This object can run emerge --sync or eix-sync, compose 
 * an appropriate shell command and parse the emerge output.
 */
class SyncProcess : public ProcessJob
{
	Q_OBJECT

public:
	SyncProcess( QObject* parent = 0, const char* name = 0 );
	~SyncProcess();

	bool statusMessagesEnabled() { return false; }

signals:
	/**
	 * Emitted when output from the child process has been received,
	 * like seen on terminal output. This is a combination of stdout
	 * and stderr.
	 *
	 * @param output  The data that has been received.
	 */
	//void receivedOutput( const QString& output );

protected:
	ProcessType processType();
	QStringList processNameAndArguments();
};


}

#endif // LIBPAKTSYNCPROCESS_H
