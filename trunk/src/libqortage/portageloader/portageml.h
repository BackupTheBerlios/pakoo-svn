/***************************************************************************
 *   Copyright (C) 2004 by karye <karye@users.sourceforge.net>             *
 *   Copyright (C) 2005 by Jakob Petsovits <jpetso@gmx.at>                 *
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

#ifndef PORTAGEML_H
#define PORTAGEML_H

#include "portageloaderbase.h"

#include <qdom.h>

class Package;
class PortageTree;


/**
 * Naturally, scanning the portage tree is relatively slow, so you don't want to
 * do that every time when starting the program. For this reason, the PortageML
 * class has been created, which can "cache" a PortageTree object. It reads
 * and writes XML files containing a textual representation of the tree
 * and its packages. That way a PortageTree object can be saved and restored
 * fast. It can optionally be run in an own thread.
 *
 * @short  A class to read and write an XML representation of a PortageTree object.
 */
class PortageML : public PortageLoaderBase
{
public:
    PortageML();

	PortageLoaderBase::Error loadFile( PortageTree* portageTree, const QString& filename );
	PortageLoaderBase::Error saveFile( PortageTree* portageTree, const QString& filename );

	bool startLoadingFile( QObject* receiver, PortageTree* portageTree, const QString& filename );
	bool startSavingFile( QObject* receiver, PortageTree* portageTree, const QString& filename );

protected:
	enum Action {
		LoadFile,
		SaveFile
	};

	void run();

	PortageLoaderBase::Error loadTreeElement( const QDomElement& element );
	bool loadPackageElement( const QDomElement& element );
	bool loadVersionElement( const QDomElement& element );
	QDomElement createTreeElement( QDomDocument& doc );
	QDomElement createPackageElement( QDomDocument& doc );

	//! The PortageTree object that will be filled (when loading) or read (when saving).
	PortageTree* tree;
	//! The currently processed package.
	Package* package;

	//! The action that will be performed when running as thread.
	PortageML::Action action;
	//! The file that will be read or written.
	QString filename;
	//! A counter that is incremented with each added package.
	int packageCount;
	//! A counter that is incremented with each added package where the installed flag is set.
	int packageCountInstalled;
};

#endif // PORTAGEML_H
