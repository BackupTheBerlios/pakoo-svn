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

#ifndef LIBPAKTPORTAGESETTINGS_H
#define LIBPAKTPORTAGESETTINGS_H

#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>

// Regexp for shell variables, from 'info bash':
// A `word' consisting solely of letters, numbers, and underscores,
// and beginning with a letter or underscore.
#define RXSHELLVARIABLE "(?:[a-z]|[A-Z]|_)+(?:[a-z]|[A-Z]|[0-9]|_)*"


namespace libpakt {

/**
 * This class holds configuration settings.
 * Most of the global Portage options are stored in here.
 * All configuration values can be modified using setValue() or addToValue()
 * and retrieved using value(). For some of the configuration values,
 * there are extra convenience functions.
 */
class PortageSettings
{
public:
	typedef QMap<QString,QString> ConfigValueMap;

	PortageSettings();

	bool isIncremental( const QString& name );

	QString value( const QString& name );
	void setValue( const QString& name, const QString& value );
	void addToValue( const QString& name, const QString& value );

	void clear();

	ConfigValueMap* configValueMap();

	// convenience functions
	QString mainlineTreeDirectory();
	QStringList overlayTreeDirectories();
	QString acceptedKeyword();

	// not in the Portage settings but good to have in this object
	void setInstalledPackagesDirectory( const QString& directory );
	QString installedPackagesDirectory();
	void setCacheDirectory( const QString& directory );
	QString cacheDirectory();
	void setPreferCache( bool preferCache );
	bool preferCache();

protected:
	QString substituteShellVariables( const QString& value );

private:
	ConfigValueMap m_configValues;
};

}

#endif // LIBPAKTPORTAGESETTINGS_H
