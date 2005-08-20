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

#ifndef LIBPAKTFILEMAKECONFIGLOADER_H
#define LIBPAKTFILEMAKECONFIGLOADER_H

#include "../core/fileloaderbase.h"

#include <qregexp.h>


namespace libpakt {

class PortageSettings;

/**
 * This is a class that is able to load files like /etc/make.conf,
 * /etc/make.globals or the make.defaults files in each profile directory.
 * Calling loadFile() retrieves the configuration values and stores them
 * into a given PortageSettings object.
 */
class FileMakeConfigLoader : public FileLoaderBase
{
	Q_OBJECT

public:
	FileMakeConfigLoader();

	void setSettingsObject( PortageSettings* settings );

private:
	bool check();
	bool isLineProcessed( const QString& line );
	void processLine( const QString& line );

	//! The PortageSettings object that will be filled with configuration values.
	PortageSettings* settings;

	// Regexp for a configuration line, like ARCH="x86" or SUPPORT_ALSA=1
	QRegExp rxConfigLine;
};

}

#endif // LIBPAKTFILEMAKECONFIGLOADER_H
