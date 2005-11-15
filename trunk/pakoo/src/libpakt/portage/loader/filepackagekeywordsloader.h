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

#ifndef LIBPAKTFILEPACKAGEKEYWORDSLOADER_H
#define LIBPAKTFILEPACKAGEKEYWORDSLOADER_H

#include "../../base/loader/fileatomloaderbase.h"

#include <qstringlist.h>


namespace libpakt {

/**
 * Using this class, you're able to load additional keywords from
 * package.keywords files (from what I know there's only one,
 * which is residing in /etc/portage/package.keywords).
 * Just call start() or perform() and enjoy correct keywording.
 */
class FilePackageKeywordsLoader : public FileAtomLoaderBase
{
	Q_OBJECT

public:
    FilePackageKeywordsLoader();

private:
	bool isLineProcessed( const QString& line );
	bool setAtomString( const QString& line );
	void processVersion( PortagePackageVersion* version );

	QStringList m_keywords;
};

}

#endif // LIBPAKTFILEPACKAGEKEYWORDSLOADER_H
