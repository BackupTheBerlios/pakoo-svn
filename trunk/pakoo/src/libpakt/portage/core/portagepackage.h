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

#ifndef LIBPAKTPORTAGEPACKAGE_H
#define LIBPAKTPORTAGEPACKAGE_H

#include "../../core/package.h"
#include "portagepackageversion.h"

#include <qstring.h>


namespace libpakt {

/**
 * A Package specializing in Portage.
 * It creates PortageVersion children and features slot support.
 */
class PortagePackage : public Package
{
public:
	PortagePackage( PackageCategory* category, const QString& name );

	void clear();
	void removeVersion( const QString& version );
	PortagePackageVersion* insertVersion( const QString& versionString );
	PortagePackageVersion* version( const QString& versionString );

	bool canUpdate( PackageVersion* version );

	QString description();
	QString shortDescription();

	QStringList slotList();
	QValueList<PackageVersion*> sortedVersionListInSlot( const QString& slot );

protected:
	PortagePackageVersion* createPackageVersion( const QString& versionString );

private:
	QString m_cachedDescription;
};

}

#endif // LIBPAKTPORTAGEPACKAGE_H