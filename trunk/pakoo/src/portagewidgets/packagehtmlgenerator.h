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

#ifndef LIBPAKTPACKAGEHTMLGENERATOR_H
#define LIBPAKTPACKAGEHTMLGENERATOR_H

#include <htmlgenerator.h>

namespace libpakt {

class Package;

/**
 * This HTML Generator provides a standard implementation and capabilities
 * for derived classes to extend
 */
class PackageHTMLGenerator : public HTMLGenerator
{
public:
	PackageHTMLGenerator();

	void setPackage( Package* package );
	Package* package();

	virtual QString html();

protected:
	virtual QString cssRaw();
	virtual QString bodyElement();
	virtual QString captionSnippet();
	virtual QString descriptionSnippet();
	virtual QString availabilitySnippet();

private:
	Package* m_package;
};

}

#endif // LIBPAKTPACKAGEHTMLGENERATOR_H
