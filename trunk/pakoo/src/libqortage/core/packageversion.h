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

#ifndef LIBPAKTPACKAGEVERSION_H
#define LIBPAKTPACKAGEVERSION_H

#include <qstring.h>
#include <qstringlist.h>
#include <qregexp.h>


namespace libpakt {

/**
 * PackageVersion is a class for managing package version information,
 * such as description, home page or use flags, and of course the
 * version string itself.
 *
 * @short  A class for storing package version information.
 */
class PackageVersion
{
public:
	//! The "maskedness" of a package version.
	enum Stability {
		Stable,
		Masked,
		HardMasked,
		NotAvailable
	};

	PackageVersion( const QString& version = "" );

	PackageVersion::Stability stability( const QString& arch );
	bool isNewerThan( const QString& otherVersion );
	bool isOlderThan( const QString& otherVersion );


	// Info retrievable by retrieving QFileInfos for ebuilds
	// (without parsing their contents):

	//! The package version string.
	QString version;
	//! Date of the ebuild file's last modification.
	QString date;
	//! true if the package is installed, false otherwise.
	bool installed;
	//! true if the package is from the overlay tree, false otherwise.
	bool overlay;


	// Info retrievable by scanning and parsing the contents of an ebuild file:

	//! A short line describing the package.
	QString description;
	//! URL of the package's home page.
	QString homepage;
	//! The slot for this version. Mostly a number, but only has to be interpreted as string.
	QString slot;
	//! List of licenses that are used in the package.
	QStringList licenses;
	//! List of keywords, like x86 or ~alpha.
	QStringList keywords;
	//! List of use flags that influence compilation of the package.
	QStringList useflags;
	//! A list of additionally accepted keywords for this specific package.
	QStringList acceptedKeywords;

	//! A flag which is true if the ebuild belonging to this package has been parsed.
	bool hasDetailedInfo;


	// Info that's not in the ebuild:

	//! Downloaded file size in bytes (retrievable by scanning the digest).
	long size;
	/**
	 * true if this version is hardmasked, false otherwise.
	 * Retrievable by scanning package.[un]mask and Co.
	 */
	bool isHardMasked;

private:
	QRegExp rxNumber, rxTrailingChar, rxSuffix, rxRevision;

	int revisionNumber( const QString& versionString, int* foundPos = NULL );
	long suffixNumber( const QString& versionString, int* foundPos = NULL );
	int trailingCharNumber( const QString& versionString, int* foundPos = NULL );
};

}

#endif // LIBPAKTPACKAGEVERSION_H
