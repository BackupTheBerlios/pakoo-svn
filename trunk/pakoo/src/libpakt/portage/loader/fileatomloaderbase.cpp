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

#include "fileatomloaderbase.h"

#include "../../base/core/packagelist.h"
#include "../core/dependatom.h"

#include <qfile.h>
#include <qtextstream.h>

#include <kdebug.h>
#include <klocale.h>


namespace libpakt {

/**
 * Initialize this object.
 */
FileAtomLoaderBase::FileAtomLoaderBase() : FileLoaderBase()
{
	m_packages = NULL;
}


/**
 * Set the PackageList object that will be filled with packages
 * (in case of loading from a file) or used as source of
 * package information (in case of saving to a file).
 */
void FileAtomLoaderBase::setPackageList(
	TemplatedPackageList<PortagePackage>* packages )
{
	m_packages = packages;
}

/**
 * Check for a valid package list before processing the file.
 */
bool FileAtomLoaderBase::check()
{
	// Check for an invalid list, which would be bad
	if( m_packages == NULL ) {
		kdDebug() << i18n( "FileAtomLoaderBase debug output. "
		                   "%1 is the file that was about to load.",
			"FileAtomLoaderBase::check(): Didn't start loading %1 because "
			"the PackageList object has not been set" )
				.arg( fileName() )
			<< endl;
		return false;
	}
	else {
		return true;
	}
}

/**
 * Initialize the 'atom' member variable.
 */
bool FileAtomLoaderBase::init()
{
	m_atom = new DependAtom( m_packages );
	return true;
}

/**
 * Destroy the 'atom' member variable and return Success.
 */
IJob::JobResult FileAtomLoaderBase::finish()
{
	delete m_atom;
	return Success;
}

/**
 * Scan and process the file, which most probably means modifying
 * some properties of packages inside the package list. For each line
 * of the given file, setAtomString() is called. setAtomString() extracts
 * the atom string from the line so that matching versions of this
 * DEPEND atom can be retrieved. Then processVersion() is called
 * for each version matching the DEPEND atom.
 */
void FileAtomLoaderBase::processLine( const QString& line )
{
	// call preprocess(), to set atomString (if it doesn't return false)
	if( setAtomString(line) == false )
		return;

	if( m_atom->parse(m_atomString) == false )
		return;

	// get the matching versions, and call process() on each of them
	QValueList<PortagePackageVersion*> versions = m_atom->matchingVersions();
	QValueList<PortagePackageVersion*>::iterator versionIterator;

	for( versionIterator = versions.begin();
			versionIterator != versions.end(); versionIterator++ )
	{
		processVersion( *versionIterator );
	}
}

} // namespace
