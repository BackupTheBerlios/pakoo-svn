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

#ifndef LIBPAKTFILEATOMLOADERBASE_H
#define LIBPAKTFILEATOMLOADERBASE_H

#include <qstring.h>

#include "../../base/core/fileloaderbase.h"


namespace libpakt {

class PackageList;
template<class T> class TemplatedPackageList;
class PortagePackage;
class PortagePackageVersion;
class DependAtom;

/**
 * This is a base class for derived ones that read files with one
 * DEPEND atom per line, like the config files in /etc/portage/.
 *
 * To use it, set it up calling the setPackageList and setFileName
 * member functions, then call start() or perform() to process the file.
 */
class FileAtomLoaderBase : public FileLoaderBase
{
	Q_OBJECT

public:
	FileAtomLoaderBase();

	void setPackageList( TemplatedPackageList<PortagePackage>* packages );

protected:
	//! A DEPEND atom validator / package version retriever
	DependAtom* m_atom;
	//! The string of the current line's DEPEND atom, to be set by setAtomString().
	QString m_atomString;

	//! The PackageList object whose packages will be modified.
	TemplatedPackageList<PortagePackage>* m_packages;

	/**
	 * This purely virtual function is called by processLine() for every
	 * non-empty line and has to extract the DEPEND atom string from there
	 * and store it in the 'm_atomString' member variable.
	 *
	 * If the matching versions should not be processed (like when it's a
	 * comment, or if you just need the line string) this function can return
	 * false to tell processLine() to continue to the next line instead of
	 * calling processVersion() for matching versions.
	 *
	 * If there is additional information inside the line (like keywords)
	 * you might want to save them into a member variable, so that you can
	 * access it when process() is called.
	 *
	 * Further, you can assume that the 'm_atom' member is a valid
	 * DependAtom object, so use it if you need to.
	 */
	virtual bool setAtomString( const QString& line ) = 0;

	/**
	 * This purely virtual function is called for every package version
	 * in the tree that matches the DEPEND atom of the currently scanned
	 * line in the file. In most cases, you will change some version
	 * property here (like (un)hardmasking the version, for example).
	 *
	 * Further, you can assume that 'm_atom' is a valid DependAtom object
	 * which is containing info about the current line's atom.
	 */
	virtual void processVersion( PortagePackageVersion* version ) = 0;

private:
	bool check();
	bool init();
	void processLine( const QString& line );
	JobResult finish();
};

}

#endif // FLIBPAKTILEATOMLOADERBASE_H
