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

#include "../core/portagesettings.h"

#include <qfile.h>
#include <qtextstream.h>

#include <kdebug.h>
#include <klocale.h>


namespace libpakt {

/**
 * Initialize this object.
 */
FileMakeConfigLoader::FileMakeConfigLoader()
: FileLoaderBase(),
	m_rxConfigLine("(" RXSHELLVARIABLE ")=([^#]*)")
// RXSHELLVARIABLE is defined in portagesettings.h
{
	m_settings = NULL;
}

/**
 * Set the PortageSettings object that will be
 * filled with configuration values.
 */
void FileMakeConfigLoader::setSettingsObject( PortageSettings* settings )
{
	m_settings = settings;
}

/**
 * Check for a valid settings object before processing the file.
 */
bool FileMakeConfigLoader::check()
{
	// Check for an invalid settings object, which would be bad
	if( m_settings == NULL ) {
		kdDebug() << i18n( "FileMakeConfigLoader debug output. "
		                   "%1 is the file that was about to load.",
			"FileMakeConfigLoader::check(): Didn't start loading %1 because "
			"the settings object has not been set" )
				.arg( fileName() )
			<< endl;
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
 * it will be stored in this object's 'm_settings' member.
 *
 * @param line  The current line that has been read from the file.
 */
void FileMakeConfigLoader::processLine( const QString& line )
{
	if( m_rxConfigLine.search(line) > -1 ) // found a configuration value
	{
		QString name = m_rxConfigLine.cap(1);

		QString value = m_rxConfigLine.cap( 2 ).stripWhiteSpace();
		if( value.startsWith("\"") && value.endsWith("\"") ) {
			value = value.mid( 1, value.length() - 2 );
		}

		// don't replace incremental variables, rather sum them up
		if( m_settings->isIncremental(name) == true )
		{
			m_settings->addToValue( name, value );
		}
		else {
			m_settings->setValue( name, value );
		}
	}
}

} // namespace
