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

#ifndef LIBPAKTPORTAGEML_H
#define LIBPAKTPORTAGEML_H

#include "../core/threadedjob.h"

#include <qdom.h>


namespace libpakt {

class Package;
class PackageList;

/**
 * Naturally, scanning the portage tree is relatively slow, so you maybe don't
 * want to do it every time when starting the program. For this reason, the
 * PortageML class has been created, which can serialize ("cache") a
 * PackageList object. It reads and writes XML files containing a textual
 * representation of the tree and its packages. That way a PackageList object
 * can be saved and restored fast.
 *
 * Before starting the thread using start(), you'll have to call
 * setTreeObject(), setFileName() and setAction().
 *
 * @short  A class to read and write an XML representation of a PackageList object.
 */
class PortageML : public ThreadedJob
{
	Q_OBJECT

public:
	enum Action {
		LoadFile,
		SaveFile
	};

	PortageML();

	void setPackageList( PackageList* packages );
	void setFileName( const QString& filename );
	void setAction( PortageML::Action action );

signals:
	/**
	 * Emitted every once in a while when packages have been added to the
	 * PackageList object. The arguments specify the number of available
	 * packages and the number of installed ones.
	 * For the total amount of packages, please count() them when
	 * loading has finished.
	 */
	void packagesScanned( int packageCountAvailable,
	                      int packageCountInstalled );

	/**
	 * Emitted if the package tree has successfully been loaded from the
	 * specified file. The PackageList object now contains exactly the
	 * packages, versions and detailed info that have been read from
	 * the file.
	 */
	void finishedLoading( PackageList* packages, const QString& filename );

	/**
	 * Emitted if the package tree has successfully been saved to the
	 * specified file. The file now contains information about packages,
	 * versions and package details that can be read back afterwards.
	 */
	void finishedSaving( PackageList* packages, const QString& filename );

	/**
	 * Emitted for untranslated debug output, like starting the job,
	 * finishing part of it, or error messages.
	 */
	void debugOutput( QString output );

protected:
	JobResult performThread();
	void customEvent( QCustomEvent* event );

private:
	enum PortageMLEventType
	{
		PackagesScannedEventType = QEvent::User + 14342,
		FinishedFileEventType = QEvent::User + 14343
	};

	bool loadFile();
	bool saveFile();

	bool loadTreeElement( const QDomElement& element );
	bool loadPackageElement( const QDomElement& element );
	bool loadVersionElement( const QDomElement& element );
	QDomElement createTreeElement( QDomDocument& doc );
	QDomElement createPackageElement( QDomDocument& doc );

	void emitFinishedLoading();
	void emitFinishedSaving();
	void emitPackagesScanned();

	//! The PackageList object that will be filled (when loading) or read (when saving).
	PackageList* packages;
	//! The action that will be performed when running as thread.
	PortageML::Action action;
	//! The file that will be read or written.
	QString filename;

	//! The currently processed package.
	Package* package;
	//! A counter that is incremented with each added package.
	int packageCountAvailable;
	//! A counter that is incremented with each added package where the installed flag is set.
	int packageCountInstalled;


	//
	// nested event classes
	//

	class FinishedFileEvent : public QCustomEvent
	{
	public:
		FinishedFileEvent() : QCustomEvent( FinishedFileEventType ) {};
		Action action;
		PackageList* packages;
		QString filename;
	};

	class PackagesScannedEvent : public QCustomEvent
	{
	public:
		PackagesScannedEvent() : QCustomEvent( PackagesScannedEventType ) {};
		int packageCountAvailable;
		int packageCountInstalled;
	};
};

}

#endif // LIBPAKTPORTAGEML_H
