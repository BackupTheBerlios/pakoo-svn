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

#include "../core/portagepackageversion.h"


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
 * Extract and set m_atomString as well as package keywords
 * from the given line. The package keywords are stored internally
 * so that they can be retrieved by processVersion afterwards.
 *
 * @return Always true
 */
bool FilePackageKeywordsLoader::setAtomString( const QString& line )
{
	// set the atom string
	QStringList tokens = QStringList::split( ' ', line );
	m_atomString = tokens[0];
	m_keywords.clear();

	// extract this line's keywords
	QStringList::iterator tokenIterator = tokens.begin();
	tokenIterator++;
	while( tokenIterator != tokens.end() )
	{
		m_keywords.prepend( *tokenIterator );
		tokenIterator++;
	}
	if( m_keywords.empty() ) {
		m_keywords.prepend("~*");
		// in fact, it would be: m_keywords.prepend("~" + arch), but anyways
	}
	return true;
}

/**
 * Set additional keywords for the found package version.
 */
void FilePackageKeywordsLoader::processVersion(
	PortagePackageVersion* version )
{
	for( QStringList::iterator keywordIterator = m_keywords.begin();
	     keywordIterator != m_keywords.end(); keywordIterator++ )
	{
		version->acceptedKeywords().prepend( *keywordIterator );
	}
}

} // namespace
