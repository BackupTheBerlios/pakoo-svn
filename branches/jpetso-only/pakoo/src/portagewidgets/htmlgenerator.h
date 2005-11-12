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

#ifndef LIBPAKTHTMLGENERATOR_H
#define LIBPAKTHTMLGENERATOR_H

#include <qstring.h>


namespace libpakt {

/**
 * @short A base class for HTML generating classes.
 *
 * HTMLGenerator essentially consists of one method that should be called,
 * which is html(). Derived classes may or may not provide means to
 * configure the output.
 */
class HTMLGenerator
{
public:
	HTMLGenerator();
	virtual ~HTMLGenerator() {};

	/**
	 * The main method of this class.
	 * Derived classes are supposed to overload this and return
	 * HTML markup that can be used in an info widget (or any other
	 * HTML capable output device, like a web browser).
	 */
	virtual QString html() = 0;

protected:
	QString headElement( const QString& title );
	QString cssElement();

	/**
	 * Returns pure CSS. No HTML, no tags, just style definitions.
	 * Derived classes can overload this method and get a
	 * ready-to-use CSS element with cssElement().
	 */
	virtual QString cssRaw() { return ""; }

};

}

#endif // LIBPAKTHTMLGENERATOR_H
