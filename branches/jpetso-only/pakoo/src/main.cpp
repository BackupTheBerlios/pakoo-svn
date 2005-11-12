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


#include <kapplication.h>
#include <dcopclient.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include "pakoo.h"

// APPNAME ("Pakoo")is defined in pakoo.h
// PACKAGE ("pakoo") and VERSION (e.g. "0.1") are
// set by configure.in.in and defined in config.h
#define COPYRIGHT "(c) 2005 Jakob Petsovits\n(c) 2004 karye"
#define HOMEPAGE "http://pakoo.berlios.de"
#define DESCRIPTION I18N_NOOP("\nPakoo - Package management for Gentoo Linux")
#define DISCLAIMER  I18N_NOOP( \
   "   The name \"Gentoo\" and the \"g\" logo (as well as the derived\n" \
   "   Pakoo \"p\" logo) are trademarks of Gentoo Technologies, Inc.,\n" \
   "   and the Gentoo artwork is their copyright as well. Pakoo is\n" \
   "   neither part of the Gentoo project nor directed or managed\n" \
   "   by Gentoo Foundation, Inc." )

//static KCmdLineOptions options[] =
//{
//    { "+[URL]", I18N_NOOP( "Document to open" ), 0 },
//    KCmdLineLastOption
//};

int main(int argc, char **argv)
{
	KAboutData about(PACKAGE, APPNAME, VERSION, DESCRIPTION,
                     KAboutData::License_GPL, COPYRIGHT,
	                 DISCLAIMER, HOMEPAGE,
	                 "https://developer.berlios.de/bugs/?group_id=3829"
	                 /* the place for bug reports */ );

	about.addAuthor( "Jakob Petsovits",
	                 I18N_NOOP("Main Pakoo developer"),
	                 "jpetso@gmx.at" );
	about.addAuthor( "karye", I18N_NOOP("Initial code (from Kuroo)"),
	                 "karye@users.sourceforge.net", "http://tux.myftp.org" );

	about.addCredit( I18N_NOOP("Doug Whiteley (a.k.a. rezza)"),
	                 I18N_NOOP("who did the the amazing web design for Pakoo."),
	                 "dougwhiteley@gmail.com", 0 );
	about.addCredit( I18N_NOOP("Gentoo's Portage developers"),
	                 I18N_NOOP("For making a package management system that just rocks."),
	                 0, "http://www.gentoo.org" );
	about.addCredit( I18N_NOOP("The KDE developers (including Trolltech with Qt)"),
	                 I18N_NOOP("It's just a great desktop and development framework."),
	                 0, "http://www.kde.org" );

	KCmdLineArgs::init(argc, argv, &about);
	KApplication app;

	// register ourselves as a dcop client
	app.dcopClient()->registerAs(app.name(), false);

	// see if the app is starting with session management
	if (app.isRestored())
	{
		RESTORE(Pakoo);
	}
	else
	{
		// no session.. just start up normally
		/*KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
		if (args->count() == 0)
		{*/
			Pakoo *widget = new Pakoo;
			widget->show();
		/*}
		else
		{
			for (int i = 0; i < args->count(); i++)
			{
				pakoo *widget = new pakoo;
				widget->show();
				widget->load(args->url(i));
			}
		}
		args->clear();*/
	}

	return app.exec();
}
