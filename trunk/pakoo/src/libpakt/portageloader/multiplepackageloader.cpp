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

#include "multiplepackageloader.h"

#include "../core/packagelist.h"
#include "packageloader.h"

#include <klocale.h>
#include <kdebug.h>


namespace libpakt {

/**
 * Empty constructor. The setPackageLoader() and setPackageList() member
 * functions have yet to be called.
 */
MultiplePackageLoader::MultiplePackageLoader( PackageLoader* loader )
: ThreadedJob()
{
	setPackageLoader( loader );
	m_packages = NULL;
	m_autoDeleteLoader = false;
}

MultiplePackageLoader::~ MultiplePackageLoader( )
{
	if( m_autoDeleteLoader == true && m_loader != NULL ) {
		m_loader->deleteLater();
	}
}


/**
 * Retrieve the PackageLoader that is assigned to this object.
 */
PackageLoader* MultiplePackageLoader::packageLoader()
{
	return m_loader;
}

/**
 * This object was given a pointer to a PackageLoader object which
 * does the actual work when loading package details.
 * Calling this function with 'true' means that this PackageLoader
 * object will automatically be deleted when this MultiplePackageLoader
 * is destroyed.
 *
 * By default, auto-delete is turned off (autoDelete == false).
 */
void MultiplePackageLoader::setAutoDeletePackageLoader( bool autoDelete )
{
	m_autoDeleteLoader = autoDelete;
}

/**
 * Set the PackageLoader that will be used for retrieving
 * the actual package info.
 */
void MultiplePackageLoader::setPackageLoader( PackageLoader* loader )
{
	m_loader = loader;
}

/**
 * Set the PackageList object that whose packages will be filled
 * with detailed package info.
 */
void MultiplePackageLoader::setPackageList( PackageList* packages )
{
	m_packages = packages;
}


/**
 * The function that is called when the job is executed.
 * It should be called using start() or perform() after the scanner
 * configuration has been set up, which is at least a call of
 * setPackageLoader() and setPackageList().
 */
IJob::JobResult MultiplePackageLoader::performThread()
{
	if( m_loader == NULL )
	{
		kdDebug() << i18n( "MultiplePackageLoader debug output",
			"MultiplePackageLoader::performThread(): "
			"Didn't start because the PackageLoader is NULL." )
			<< endl;
		return Failure;
	}
	if( m_packages == NULL )
	{
		kdDebug() << i18n( "MultiplePackageLoader debug output",
			"MultiplePackageLoader::performThread(): "
			"Didn't start because the PackageList is NULL." )
			<< endl;
		return Failure;
	}

	PackageList::iterator packageIteratorEnd = m_packages->end();

	// Iterate through all packages
	for( PackageList::iterator packageIterator = m_packages->begin();
	     packageIterator != packageIteratorEnd; ++packageIterator )
	{
		// scan the current package
		m_loader->setPackage( *packageIterator );
		m_loader->perform();

		if( aborting() ) {
			kdDebug() << i18n( "MultiplePackageLoader debug output",
				"MultiplePackageLoader::performThread(): "
				"Aborting on user request" )
				<< endl;
			return Failure;
		}
	}

	// make sure no one tries to access it when it might already be deleted
	m_loader->setPackage( NULL );

	// all packages have been scanned
	return Success;
}

} // namespace
