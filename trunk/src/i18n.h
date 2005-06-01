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

#ifndef PAKOO_I18N_H
#define PAKOO_I18N_H

#include <klocale.h>

// Status bar text fragments
#define LOADINGTREEPACKAGESTEXT i18n("Loading packages from portage tree: %1 packages (%2 installed).")
#define LOADINGINSTALLEDPACKAGESTEXT i18n("Loading installed packages: %1 packages (%2 installed).")
#define ERRORLOADINGTEXT i18n("Status bar text when the tree wasn't loaded.", "Error loading portage tree.")
#define PACKAGESINCATEGORYTEXT i18n("Displayed when browsing packages. %2 is the category name.", "%1 packages in %2, %3 installed.%4")
#define THEPORTAGETREETEXT i18n("Substitute for the category name when browsing 'All Packages'.", "the portage tree")
#define LOADINGPACKAGEDETAILSTEXT i18n("Appended to the 'x packages in category y' text (as %4) when detailed descriptions are still loaded.", " (Loading package details...)")

// debug output text
#define SECONDSTEXT i18n("Debug output, showing the elapsed loading time for a portage tree.", "(%1 seconds)")

#endif // PAKOO_I18N_H
