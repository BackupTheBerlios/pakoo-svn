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

#include "filemakeconfigloader.h"

#include <qfile.h>
#include <qtextstream.h>

#include "../core/portagesettings.h"


namespace libpakt {

/**
 * Initialize this object.
 */
FileMakeConfigLoader::FileMakeConfigLoader()
: FileLoaderBase(),
  rxConfigLine("(" RXSHELLVARIABLE ")=([^#]*)")
// RXSHELLVARIABLE is defined in portagesettings.h
{
	settings = NULL;
}

/**
 * Set the PortageSettings object that will be
 * filled with configuration values.
 */
void FileMakeConfigLoader::setSettingsObject( PortageSettings* settings )
{
	this->settings = settings;
}

/**
 * Check for a valid settings object before processing the file.
 */
bool FileMakeConfigLoader::check()
{
	// Check for an invalid settings object, which would be bad
	if( settings == NULL ) {
		emit debugOutput(
			QString( "Didn't start loading %1 because "
			         "the settings object has not been set" )
				.arg( filename )
		);
		return false;
	}
	else {
		return true;
	}
}

/**
 * Returns false for empty or comment lines.
 */
bool FileMakeConfigLoader::isLineProcessed( const QString& line )
{
	if( line.isEmpty() || line.stripWhiteSpace().startsWith("#") )
		return false;
	else
		return true;
}

/**
 * Process one single line of the file. If it contains a configuration value,
 * it will be stored in this object's 'settings' member.
 *
 * @param line  The current line that has been read from the file.
 */
void FileMakeConfigLoader::processLine( const QString& line )
{
	if( rxConfigLine.search(line) > -1 ) // found a configuration value
	{
		QString name = rxConfigLine.cap(1);

		QString value = rxConfigLine.cap( 2 ).stripWhiteSpace();
		if( value.startsWith("\"") && value.endsWith("\"") ) {
			value = value.mid( 1, value.length() - 2 );
		}

		// don't replace incremental variables, rather sum them up
		if( settings->isIncremental(name) == true )
		{
			settings->addToValue( name, value );
		}
		else {
			settings->setValue( name, value );
		}
	}
}

} // namespace
