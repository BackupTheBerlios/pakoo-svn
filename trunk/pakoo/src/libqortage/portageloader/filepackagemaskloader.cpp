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

#include "filepackagemaskloader.h"

#include "../core/packageversion.h"


/**
 * Initialize this object.
 */
FilePackageMaskLoader::FilePackageMaskLoader()
 : FileAtomLoaderBase()
{
	mask = true;
}

/**
 * Define if found packages should be masked or unmasked. Possible
 * values are therefore FileHardMaskLoader::Mask (which is the default)
 * or FileHardMaskLoader::Unmask.
 */
void FilePackageMaskLoader::setMode( FilePackageMaskLoader::Mode mode )
{
	if( mode == Mask )
		mask = true;
	else if( mode == Unmask )
		mask = false;
}

/**
 * If the line is a comment then false is returned.
 * Otherwise the atomString is set to the whole line.
 */
bool FilePackageMaskLoader::processLine( const QString& line )
{
	if( line.startsWith("#") ) {
		return false;
	}
	else {
		atomString = line;
		return true;
	}
}

/**
 * Set or unset (depending on the object's configuration)
 * the version's hardmasked flag.
 */
void FilePackageMaskLoader::processVersion( PackageVersion* version )
{
	version->isHardMasked = mask;
}
