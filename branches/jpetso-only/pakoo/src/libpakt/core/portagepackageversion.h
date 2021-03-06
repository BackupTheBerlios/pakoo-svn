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

#ifndef LIBPAKTPORTAGEPACKAGEVERSION_H
#define LIBPAKTPORTAGEPACKAGEVERSION_H

#include "packageversion.h"

namespace libpakt {

/**
 * A PackageVersion specializing in Portage.
 * It features accurate version comparison just like Portage itself does,
 * and can store additional values from Portage that the original
 * PackageVersion does not provide.
 */
class PortagePackageVersion : public PackageVersion
{
public:
	friend class PortagePackage;

	//! The "maskedness" of a package version.
	enum Stability {
		Stable,
		Masked,
		HardMasked,
		NotAvailable
	};

	bool isAvailable() const;

    bool isNewerThan( const QString& otherVersion ) const;

	PortagePackageVersion::Stability stability( const QString& arch ) const;

	bool isInstalled() const;
	bool isOverlay() const;
	const QString& date() const;
	const QString& description() const;
	const QString& homepage() const;
	const QString& slot() const;
	QStringList& licenses();
	QStringList& keywords();
	QStringList& useflags();
	QStringList& acceptedKeywords();
	long size() const;
	bool isHardMasked() const;
	bool hasDetailedInfo() const;

	void setInstalled( bool isInstalled );
	void setOverlay( bool isOverlay );
	void setDate( const QString& date );
	void setDescription( const QString& description );
	void setHomepage( const QString& homepage );
	void setSlot( const QString& slot );
	void setLicenses( const QStringList& licenses );
	void setKeywords( const QStringList& keywords );
	void setUseflags( const QStringList& useflags );
	void setAcceptedKeywords( const QStringList& acceptedKeywords );
	void setSize( long size );
	void setHardMasked( bool isHardMasked );
	void setHasDetailedInfo( bool hasDetailedInfo );

protected:
	PortagePackageVersion( Package* parent, const QString& version );

private:
	int revisionNumber( const QString& versionString, int* foundPos = NULL ) const;
	long suffixNumber( const QString& versionString, int* foundPos = NULL ) const;
	int trailingCharNumber( const QString& versionString, int* foundPos = NULL ) const;


	// Info retrievable by retrieving QFileInfos for ebuilds
	// (without parsing their contents):

	/** Date of the ebuild file's last modification. */
	QString m_date;
	/** true if the package is installed, false otherwise. */
	bool m_installed;
	/** true if the package is from the overlay tree, false otherwise. */
	bool m_overlay;


	// Info retrievable by scanning and parsing the contents of an ebuild file:

	/** A short line describing the package. */
	QString m_description;
	/** URL of the package's home page. */
	QString m_homepage;
	/** The slot for this version. Mostly a number, but only has to be interpreted as string. */
	QString m_slot;
	/** List of licenses that are used in the package. */
	QStringList m_licenses;
	/** List of keywords, like x86 or ~alpha. */
	QStringList m_keywords;
	/** List of use flags that influence compilation of the package. */
	QStringList m_useflags;
	/** A list of additionally accepted keywords for this specific package. */
	QStringList m_acceptedKeywords;

	/** A flag which is true if the ebuild belonging to this package has been parsed. */
	bool m_hasDetailedInfo;


	// Info that's not in the ebuild:

	/** Downloaded file size in bytes (retrievable by scanning the digest). */
	long m_size;
	/** true if this version is hardmasked, false otherwise.
	 * Retrievable by scanning package.[un]mask and Co. */
	bool m_isHardMasked;

	QRegExp rxNumber, rxRevision, rxSuffix, rxTrailingChar;
};

}

#endif // LIBPAKTPORTAGEPACKAGEVERSION_H
