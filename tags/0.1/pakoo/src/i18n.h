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

// Menu entry texts
#define MENUTEXT_INSTALLEBUILD i18n("Menu command", "&Install Ebuild File...")
#define MENUTEXT_SYNC i18n("Menu command", "&Sync")
#define MENUTEXT_UPDATE i18n("Menu command", "&Update Packages...")
#define MENUTEXT_CLEANUP i18n("Menu command", "&Clean Up...")
#define MENUTEXT_FIND i18n("Menu command", "&Find Packages...")
#define MENUTEXT_INSTALL i18n("Menu command", "&Install Selected...")
#define MENUTEXT_UNINSTALL i18n("Menu command", "&Uninstall Selected...")
#define MENUTEXT_ i18n("Menu command", )

// Widget labels
#define TREEVIEWTEXT i18n("Label of the package tree view's vertical tab", "Browse Packages")
#define ACTIONVIEWTEXT i18n("Label of the action view's vertical tab", "Actions")
#define CONFIGVIEWTEXT i18n("Label of the config view's vertical tab", "Configuration")

// Status bar text fragments
#define LOADINGTREEPACKAGESTEXT i18n("Loading packages from portage tree: %1 packages (%2 installed).")
#define LOADINGINSTALLEDPACKAGESTEXT i18n("Loading installed packages: %1 packages (%2 installed).")
#define ERRORLOADINGTEXT i18n("Status bar text when the tree wasn't loaded.", "Error loading portage tree.")
#define PACKAGESINCATEGORYTEXT i18n("Displayed when browsing packages. %2 is the category name.", "%1 packages in %2, %3 installed.%4")
#define THEPORTAGETREETEXT i18n("Substitute for the category name when browsing 'All Packages'.", "the portage tree")
#define LOADINGPACKAGEDETAILSTEXT i18n("Appended to the 'x packages in category y' text (as %4) when detailed descriptions are still loaded.", " (Loading package details...)")

// Tooltips
#define TOOLTIP_ABORTLOADINGPACKAGEDETAILS i18n("Tooltip for the button next to the status bar's progress bar.", "Abort loading package details")

// What's this help texts
#define WHATSTHIS_ABORTLOADINGPACKAGEDETAILS i18n("What's this for the button next to the status bar's progress bar.", "With this button you can abort the progress of loading package details. This stops harddisk reading activity, with the effect that remaining package descriptions are not loaded until you browse another package category.")

// debug output text
#define SECONDSTEXT i18n("Debug output, showing the elapsed loading time for a portage tree.", "(%1 seconds)")

#endif // PAKOO_I18N_H
