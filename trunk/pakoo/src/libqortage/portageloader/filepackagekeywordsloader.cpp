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


namespace libpakt {

/**
 * Initialize this object.
 */
FilePackageKeywordsLoader::FilePackageKeywordsLoader() : FileAtomLoaderBase()
{}

/**
 * Returns false for empty or comment lines.
 */
bool FilePackageKeywordsLoader::isLineProcessed( const QString& line )
{
	if( line.isEmpty() || line.startsWith("#") )
		return false;
	else
		return true;
}

/**
 * Set the atomString to the whole line, always returning true.
 */
bool FilePackageKeywordsLoader::setAtomString( const QString& line )
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

} // namespace
