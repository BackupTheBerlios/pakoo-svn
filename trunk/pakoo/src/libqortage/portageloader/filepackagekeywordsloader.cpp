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

#include "filepackagekeywordsloader.h"

#include "packageversion.h"


/**
 * Initialize this object.
 */
FilePackageKeywordsLoader::FilePackageKeywordsLoader()
: FileAtomLoaderBase()
{
}

/**
 * If the line is a comment then false is returned.
 * Otherwise the atomString is set to the whole line.
 */
bool FilePackageKeywordsLoader::processLine( const QString& line )
{
	if( line.startsWith("#") ) {
		return false;
	}
	else
	{
		QStringList tokens = QStringList::split( ' ', line );
		atomString = tokens[0];
		keywords.clear();

		QStringList::iterator tokenIterator = tokens.begin();
		tokenIterator++;
		while( tokenIterator != tokens.end() )
		{
			keywords.prepend( *tokenIterator );
			tokenIterator++;
		}
		if( keywords.empty() ) {
			keywords.prepend("~*");
			// in fact, it would be: keywords.prepend("~" + arch), but anyways
		}

		return true;
	}
}

/**
 * Set additional keywords for the found package version.
 */
void FilePackageKeywordsLoader::processVersion( PackageVersion* version )
{
	for( QStringList::iterator keywordIterator = keywords.begin();
	     keywordIterator != keywords.end(); keywordIterator++ )
	{
		version->acceptedKeywords.prepend( *keywordIterator );
	}
}
