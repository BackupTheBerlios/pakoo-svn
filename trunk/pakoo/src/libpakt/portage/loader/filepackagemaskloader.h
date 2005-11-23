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

#ifndef LIBPAKTFILEPACKAGEMASKLOADER_H
#define LIBPAKTFILEPACKAGEMASKLOADER_H

#include "../loader/fileatomloaderbase.h"


namespace libpakt {

/**
 * This is a class that is able to load hardmasking files, such as
 * package.mask or package.unmask from /etc/portage/.
 * Starting the loader (using start() or perform()) sets or unsets
 * the hardmasked flag for each package version contained in this file.
 * By calling setMode(), you can decide if the found packages will be
 * masked or unmasked.
 */
class FilePackageMaskLoader : public FileAtomLoaderBase
{
	Q_OBJECT

public:
	enum Mode {
		Mask,
		Unmask
	};

	FilePackageMaskLoader();
	void setMode( FilePackageMaskLoader::Mode mode );

protected:
	bool isLineProcessed( const QString& line );
	bool setAtomString( const QString& line );
	void processVersion( PortagePackageVersion* version );

private:
	bool m_mask;
};

}

#endif // LIBPAKTFILEPACKAGEMASKLOADER_H
