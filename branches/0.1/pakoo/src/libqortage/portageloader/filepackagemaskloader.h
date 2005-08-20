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

#ifndef FILEPACKAGEMASKLOADER_H
#define FILEPACKAGEMASKLOADER_H

#include "fileatomloaderbase.h"

/**
 * This is a class that is able to load hardmasking files, such as
 * package.mask or package.unmask from /etc/portage/.
 * Calling loadFile() sets or unsets the hardmasked flag for each
 * package version contained in this file. By calling setMode(),
 * you can decide if the found packages will be masked or unmasked.
 */
class FilePackageMaskLoader : public FileAtomLoaderBase
{
public:
	enum Mode {
		Mask,
		Unmask
	};

	FilePackageMaskLoader();
	void setMode( FilePackageMaskLoader::Mode mode );

protected:
	bool processLine( const QString& line );
    void processVersion( PackageVersion* version );

private:
	bool mask;
};

#endif
