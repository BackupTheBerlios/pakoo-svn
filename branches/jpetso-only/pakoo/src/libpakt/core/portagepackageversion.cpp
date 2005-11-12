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

#include "portagepackageversion.h"

namespace libpakt {

/**
 * Initialize the version with its version string.
 * Protected so that only PortagePackage can construct
 * a PortagePackageVersion object.
 */
PortagePackageVersion::PortagePackageVersion( Package* parent,
                                              const QString& version )
	: PackageVersion( parent, version ),
	// Regexp for a simple number, for use as a version number part
	rxNumber("\\d+"),
	// Regexp for a revision number, which are everywhere
	rxRevision("-r(\\d+)$"),
	// Regexp for a version suffix, like in freetts-1.2_beta
	rxSuffix("_(alpha|beta|pre|rc|p)(\\d?)(?:-r\\d+)?$"),
	// Regexp for a trailing character, like in util-linux-2.12i
	rxTrailingChar("\\d([a-z])(?:_(?:alpha|beta|pre|rc|p)\\d?)?(?:-r\\d+)?$")
{
	m_installed = false;
	m_overlay = false;
	/* This is what QString does automatically:
	m_date = "";
	m_description = "";
	m_homepage = "";
	m_slot = "";*/
	m_hasDetailedInfo = false;
	m_size = 0;
	m_isHardMasked = false;
}

/**
 * Returns true if this version is available (as in: can be installed)
 * and false if not.
 */
bool PortagePackageVersion::isAvailable() const
{
	//TODO: remove x86 and get arch it from the PortageSettings
	return (stability("x86") == Stable);
}

/**
 * Find out if this version has a higher version number than another one.
 *
 * @param otherVersion  Version string of the version that should be compared
 *                      to this one.
 * @return  true if this version is newer than the one given in the argument.
 *          false if the other version is newer (or if the strings are equal).
 */
bool PortagePackageVersion::isNewerThan( const QString& otherVersion ) const
{
	// this* is for this->version, that* is for otherVersion
	QString thisNum, thatNum;

	// Numerical representation of the suffix.
	// Higher number means higher precedence.
	long thisSuffix, thatSuffix;

	// Revision number. 0 means the version doesn't have a revision.
	int thisRevision, thatRevision;
	// Numerical representation for possible trailing characters.
	int thisTrailingChar, thatTrailingChar;

	uint pos; // multi-purpose position integer

	// start index of revision, suffix, and trailing character
	int revisionPos, suffixPos, trailingCharPos;

	// Retrieve revision, suffix and their positions in the version string
	thisRevision = PortagePackageVersion::revisionNumber( version(),
	                                                      &revisionPos );
	thisSuffix = PortagePackageVersion::suffixNumber( version(), &suffixPos );
	thisTrailingChar = PortagePackageVersion::trailingCharNumber(
		version(), &trailingCharPos );

	// determine the first non-base-version character
	if( trailingCharPos != -1 )
		pos = trailingCharPos;
	else if( suffixPos != -1 )
		pos = suffixPos;
	else if( revisionPos != -1 )
		pos = revisionPos;
	else
		pos = version().length();

	// So, now we have a version string stripped of suffix and revision
	QString thisBaseVersion( version().left(pos) );

	// Same procedure for the other version string
	thatRevision = PortagePackageVersion::revisionNumber( otherVersion,
	                                                      &revisionPos );
	thatSuffix = PortagePackageVersion::suffixNumber( otherVersion,
	                                                  &suffixPos );
	thatTrailingChar = PortagePackageVersion::trailingCharNumber(
		otherVersion, &trailingCharPos );

	// determine the first non-base-version character
	if( trailingCharPos != -1 )
		pos = trailingCharPos;
	else if( suffixPos != -1 )
		pos = suffixPos;
	else if( revisionPos != -1 )
		pos = revisionPos;
	else
		pos = version().length();

	// So, now we have a version string stripped of suffix and revision
	QString thatBaseVersion( otherVersion.left(pos) );


	// Ok, let's compare the remaining version strings.
	// This is done by step-by-step comparing the number parts
	// of this->version and the otherVersion.
	// For example: in "1.2.3" vs. "1.3.4", first "1" is compared with "1",
	// then "2" is compared with "3", and the third part isn't considered.

	pos = 0; // pos is the start index for number searches
	do
	{
		rxNumber.search( version(), pos );
		thisNum = (const_cast<PortagePackageVersion*>(this))->rxNumber.cap(0);
		rxNumber.search( otherVersion, pos );
		thatNum = (const_cast<PortagePackageVersion*>(this))->rxNumber.cap(0);

		if( thisNum.isEmpty() || thatNum.isEmpty() )
			return false; // should not happen, just to make sure

		if( thisNum != thatNum ) // the current version number parts differ
		{
		if( thisNum.toInt() > thatNum.toInt() ) // this->version is newer:
			return true;
			else // well, the otherVersion is newer.
				return false;
		}
		else // thisNum == thatNum
		{
			pos += thisNum.length();

			// check if any base-version string ends here
			if( thisBaseVersion.length() <= pos
			    && thatBaseVersion.length() <= pos ) // both versions end here
			{
				// compare trailing characters, if they are the same,
				// then suffixes, if they are also the same, then revisions
				if( thisTrailingChar > thatTrailingChar )
					return true;
				else if( thisTrailingChar < thatTrailingChar )
					return false;
				else if( thisTrailingChar == thatTrailingChar )
				{
					if( thisSuffix > thatSuffix )
						return true;
					else if( thisSuffix < thatSuffix )
						return false;
					else if( thisSuffix == thatSuffix )
					{
						if( thisRevision > thatRevision )
							return true;
						else // if( thisRevision <= thatRevision )
							return false;
					}
				}
			}
			else if( thisBaseVersion.length() <= pos ) { // but not the other one
				return false; // because the other version continues, like ".1"
			}
			else if( thatBaseVersion.length() <= pos ) { // but not this one
				return true; // because this version continues, like ".1"
			}
			// so, both strings are not finished yet? then, go on...
			else
			{
				pos++; // skip the "." (to the start of the next number part)
				continue;
			}
		}
	}
	while( true );
	// infinite loops are prevented by the <= instead of ==
	// at the length checks

} // end of isNewerThan()


/**
 * Find out how stable this version is marked (stable, masked and such).
 * The check is done for the given architecture. If this version contains
 * additionally accepted keywords, these are checked as well.
 *
 * @param arch  The architecture that the stability should be checked for
 *              (e.g. "x86" or "~alpha").
 */
PortagePackageVersion::Stability PortagePackageVersion::stability(
	const QString& arch ) const
{
	if( m_isHardMasked == true )
		return HardMasked;

	// check for additional keywords
	if( !m_acceptedKeywords.empty() )
	{
		QString pureArch( arch );
		pureArch.remove('~');

		// The following checks are not completely correct, as they only check
		// against arch instead of all version keywords. Should be sufficient
		// for normal use though, as people are not supposed to add anything
		// but ~arch or -~arch to ACCEPT_KEYWORDS/package.keywords.
		for( QStringList::const_iterator keywordIterator = m_acceptedKeywords.begin();
		     keywordIterator != m_acceptedKeywords.end(); keywordIterator++ )
		{
			// Accept masked and stable packages
			// when the accepted keyword is ~arch or ~*
			if( ( *keywordIterator == "~*" || *keywordIterator == "~" + arch )
			    &&
			    ( m_keywords.contains("~" + pureArch)
			      || m_keywords.contains(pureArch)   )
			  )
			{
				return Stable;
			}
			// Don't accept packages when the accepted keyword is -arch
			else if( *keywordIterator == "-" + arch
			         && m_keywords.contains(arch) )
			{
				return NotAvailable;
			}
			// Accept stable packages for an accepted keyword named "*"
			else if( *keywordIterator == "*"
			         && m_keywords.contains(pureArch) )
			{
				return Stable;
			}
			// Don't accept anything if it's got -* in it
			else if( *keywordIterator == "-*" ) {
				return NotAvailable;
			}
		}
	}

	// check if the architecture is in there "as is"
	if( m_keywords.contains(arch) )
		return Stable;
	// check if there is a masked version of the architecture in there
	else if( m_keywords.contains( "~" + arch ) )
		return Masked;
	// if arch is masked, check if a stable version is in there
	else if( (arch[0] == '~') && (m_keywords.contains( arch.mid(1) )) )
		return Stable;
	// well, no such arch in the version info
	else // which is also "-*"
		return NotAvailable;
}


/**
 * Search for a revision (like "-r1") at the end of a version string
 * and get its revision number.
 *
 * @param versionString  The version that maybe contains a revision.
 * @param foundPos  A pointer to an integer that will be filled with the
 *                  index of "-" (from "-rNUM") inside the string (if there
 *                  is a revision), or -1 if no revision has been found.
 *                  You can leave out this argument if you don't need it.
 * @returns  The revision number (NUM out of "-rNUM"),
 *           or 0 if no revision has been found.
 */
int PortagePackageVersion::revisionNumber( const QString& versionString,
                                           int* foundPos ) const
{
	// search for a possible revision
	int pos = rxRevision.search( versionString );

	if( foundPos != NULL ) // return the position inside the string
		*foundPos = pos;

	if( pos == -1 ) {  // no revision, which is essentially the same as -r0
		return 0;
	}
	else { // has a revision, get its number
		return (const_cast<PortagePackageVersion*>(this))->
			rxRevision.cap(1).toInt();
	}
} // end of revisionNumber()

/**
 * Search for a suffix (like "_rc1", or "_beta2") inside a version string
 * and get a numerical representation for it.
 *
 * @param versionString  The version that maybe contains a suffix.
 * @param foundPos  A pointer to an integer that will be filled with the
 *                  index of "_" (e.g., from "_beta") inside the string (if
 *                  there is a suffix), or -1 if no suffix has been found.
 *                  You can leave out this argument if you don't need it.
 * @returns  A numerical representation of the suffix. If there is no suffix
 *           then this is 0. Suffixes with lower precendence ("_alpha" up to
 *           "_rc") get numbers below 0, while suffixes with higher precedence
 *           (that's only "_p") get numbers above 0. Overall, a higher number
 *           means higher precedence.
 */
long PortagePackageVersion::suffixNumber( const QString& versionString,
                                          int* foundPos ) const
{
	// search for a possible suffix
	int pos = rxSuffix.search( versionString );

	if( foundPos != NULL ) // return the position inside the string
		*foundPos = pos;

	if( pos == -1 ) { // no suffix, which means no change in importance
		return 0;
	}
	else // there is a suffix, check which one
	{
		// the regexps have to be casted to normal, because the const
		// method signature makes them const too (which is not wanted)
		QString suffix = (const_cast<PortagePackageVersion*>(this))->
			rxSuffix.cap(1);
		int suffixNumber = (const_cast<PortagePackageVersion*>(this))->
			rxSuffix.cap(2).toInt(); // X in, say "_betaX"
		// if it's just "_beta" (which is allowed), toInt("") returns 0.

		// The big numbers are needed because some people use stuff like
		// _p20041130 which is more than an integer can handle
		if( suffix == "alpha" ) // alpha release
			return -400000000 + suffixNumber;
		else if( suffix == "beta" ) // beta release
			return -300000000 + suffixNumber;
		else if( suffix == "pre" ) // prerelease
			return -200000000 + suffixNumber;
		else if( suffix == "rc" ) // release candidate
			return -100000000 + suffixNumber;
		else if( suffix == "p" ) // patch level. better than a normal release.
			return 100000000 + suffixNumber;
		else // should not happen, but anyways
			return 0;
	}
} // end of suffixNumber()

/**
 * Search for a trailing character (like "a", "b" or "i") at the end of
 * a version string and get a numerical representation for it.
 *
 * @param versionString  The version that maybe contains a trailing character.
 * @param foundPos  A pointer to an integer that will be filled with the
 *                  index of the character inside the string (if there is
 *                  such a character), or -1 if no character has been found.
 *                  You can leave out this argument if you don't need it.
 * @returns  A numerical representation of the character (1 or more).
 *           or 0 if no revision has been found.
 */
int PortagePackageVersion::trailingCharNumber( const QString& versionString,
                                               int* foundPos ) const
{
	// search for a possible trailing character
	int pos = rxTrailingChar.search( versionString );
	if( pos != -1 )
		pos++; // because the regexp starts one character left
		       // of the actual position

	if( foundPos != NULL ) // return the position inside the string
		*foundPos = pos;

	if( pos == -1 ) {  // no trailing character, so return 0
		return 0;
	}
	else { // has a trailing character, get its number
		return (int) versionString.at(pos).latin1();
	}
} // end of trailingCharNumber()


//
// Accessor methods
//

/**
 * Returns the package description.
 * @see setDescription
 */
const QString& PortagePackageVersion::description() const
{
	return m_description;
}

/**
 * Set the package description of this package version.
 * @see description
 */
void PortagePackageVersion::setDescription( const QString& description )
{
	m_description = description;
}

/**
 * Returns true if this package version is installed, false if not.
 * @see setInstalled
 */
bool PortagePackageVersion::isInstalled() const
{
	return m_installed;
}

/**
 * Set this true if you know that this package version is installed.
 * By default it is assumed that it's not.
 * @see isInstalled
 */
void PortagePackageVersion::setInstalled( bool isInstalled )
{
	m_installed = isInstalled;
}

/**
 * Returns true if this package version resides in the Portage overlay,
 * false if it's in the mainline Portage tree.
 * @see setOverlay
 */
bool PortagePackageVersion::isOverlay() const
{
	return m_overlay;
}

/**
 * Set this true if you know that this package version resides in the Portage
 * overlay. By default it is assumed that it's in the mainline Portage tree.
 * @see isOverlay
 */
void PortagePackageVersion::setOverlay( bool isOverlay )
{
	m_overlay = isOverlay;
}

/**
 * Get the ebuild's last modification time.
 * @see setDate
 */
const QString& PortagePackageVersion::date() const
{
	return m_date;
}

/**
 * Set the ebuild's last modification time.
 * @see date
 */
void PortagePackageVersion::setDate( const QString& date )
{
	m_date = date;
}

/**
 * Get the home page of this package version.
 * @see setHomepage
 */
const QString& PortagePackageVersion::homepage() const
{
	return m_homepage;
}

/**
 * Set the home page of this package version.
 * @see homepage
 */
void PortagePackageVersion::setHomepage( const QString& homepage )
{
	m_homepage = homepage;
}

/**
 * Get the slot that this package is in.
 * @see setSlot
 */
const QString& PortagePackageVersion::slot() const
{
	return m_slot;
}

/**
 * Set the slot that this package is in.
 * @see slot
 */
void PortagePackageVersion::setSlot( const QString& slot )
{
	m_slot = slot;
}

/**
 * Get the licenses used for this package.
 */
QStringList& PortagePackageVersion::licenses()
{
	return m_licenses;
}

/**
 * Set the licenses used for this package.
 */
void PortagePackageVersion::setLicenses( const QStringList& licenses )
{
	m_licenses = licenses;
}

/**
 * Get the keywords of this package.
 */
QStringList& PortagePackageVersion::keywords()
{
	return m_keywords;
}

/**
 * Set the keywords of this package.
 */
void PortagePackageVersion::setKeywords( const QStringList& keywords )
{
	m_keywords = keywords;
}

/**
 * Get the USE flags that this package can use.
 */
QStringList& PortagePackageVersion::useflags()
{
	return m_useflags;
}

/**
 * Set the USE flags that this package can use.
 */
void PortagePackageVersion::setUseflags( const QStringList& useflags )
{
	m_useflags = useflags;
}

/**
 * Get the list of accepted keywords marked for this package.
 * If the global list of keywords (etc. just x86, as in most cases) is also
 * part of this list (which may be, for example, x86, ~amd64 and ~ia64)
 * then the ebuild is stable and can be installed.
 */
QStringList& PortagePackageVersion::acceptedKeywords()
{
	return m_acceptedKeywords;
}

/**
 * Set the list of accepted keywords marked for this package.
 */
void PortagePackageVersion::setAcceptedKeywords( const QStringList& keywords )
{
	m_acceptedKeywords = keywords;
}

/**
 * Set the space that this package needs on hard disk.
 * For Portage, this is the size of the files that have to be downloaded,
 * the hard disk space of the resulting compilation can not be predicted.
 */
long PortagePackageVersion::size() const
{
	return m_size;
}

/**
 * Set the space that this package needs on hard disk.
 */
void PortagePackageVersion::setSize( long size )
{
	m_size = size;
}

/**
 * Returns true if this version is marked as hard masked, false if not.
 */
bool PortagePackageVersion::isHardMasked() const
{
	return m_isHardMasked;
}

/**
 * Set this version hard masked or not.
 * By default, versions are not marked as hard masked.
 */
void PortagePackageVersion::setHardMasked( bool isHardMasked )
{
	m_isHardMasked = isHardMasked;
}

/**
 * Determine if ebuild information for this package has already been loaded.
 * If not, you have to run a PortagePackageLoader on this object if you want
 * to access anything other than the version string or the isInstalled
 * property.
 */
bool PortagePackageVersion::hasDetailedInfo() const
{
	return m_hasDetailedInfo;
}

/**
 * To be set by the PortagePackageLoader when ebuild information has been
 * loaded.
 */
void PortagePackageVersion::setHasDetailedInfo( bool hasDetailedInfo )
{
	m_hasDetailedInfo = hasDetailedInfo;
}

} // namespace
