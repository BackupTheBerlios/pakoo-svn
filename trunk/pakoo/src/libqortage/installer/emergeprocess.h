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

#ifndef LIBPAKTEMERGEPROCESS_H
#define LIBPAKTEMERGEPROCESS_H

#include "../core/processjob.h"


namespace libpakt {

/**
 * @short A ProcessJob to install or uninstall packages.
 *
 * This object can take a list of packages, along with possible options
 * for emerge, compose an appropriate shell command and parse the
 * emerge output.
 */
class EmergeProcess : public ProcessJob
{
	Q_OBJECT

public:
	EmergeProcess( QObject* parent = 0, const char* name = 0 );
	~EmergeProcess();

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

#endif // LIBPAKTEMERGEPROCESS_H
