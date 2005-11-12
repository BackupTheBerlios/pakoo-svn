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

#ifndef PORTAGEHTMLGENERATOR_H
#define PORTAGEHTMLGENERATOR_H

#include <qstring.h>


namespace libpakt {

/**
 * A class providing functions to generate HTML sources from items
 * in a portage tree.
 */
class PortageHTMLGenerator
{
public:
	PortageHTMLGenerator( const QString& arch );
	~PortageHTMLGenerator();

	void setArchitecture( const QString& arch );
	QString fromPackage( Package* package, PackageVersion* version = NULL );

protected:
	PackageVersion* displayableVersion();
	QString packageDescription();
	QString packageHomepage();
	QString packageVersions();
	QString packageVersions( const QString& slot );

	//! The currently processed Package object.
	Package* package;
	//! The currently processed PackageVersion object.
	PackageVersion* version;
	//! The system architecture, like "x86" or "~alpha".
	QString arch;
};

}

#endif // PORTAGEHTMLGENERATOR_H
