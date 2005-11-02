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

#include "packagehtmlgenerator.h"

// libqortage includes
#include <core/package.h>

// KDE includes
#include <klocale.h>


namespace libpakt {

/**
 * Initializes this object.
 */
PackageHTMLGenerator::PackageHTMLGenerator()
 : HTMLGenerator()
{
	m_package = NULL;
}

/**
 * Specify the package for which package info should be generated.
 */
void PackageHTMLGenerator::setPackage( Package* package )
{
	m_package = package;
}

/**
 * Get the package for which package info should be generated.
 */
Package* PackageHTMLGenerator::package()
{
	return m_package;
}

/**
 * Provides standard package information. In this default implementation,
 * it contains the package name (including its category) inside the \<title\>
 * element and the output of bodyElement() for the visible stuff.
 *
 * @see bodyElement()
 */
QString PackageHTMLGenerator::html()
{
	if( package() == NULL )
		return "<html><body></body></html>";

	return "<html>"
		+ headElement( package()->category()->userVisibleName()
		               + " / " + package()->name() )
		+ bodyElement()
		+ "</html>";
}

/**
 * Implemented for a nice package info default stylesheet.
 * There are attributes for: body, .caption, .description, .packagename,
 * .installed and .notavailable.
 */
QString PackageHTMLGenerator::cssRaw()
{
	return
		"body { margin-top: 0px; margin-left: 0.5em;"
		" font-size:x-small; }\n"
		".caption { margin-top: 0.5em; }\n"
		".description { margin-top: -0.2em; }\n"
		".packagename { font-weight:bold; }\n"
		".installed { font-weight:bold; color: forestgreen; }\n"
		".notavailable { font-weight:bold; color: red; }\n";
}

/**
 * Provides the body element ready to use inside an \<html\> tag.
 * This default implementation uses the captionElement() and
 * descriptionElement() methods to construct the actual contents.
 * Both are wrapped in a \<p\> element with a CSS class name to
 * allow defining CSS styles for it. The class name of the caption
 * paragraph is "caption", and the one of the description paragraph is
 * (right!) "description".
 *
 * @see captionElement()
 * @see descriptionElement()
 */
QString PackageHTMLGenerator::bodyElement()
{
	return "<body><p class=\"caption\">" + captionSnippet() + "</p>"
		"<p class=\"description\">" + descriptionSnippet() + "</p></body>";
}

/**
 * Provides a simple HTML snippet, preferably used within a \<p\> tag, which
 * is supposed to be the caption (title) part of the package info HTML page.
 * In this default implementation, the package category, name and isInstalled
 * indicator are wrapped in \<span class="xxx"\>, so you can define CSS styles
 * for it. The CSS class names are:
   "packagecategory" for the package category
   "packagename" for the package name, and
   "isinstalled" for the short text saying if it's installed or not.
 *
 * This method assumes that package() returns a valid Package object
 * and especially not NULL.
 */
QString PackageHTMLGenerator::captionSnippet()
{
	return "<span class=\"packagecategory\">"
		+ package()->category()->userVisibleName() + "</span> / "
		"<span class=\"packagename\">" + package()->name() + "</span>"
		" - " + availabilitySnippet();
}

/**
 * Provides a simple HTML snippet, preferably used within a \<p\> tag, which
 * is supposed to be the description part of the package info HTML page.
 * In this default implementation, the package description is wrapped in
 * \<span class="packagedescription"\>, so you can define CSS styles
 * for it.
 *
 * This method assumes that package() returns a valid Package object
 * and especially not NULL.
 */
QString PackageHTMLGenerator::descriptionSnippet()
{
	return "<span class=\"packagedescription\">"
		+ package()->description() + "</span>";
}

/**
 * Provides a simple HTML snippet, preferably used within a \<p\> tag, which
 * is supposed to be part of the caption part of the package info HTML page.
 * In this default implementation, this indicator is wrapped in
 * \<span class="installed"\> if the package contains at least one installed
 * version, \<span class="notinstalled"\> if it doesn't and
 * \<span class="notavailable"\> it it can't even be installed.
 * (That way, you can define CSS styles for it depending on that).
 *
 * This method assumes that package() returns a valid Package object
 * and especially not NULL.
 */
QString PackageHTMLGenerator::availabilitySnippet()
{
	if( package()->containsInstalledVersion() ) {
		return "<span class=\"installed\">" +
		i18n("version property in the info view",
		     "installed")
		+ "</span>";
	}
	if( package()->containsAvailableVersion() == false ) {
		return "<span class=\"notavailable\">" +
			i18n("version property in the info view",
			     "not available")
			+ "</span>";
	}
	else {
		return "<span class=\"notinstalled\">" +
		i18n("version property in the info view",
		     "not installed")
		+ "</span>";
	}
}

} // namespace
