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

#include "packageversion.h"


/**
 * Initialize the version with its version string.
 */
PackageVersion::PackageVersion( QString version )
: rxNumber("\\d+"),
  rxSuffix("_(alpha|beta|pre|rc|p)(\\d+)(?:-r\\d+)?$"), // including the revision, but not for capturing
  rxRevision("-r(\\d+)$")
{
	this->version = version;
	this->installed = false;
	this->overlay = false;
	/* This is what QString does automatically:
	this->date = "";
	this->description = "";
	this->homepage = "";
	this->slot = "";*/
	this->hasDetailedInfo = false;
	this->size = 0;
	this->isHardMasked = false;
}

/**
 * Find out how stable this version is marked (stable, masked and such).
 * The check is done for the given architecture. If this version contains
 * additionally accepted keywords, these are checked as well.
 *
 * @param arch  The architecture that the stability should be checked for
 *              (e.g. "x86" or "~alpha").
 */
PackageVersion::Stability PackageVersion::stability( const QString& arch )
{
	if( isHardMasked == true )
		return HardMasked;

	// check for additional keywords
	if( !acceptedKeywords.empty() )
	{
		QString pureArch( arch );
		pureArch.remove('~');

		// The following checks are not completely correct, as they only check
		// against arch instead of all version keywords. Should be sufficient
		// for normal use though, as people are not supposed to add anything
		// but ~arch or -~arch to ACCEPT_KEYWORDS/package.keywords.
		for( QStringList::iterator keywordIterator = acceptedKeywords.begin();
		     keywordIterator != acceptedKeywords.end(); keywordIterator++ )
		{
			// Accept masked and stable packages
			// when the accepted keyword is ~arch or ~*
			if( ( *keywordIterator == "~*" || *keywordIterator == "~" + arch )
			    &&
			    ( this->keywords.contains("~" + pureArch)
			      || this->keywords.contains(pureArch)   )
			  )
			{
				return Stable;
			}
			// Don't accept packages when the accepted keyword is -arch
			else if( *keywordIterator == "-" + arch
			         && this->keywords.contains(arch) )
			{
				return NotAvailable;
			}
			// Accept stable packages for an accepted keyword named "*"
			else if( *keywordIterator == "*"
			         && this->keywords.contains(pureArch) )
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
	if( this->keywords.contains(arch) )
		return Stable;
	// check if there is a masked version of the architecture in there
	else if( this->keywords.contains( "~" + arch ) )
		return Masked;
	// if arch is masked, check if a stable version is in there
	else if( (arch[0] == '~') && (this->keywords.contains( arch.mid(1) )) )
		return Stable;
	// well, no such arch in the version info
	else // which is also "-*"
		return NotAvailable;
}



/**
 * Find out if this version has a higher version number than another one.
 *
 * @param otherVersion  Version string of the version that should be compared
 *                      to this one.
 * @return  true if this version is newer than the one given in the argument.
 *          false if the other version is newer (or if the strings are equal).
 */
bool PackageVersion::isNewerThan( const QString& otherVersion )
{
	// this* is for this->version, that* is for otherVersion
	QString thisNum, thatNum;

	// Numerical representation of the suffix.
	// Higher number means higher precedence.
	long thisSuffix, thatSuffix;

	// Revision number. 0 means the version doesn't have a revision.
	int thisRevision, thatRevision;

	uint pos; // multi-purpose position integer
	int revisionPos, suffixPos; // start index of revision and suffix

	// Retrieve revision, suffix and their positions in the version string
	thisRevision = PackageVersion::revisionNumber( this->version, &revisionPos );
	thisSuffix = PackageVersion::suffixNumber( this->version, &suffixPos );

	// determine the first non-base-version character
	if( suffixPos != -1 )
		pos = suffixPos;
	else if( revisionPos != -1 )
		pos = revisionPos;
	else
		pos = this->version.length();

	// So, now we have a version string stripped of suffix and revision
	QString thisBaseVersion( this->version.left(pos) );

	// Same procedure for the other version string
	thatRevision = PackageVersion::revisionNumber( otherVersion, &revisionPos );
	thatSuffix = PackageVersion::suffixNumber( otherVersion, &suffixPos );

	// determine the first non-base-version character
	if( suffixPos != -1 )
		pos = suffixPos;
	else if( revisionPos != -1 )
		pos = revisionPos;
	else
		pos = this->version.length();

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
		rxNumber.search( this->version, pos );
		thisNum = rxNumber.cap(0);
		rxNumber.search( otherVersion, pos );
		thatNum = rxNumber.cap(0);

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
				// compare suffixes, if they are also the same, then revisions
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
int PackageVersion::revisionNumber( const QString& versionString,
                                    int* foundPos )
{
	// search for a possible revision
	int pos = rxRevision.search( versionString );

	if( foundPos != NULL ) // return the position inside the string
		*foundPos = pos;

	if( pos == -1 ) {  // no revision, which is essentially the same as -r0
		return 0;
	}
	else { // has a revision, get its number
		return rxRevision.cap(1).toInt();
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
long PackageVersion::suffixNumber( const QString& versionString,
                                  int* foundPos )
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
		QString suffix = rxSuffix.cap(1);
		int suffixNumber = rxSuffix.cap(2).toInt(); // NUM in, say, "_betaNUM"

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
 * Find out if this version has a lower version number than another one.
 *
 * @param otherVersion  Version string of the version that should be compared
 *                      to this one.
 * @return  true if this version is older than the one given in the argument.
 *          false if the other version is older (or if the strings are equal).
 */
bool PackageVersion::isOlderThan( const QString& otherVersion )
{
	if( this->version == otherVersion || isNewerThan(otherVersion) == true )
		return false;
	else
		return true;
}
