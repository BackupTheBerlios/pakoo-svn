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

#include "../core/portagepackageversion.h"


namespace libpakt {

/**
 * Initialize this object.
 */
FilePackageMaskLoader::FilePackageMaskLoader() : FileAtomLoaderBase()
{
	m_mask = true;
}

/**
 * Define if found packages should be masked or unmasked. Possible
 * values are therefore FileHardMaskLoader::Mask (which is the default)
 * or FileHardMaskLoader::Unmask.
 */
void FilePackageMaskLoader::setMode( FilePackageMaskLoader::Mode mode )
{
	if( mode == Mask )
		m_mask = true;
	else if( mode == Unmask )
		m_mask = false;
}

/**
 * Returns false for empty or comment lines.
 */
bool FilePackageMaskLoader::isLineProcessed( const QString& line )
{
	if( line.isEmpty() || line.startsWith("#") )
		return false;
	else
		return true;
}

/**
 * Set the atomString to the whole line, always returning true.
 */
bool FilePackageMaskLoader::setAtomString( const QString& line )
{
	m_atomString = line;
	return true;
}

/**
 * Set or unset (depending on the object's configuration)
 * the version's hardmasked flag.
 */
void FilePackageMaskLoader::processVersion( PortagePackageVersion* version )
{
	version->setHardMasked( m_mask );
}

} // namespace
