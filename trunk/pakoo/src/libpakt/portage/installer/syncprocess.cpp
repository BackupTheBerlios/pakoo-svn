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

#include "syncprocess.h"

#include <qstringlist.h>

namespace libpakt {

/**
 * Initialize this object.
 * @see QObject::QObject(QObject*, const char*)
 */
SyncProcess::SyncProcess( QObject* parent, const char* name )
	: ProcessJob( parent, name )
{
}

/**
 * Destruct this object.
 */
SyncProcess::~SyncProcess()
{
}

ProcessJob::ProcessType SyncProcess::processType()
{
	return LocalProcess;
}

QStringList SyncProcess::processNameAndArguments( )
{
	QStringList list;
	//FIXME: Big problems: How should the user enter the password?
	//list << "su" << "-c" << "emerge --sync";
	//NOTE: Syncing is a hard one to debug:
	//      * The portage tree is only updated two times per hour.
	//      * You may be blocked for abuse...
	//FIXME: Until the above fixme is fixed:
	list << "emerge" << "--sync";
	return list;
}

} // namespace
