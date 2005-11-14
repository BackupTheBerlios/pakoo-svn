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

#include "profileloader.h"

#include "filemakeconfigloader.h"
#include "../core/portagesettings.h"

#include <qstring.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>

#include <klocale.h>
#include <kdebug.h>


namespace libpakt {

/**
 * Initialize this object.
 */
ProfileLoader::ProfileLoader()
{
	m_settings = NULL;
}

/**
 * Set the PortageSettings object that will be
 * filled with configuration values.
 */
void ProfileLoader::setSettingsObject( PortageSettings* settings )
{
	m_settings = settings;
}

/**
 * Scan files and folders to retrieve the current profile settings.
 * The found values are applied to the given PortageSettings object.
 *
 * @return  IJob::Success if the configuration values have been read
 *          into the settings object. IJob::Failure if errors have occurred.
 */
IJob::JobResult ProfileLoader::performThread()
{
	if( m_settings == NULL ) {
		kdDebug() << i18n( "ProfileLoader debug output",
			"ProfileLoader::performThread(): "
			"Didn't start the because the settings object has not been set" )
			<< endl;
		return Failure;
	}

	// get the directory where we start reading
	QDir dir;
	if( goToStartDirectory(dir) == false ) {
		kdDebug() << i18n( "ProfileLoader debug output",
			"ProfileLoader::performThread(): Couldn't find the "
			"starting directory of the cascading profile" );
		return Failure;
	}

	FileMakeConfigLoader makeConfigLoader;
	makeConfigLoader.setSettingsObject( m_settings );

	// beginning from the start directory,
	// read all of the profile directories that the 'parent' files point to
	while( true )
	{
		if( dir.exists() == false ) // should not happen
		{
			kdDebug() << i18n( "ProfileLoader debug output",
				"ProfileLoader::performThread(): "
				"Directory %1 does not exist" )
					.arg( dir.path() )
				<< endl;
			return Failure;
		}

		// load settings from the make.defaults file
		makeConfigLoader.setFileName( dir.filePath("make.defaults") );
		makeConfigLoader.perform();

		// read other files (not implemented, don't need that for now)

		// check if there are more profile directories to load
		if( goToParentDirectory( dir ) == false )
			break; // no more parent profile directories
	}

	// get additional info from /etc/make.globals and /etc/make.conf
	makeConfigLoader.setFileName( "/etc/make.globals" );
	makeConfigLoader.perform();
	makeConfigLoader.setFileName( "/etc/make.conf" );
	makeConfigLoader.perform();

	kdDebug() << i18n( "ProfileLoader debug output",
		"ProfileLoader::performThread(): "
		"Loaded the cascading profile's configuration values" )
		<< endl;
	return Success;
}

/**
 * Navigates to the directory where the cascading profile starts.
 * This is the most specific one of the directories that the profile
 * consists of. (Less abstract: The folder is something like
 * /usr/portage/profiles/default-linux/x86/2005.0 or similar.)
 * If successful, the given QDir object now contains the path of the
 * starting directory and true is returned. Otherwise the path
 * isn't changed and the function returns false.
 * This function does not check if the resulting directory exists.
 */
bool ProfileLoader::goToStartDirectory( QDir& dir )
{
	QFileInfo fileInfo("/etc/make.profile");
	if( fileInfo.isSymLink() == true )
	{
		dir.setPath( fileInfo.readLink() );
		return true;
	}
	else {
		return false;
	}
}

/**
 * Navigate the given QDir object to the current directory's
 * parent profile directory. This is the one given in the
 * 'parent' file of the current directory.
 * If successful, the given QDir object now contains the new path
 * and true is returned. Otherwise the path isn't changed and
 * the function returns false.
 * This function does not check if the resulting directory exists.
 */
bool ProfileLoader::goToParentDirectory( QDir& currentDir )
{
	// Open the 'parent' file for reading
	QFile file( currentDir.filePath("parent") );

	if( !file.open( IO_ReadOnly ) ) {
		return false; // no such file
	}

	QString line;
	QTextStream stream( &file );

	// Process each line
	while ( !stream.atEnd() )
	{
		line = stream.readLine().stripWhiteSpace();

		if( line.isEmpty() == true || line.startsWith("#") )
			continue; // empty line or comment -> next one
		else {
			// found the relevant line!
			currentDir.setPath( currentDir.filePath(line) );
			return true;
		}
	}
	return false;
}

} // namespace
