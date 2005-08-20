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

#include "portagehtmlgenerator.h"


namespace libpakt {

/**
 * Initialize this object.
 *
 * @param arch  The system architecture (e.g. "x86", "~alpha", or similar)
 *              which the generator focusses on. This is mostly used for
 *              determining stability (masking).
 */
PortageHTMLGenerator::PortageHTMLGenerator( const QString& arch )
{
	this->arch = arch;
}

/**
 * Deconstruct this object.
 */
PortageHTMLGenerator::~PortageHTMLGenerator()
{
}

/**
 * Set the architecture that is used for retrieving some
 * package details, like package masking. Default is "x86".
 */
void PortageHTMLGenerator::setArchitecture( const QString& arch )
{
	this->arch = arch;
}

/**
 * Generate HTML for a Package object. If version is NULL,
 * the generated HTML will provide an overview of the package,
 * else it will be focussed on this specific version.
 */
QString PortageHTMLGenerator::fromPackage( Package* package,
                                           PackageVersion* version )
{
	if( package == NULL )
		return "<html><body></body></html>";


	QString versionSuffix, notes;

	this->package = package;
	this->version = version;

	if( version != NULL )
		versionSuffix = "-" + version->version;
	else
		versionSuffix = "";

	notes += "(" + packageHomepage();

	if( version != NULL )
	{
		notes += " - ";
		PackageVersion::Stability stability = version->stability(arch);

		if( stability == PackageVersion::Stable )
			notes += i18n("version property in the info view",
			              "stable");
		else if( stability == PackageVersion::Masked )
			notes += i18n("version property in the info view",
			              "masked");
		else if( stability == PackageVersion::HardMasked )
			notes += i18n("version property in the info view",
			              "hard masked");
		else if( stability == PackageVersion::NotAvailable )
			notes += i18n("version property in the info view",
			              "not available");

		notes += ", ";
		if( version->installed == true ) {
			notes += i18n("version property in the info view",
			              "installed");
		}
		else {
			notes += i18n("version property in the info view",
			              "not installed");
		}
	}
	else
	{
		if( package->hasInstalledVersion() ) {
			notes += " - "
			         + i18n("package property in the info view", "installed");
		}
	}
	notes += ")";

	QString output =
		"<html><body style=\"margin-top: 0px; margin-left: 0.5em;\">"
		"<span style=\"font-size:x-small;\">"
		"<p style=\"margin-top: 0.5em;\">"
		+ package->category()->userVisibleName() + " / <strong>"
		+ package->name() + versionSuffix + "</strong> " + notes
		+ packageDescription() + "</p><p style=\"margin-top: -0.2em;\">"
		+ packageVersions() + "</p></span></body></html>";

	return output;
}

/**
 * Returns the PackageVersion object that fits best for extracting
 * version specific information (like package description or home page).
 * If this class's 'version' member is not NULL then this one is returned,
 * otherwise the function returns a recent version of the package.
 * If the package doesn't contain versions, NULL is returned.
 */
PackageVersion* PortageHTMLGenerator::displayableVersion()
{
	// possible enhancement: cache the displayableVersion so it's only
	// checked once per package. Would need new class members for
	// the cached version and a dirty flag.

	if( version != NULL ) {
		return version;
	}
	else {
		if( package->hasVersions() == false ) {
			return NULL;
		}

		PackageVersion* latestVersion = package->latestStableVersion(arch);
		if( latestVersion != NULL ) {
			return latestVersion;
		}
		else { // if there is no stable version
			return package->latestVersion();
			// latestVersion() != NULL, because package->hasVersions() == true
		}
	}
}

/**
 * Generate an HTML snippet containing a line break and the current package's
 * description (or the one of the current version).
 */
QString PortageHTMLGenerator::packageDescription()
{
	PackageVersion* dispVersion = displayableVersion();

	if( dispVersion != NULL )
		return "<br/>" + dispVersion->description;
	else
		return "";
}

/**
 * Generate an HTML snippet containing a linked HTML tag
 * (\<a href="..."\>...\</a\>) with the package's home page.
 * If none could be found, the function returns a "No Homepage" string
 * (supporting i18n, of course).
 */
QString PortageHTMLGenerator::packageHomepage()
{
	QString homepage;
	PackageVersion* dispVersion = displayableVersion();

	if( dispVersion != NULL )
		homepage = dispVersion->homepage;
	else
		homepage = "";

	if( homepage.isEmpty() == true ) {
		return i18n("Info text in the package view if the package "
		            "has no home page", "No Homepage");
	}
	else {
		return "<a href=\"" + homepage + "\">"
		       + homepage + "</a>";
	}
}

/**
 * Generate an HTML snippet containing a paragraph which displays
 * the current package's versions (sorted by slot).
 */
QString PortageHTMLGenerator::packageVersions()
{
	QString output, versionString;
	QStringList slotList = package->slotList();

	if( slotList.count() == 1 && slotList[0] == "0" ) {
		versionString = "<br/>" + packageVersions( "0" );
	}
	else
	{
		for( QStringList::iterator slotIterator = slotList.begin();
			 slotIterator != slotList.end(); slotIterator++ )
		{
			// prepend the strings, because high slots shall be first
			versionString = packageVersions( *slotIterator ) + versionString;
			versionString = "<br/><strong>"
			                + i18n("Caption in front of a slot's version "
			                       "list, will look like \"Slot xy:\".",
			                       "Slot %1").arg(*slotIterator)
			                + ":</strong> " + versionString;

		}
	}

	output = "<p>" + i18n("Caption in front of the list of package versions.",
	                      "Available Versions:") + versionString + "</p>";
	return output;
}

/**
 * Generate an HTML snippet containing a line with the given slot's versions.
 */
QString PortageHTMLGenerator::packageVersions( const QString& slot )
{
	QValueList<PackageVersion*> versions
		= package->sortedVersionListInSlot( slot );
	QValueList<PackageVersion*>::iterator versionIterator;
	QString output, border;

	//TODO: compile one style string in order not to duplicate code
	for( versionIterator = versions.begin();
	     versionIterator != versions.end(); versionIterator++ )
	{
		if( versionIterator != versions.begin() )
			output += ", ";

		if( version != NULL
		    && (*versionIterator)->version == version->version )
		{
			// mark selected versions with a black border
			border = "border-style: solid; border-width: thin;"
			         "border-color: black";
		}
		else
		{
			// don't mark non-selected versions
			border = "border-style: dotted; border-width: thin;"
			         "border-color: window";
		}

		if( (*versionIterator)->installed == true )
		{
			// mark installed versions bold and with green color
			output += "<span style=\"color: forestgreen;" + border
			          + "\"><strong>" + (*versionIterator)->version
			          + "</strong></span>";
		}
		else
		{
			PackageVersion::Stability stability
				= (*versionIterator)->stability(arch);

			if( stability == PackageVersion::Masked )
			{
				// mark masked versions red
				output += "<span style=\"color: red;" + border + "\">"
				          + (*versionIterator)->version + "</span>";
			}
			else if( stability == PackageVersion::HardMasked
			         || stability == PackageVersion::NotAvailable )
			{
				// mark non-available versions red and striked though
				output += "<span style=\"color: red; "
						"text-decoration: line-through;"
						+ border + "\">" + (*versionIterator)->version
						+ "</span>";
			}
			else { // stable
				output += "<span style=\"color: windowtext;" + border + "\">"
						+ (*versionIterator)->version + "</span>";
			}
		}
	}
	return output;
}

} // namespace
