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

#include "fileloaderbase.h"

#include "../core/packagelist.h"
#include "../core/dependatom.h"

#include <qfile.h>
#include <qtextstream.h>

#include <kdebug.h>
#include <klocale.h>


namespace libpakt {

/**
 * Initialize this object.
 */
FileLoaderBase::FileLoaderBase()
{
	m_filename = QString::null;
}


/**
 * Set the name of the file that is used for loading and saving
 * the package list.
 */
void FileLoaderBase::setFileName( const QString& filename )
{
	m_filename = filename;
}

/**
 * Called by perform() every time a line has been read and before
 * processLine() is called.
 *
 * If this function returns false, processLine() will not be called
 * and perform() goes on with the next line in the file.
 * It can be overloaded in order to skip comment lines. By default, this
 * function returns false for empty lines and true for all other ones.
 */
bool FileLoaderBase::isLineProcessed( const QString& line )
{
	if( line.isEmpty() )
		return false;
	else
		return true;
}

/**
 * Scan and process the file. This function calls several virtual functions
 * which can be overloaded by derived classes.
 */
IJob::JobResult FileLoaderBase::performThread()
{
	// Check if the file has been defined
	if( m_filename.isNull() ) {
		kdDebug() << i18n( "FileLoaderBase debug output.",
			"FileLoaderBase::performThread(): Didn't start loading because "
			"the file name has not been set")
			<< endl;
		return Failure;
	}

	if( check() == false )
		return Failure;

	// Open the file for reading
	QFile file( m_filename );

	if( !file.open( IO_ReadOnly ) ) {
		kdDebug() << i18n( "FileLoaderBase debug output. "
		                   "%1 is the file that was about to load",
			"FileLoaderBase::performThread(): Couldn't open %1 for reading" )
				.arg( m_filename )
		<< endl;
		return Failure;
	}

	QString line;
	QTextStream stream( &file );

	if( init() == false )
		return Failure;

	// Process each line
	while ( !stream.atEnd() )
	{
		line = stream.readLine();

		if( isLineProcessed(line) == false )
			continue;

		processLine(line);

		if( aborting() == true )
			break;
	}

	file.close();

	return finish();
}

} // namespace
