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

#include "htmlgenerator.h"

namespace libpakt {

/**
 * Initializes this object.
 */
HTMLGenerator::HTMLGenerator()
{
}

/**
 * Provides a \<head\> element ready for use inside an \<html\> tag.
 * It consists of a title (given as argument) and a css style
 * (which can be defined by overloading cssRaw()).
 *
 * @see cssElement()
 * @see cssRaw()
 */
QString HTMLGenerator::headElement( const QString& title )
{
	return "<head><title>" + title + "</title>" + cssElement() + "</head>";
}

/**
 * Provides a \<style\> element ready for use inside an HTML \<head\> tag.
 * It makes use of cssRaw() which can be defined by derived classes
 * to get their styles in easily.
 *
 * @see cssRaw()
 */
QString HTMLGenerator::cssElement()
{
	QString css = cssRaw();
	if( css.isEmpty() ) {
		return "";
	}
	else {
		return "<style type=\"text/css\">\n<!--\n"
			+ cssRaw() + "\n-->\n</style>";
	}
}

}
