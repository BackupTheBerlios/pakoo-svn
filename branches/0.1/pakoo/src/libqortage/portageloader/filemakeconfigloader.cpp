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


/**
 * Initialize this object.
 */
FileMakeConfigLoader::FileMakeConfigLoader()
: rxConfigLine("(" RXSHELLVARIABLE ")=([^#]*)")
// RXSHELLVARIABLE is defined in portagesettings.h
{
}

/**
 * Scan and process a configuration file, and store the retrieved
 * configuration values into the PortageSettings object.
 *
 * @param settings  The PortageSettings object that will be filled
 *                  with configuration values.
 * @param filename  Path of the configuration file.
 *
 * @return  PortageLoaderBase::NoError if the file has successfully been handled.
 *          PortageLoaderBase::OpenFileError if there was an error opening the file.
 *          PortageLoaderBase::NullObjectError if the given settings object is NULL.
 */
PortageLoaderBase::Error FileMakeConfigLoader::loadFile(
	PortageSettings* settings, const QString& filename )
{
	// Check on a NULL settings object, which would be bad
	if( settings == NULL )
		return NullObjectError;
	else
		this->settings = settings;

	// Open the file for reading
	QFile file( filename );

	if( !file.open( IO_ReadOnly ) ) {
		return OpenFileError;
	}

	QString line;
	QTextStream stream( &file );

	// Process each line
	while ( !stream.atEnd() )
	{
		line = stream.readLine();

		if( line.isEmpty() == true )
			continue;

		processLine( line );
	}

	return NoError;
}

/**
 * Process one single line of the file.
 * If it contains a configuration value, it will be stored
 * in this object's 'settings' member.
 * Returns false if the line is a comment, true otherwise.
 */
bool FileMakeConfigLoader::processLine( const QString& line )
{
	if( line.stripWhiteSpace().startsWith("#") ) {
		return false;
	}

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
	return true;
}
