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

#include "../core/portagetree.h"
#include "../core/dependatom.h"

#include <qfile.h>
#include <qtextstream.h>


/**
 * Initialize this object.
 */
FileAtomLoaderBase::FileAtomLoaderBase()
: PortageLoaderBase()
{
}

/**
 * Scan and process the file, which most probably means modifying
 * some properties of packages inside the given tree. For each line
 * of the given file, processLine() is called. processLine() extracts
 * the atom string from the line so that matching versions of this
 * DEPEND atom can be retrieved. Then processVersion() is called
 * for each version matching the DEPEND atom.
 *
 * @param portageTree  The PortageTree object whose packages will be modified.
 * @param filename     The file that contains masking information.
 *
 * @return  PortageLoaderBase::NoError if the file has successfully be handled.
 *          PortageLoaderBase::OpenFileError if there was an error opening the file.
 *          PortageLoaderBase::NullTreeError if the given tree is NULL.
 */
PortageLoaderBase::Error FileAtomLoaderBase::loadFile(
	PortageTree* portageTree, const QString& filename )
{
	// Check on a NULL tree, which would be bad
	if( portageTree == NULL ) {
		return NullTreeError;
	}
	tree = portageTree;

	// Open the file for reading
	QFile file( filename );

	if( !file.open( IO_ReadOnly ) ) {
		return OpenFileError;
	}

	QString line;
	QTextStream stream( &file );

	atom = new DependAtom( tree );

	// Process each line
	while ( !stream.atEnd() )
	{
		line = stream.readLine();

		if( line.isEmpty() == true )
			continue;

		// call preprocess(), to set atomString (if it doesn't return false)
		if( processLine(line) == false )
			continue;

		if( atom->parse(atomString) == false )
			continue;

		// get the matching versions, and call process() on each of them
		QValueList<PackageVersion*> versions = atom->matchingVersions();
		QValueList<PackageVersion*>::iterator versionIterator;

		for( versionIterator = versions.begin();
		     versionIterator != versions.end(); versionIterator++ )
		{
			processVersion( *versionIterator );
		}
	}

	delete atom;

	return NoError;
}
