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

#include "portagesettings.h"

#include <qregexp.h>


namespace libpakt {

/**
 * Initialize this object.
 */
PortageSettings::PortageSettings()
{
}

/**
 * Remove all configuration values from this object.
 */
void PortageSettings::clear()
{
	m_configValues.clear();
}

/**
 * Returns true if the property with the given name is an incremental variable,
 * which means that if it is defined in multiple files then it sums up instead
 * of being replaced every time. If it's not an incremental variable,
 * the function returns false.
 *
 * Note: This function should be used to determine whether to call setValue()
 * (for non-incremental variables) or addToValue() (for incremental ones)
 * when filling this object with configuration values.
 */
bool PortageSettings::isIncremental( const QString& name )
{
	if( name == "USE" || name == "ACCEPT_KEYWORDS"
	    || name.startsWith("CONFIG_PROTECT") || name == "FEATURES" )
	{
		return true;
	}
	else {
		return false;
	}
}

/**
 * Retrieve a configuration property.
 * If it doesn't exist then QString::null is returned.
 */
QString PortageSettings::value( const QString& name )
{
	if( m_configValues.contains(name) ) {
		return substituteShellVariables( m_configValues[name] );
	}
	else
		return QString::null;
}

/**
 * Set a configuration property to a given value. If a value with this name
 * already exists, it will be overwritten. If not, the value will be created.
 */
void PortageSettings::setValue( const QString& name, const QString& value )
{
	m_configValues[name] = value;
}

/**
 * Create or modify a property. This function should be used when setting
 * incremental variables, and it handles important stuff like removing values
 * when they have a "-" prepended.
 */
void PortageSettings::addToValue( const QString& name, const QString& value )
{
	if( m_configValues.contains(name) == false )
	{
		m_configValues[name] = value;
	}
	else // the value already exists
	{
		QStringList addedValues = QStringList::split( ' ', value );
		QStringList values = QStringList::split( ' ', m_configValues[name] );

		// go through all new entries
		QStringList::iterator addedValueIteratorEnd = addedValues.end();
		for( QStringList::iterator addedValueIterator = addedValues.begin();
		     addedValueIterator != addedValueIteratorEnd;
		     ++addedValueIterator )
		{
			//
			// Portage has a special handling for adding and removing values.
			// When a higher-priority file (to be read after the low-priority
			// one) prepends "-" to the value, it is deleted no matter if
			// it was there before or not. Values without that hyphen are
			// simply added to the list.
			//
			if( (*addedValueIterator).startsWith("-") ) // to be removed
			{
				if( (*addedValueIterator) == "-*" ) { // remove all
					values.clear();
					continue;
				}

				QStringList::iterator removedValueIterator =
					values.find( (*addedValueIterator).mid(1) );

				if( removedValueIterator != values.end() )
				{
					// the value exists, remove it
					values.remove( removedValueIterator );
				}
				// if the value doesn't exist, it doesn't have to be removed
			}
			else // doesn't start with "-", so it's a value to be added
			{
				if( values.contains(*addedValueIterator) == false )
					values.append( *addedValueIterator );
				// if it's already in there, it doesn't have to be added
			}
		}

		// Ok, now we have a fine updated list of values.
		// Let's bring them back to QString format.
		m_configValues[name] = values.join(" ");
	}
}

/**
 * Return the list of configuration values.
 */
PortageSettings::ConfigValueMap* PortageSettings::configValueMap()
{
	return &(this->m_configValues);
}

/**
 * Perform shell variable substitution for configuration values.
 * I don't like doing that manually, but I don't see other possibilities
 * to do so.
 */
QString PortageSettings::substituteShellVariables( const QString& value )
{
	QString result = value;
	QRegExp rxSubstitution(
		"\\$((?:\\{" RXSHELLVARIABLE "\\})|(?:" RXSHELLVARIABLE "))");
	rxSubstitution.setMinimal( true );

	int pos = 0;
	while( true )
	{
		pos = rxSubstitution.search( result, pos );
		if( pos == -1 )
			break; // the string doesn't contain any substitutions (anymore)

		QString varName = rxSubstitution.cap(1);
		if( varName.startsWith("{") )
		{ // remove superfluous parentheses, if needed
			varName = varName.mid( 1, varName.length() - 2 );
		}

		if( m_configValues.contains(varName) )
		{
			result = result.replace( pos, rxSubstitution.matchedLength(),
			                         m_configValues[varName] );
		}
		else // can't replace - skip forward to get rid of endless loops
		{
			pos += rxSubstitution.matchedLength();
			continue; // either $VARNAME or ${VARNAME}, but not $VARNAME}
		}
	}

	return result;
}

/**
 * Convenience function to retrieve the directory string of the mainline
 * Portage tree. If there is no appropriate value, the function returns
 * the Gentoo default.
 */
QString PortageSettings::mainlineTreeDirectory()
{
	QString directory = value("PORTDIR");
	if( directory == QString::null )
		return "/usr/portage";
	else
		return directory;
}

/**
 * Convenience function to retrieve the the list of directory strings of
 * the overlay trees. If there is no appropriate value, the function returns
 * an empty list.
 */
QStringList PortageSettings::overlayTreeDirectories()
{
	QString dirs = value("PORTDIR_OVERLAY");
	if( dirs != QString::null ) {
		return QStringList::split( " ", dirs );
	}
	else {
		return QStringList();
	}
}

/**
 * Convenience function to retrieve the the list of accepted keywords.
 * If there is no appropriate value, the function returns an QString::null.
 */
QString PortageSettings::acceptedKeyword()
{
	if( m_configValues.contains("ACCEPT_KEYWORDS") )
	{
		QStringList values = QStringList::split(" ", value("ACCEPT_KEYWORDS"));
		if( values.count() == 1 ) // only one element, likely not to be masked
			return values[0];

		// this function does only return one item,
		// so in case of multiple ones the masked one is preferred
		QStringList::iterator valueIteratorEnd = values.end();
		for( QStringList::iterator valueIterator = values.begin();
		     valueIterator != valueIteratorEnd; ++valueIterator )
		{
			if( (*valueIterator).startsWith("~") )
				return *valueIterator;
		}
		// if no masked one has been found
		// (which should not happen in a proper configuration):
		return values[0];
	}
	else {
		return QString::null;
	}
}

/**
 * Set the directory where the installed packages reside.
 * This is not a Portage setting and must therefore be set specifically.
 */
void PortageSettings::setInstalledPackagesDirectory(
	const QString& directory )
{
	setValue( "libpakt:installedPackagesDir", directory );
}

/**
 * Return the directory where the installed packages reside.
 * This is not a Portage setting and must therefore be set specifically.
 * If there is no appropriate value, the function returns the Gentoo default.
 */
QString PortageSettings::installedPackagesDirectory()
{
	QString directory = value("libpakt:installedPackagesDir");
	if( directory == QString::null )
		return "/var/db/pkg";
	else
		return directory;
}

/**
 * Set the directory where the Portage cache resides.
 * This is not a Portage setting and must therefore be set specifically.
 */
void PortageSettings::setCacheDirectory(
	const QString& directory )
{
	setValue( "libpakt:cacheDir", directory );
}

/**
 * Set the directory where the Portage cache resides.
 * This is not a Portage setting and must therefore be set specifically.
 * If there is no appropriate value, the function returns the Gentoo default.
 */
QString PortageSettings::cacheDirectory()
{
	QString directory = value("libpakt:cacheDir");
	if( directory == QString::null )
		return "/var/cache/edb/dep";
	else
		return directory;
}

/**
 * Define if/what Portage cache should be used. This should lead to
 * faster reading of tree structure and package info, but can lead to
 * wrong results if the cache isn't available (there are packages
 * in Portage designed to replace the cache) or if it's outdated
 * (which can be fixed by 'emerge metadata' and shouldn't normally happen).
 *
 * This is not a Portage setting and must therefore be set specifically.
 */
void PortageSettings::setPreferredPackageSource( PackageSource packageSource )
{
	if( packageSource == PortageTree )
		setValue( "libpakt:preferredPackageSource", "PortageTree" );
	else if( packageSource == CdbCache )
		setValue( "libpakt:preferredPackageSource", "CdbCache" );
	else
		m_configValues.remove( "libpakt:preferredPackageSource" );
}

/**
 * Determine if/what Portage cache should be used.
 * This is not a Portage setting and must therefore be set specifically.
 * If there is no appropriate value, the function returns false.
 */
PackageSource PortageSettings::preferredPackageSource()
{
	if( m_configValues.contains("libpakt:preferredPackageSource") ) {
		QString packageSource = value("libpakt:preferredPackageSource");
		if( packageSource == "PortageTree" )
			return PortageTree;
		else
			return CdbCache;
	}
	else
		return FlatCache;
}

} // namespace
