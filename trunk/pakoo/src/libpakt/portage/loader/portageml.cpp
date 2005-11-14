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

#include "portageml.h"

#include "../../core/threadedjob.h"
#include "../core/portagepackageversion.h"
#include "../core/portagepackage.h"
#include "../../core/packagelist.h"
#include "../core/portagecategory.h"

#include <qfile.h>
#include <qdatetime.h>
#include <qapplication.h>

#include <kdebug.h>
#include <klocale.h>

#define FILETYPESTRING    "portageML"
#define TREEELEMENTSTRING "portagetree"
#define PACKAGEELEMENTSTRING "package"
#define VERSIONELEMENTSTRING "version"


namespace libpakt {

/**
 * Initialize this object.
 */
PortageML::PortageML() : ThreadedJob()
{
	m_packages = NULL;
	m_package = NULL;
	m_action = LoadFile;
	m_filename = QString::null;
}


/**
 * Set the PackageList object that will be filled with packages
 * (in case of loading from a file) or used as source of
 * package information (in case of saving to a file).
 */
void PortageML::setPackageList( TemplatedPackageList<PortagePackage>* packages )
{
	m_packages = packages;
}

/**
 * Set the name of the file that is used for loading and saving
 * the package list.
 */
void PortageML::setFileName( const QString& filename )
{
	m_filename = filename;
}

/**
 * Specify if you want to load from the file or save to it.
 * You can use one of the constants PortageML::LoadFile
 * or PortageML::SaveFile as argument.
 */
void PortageML::setAction( Action action )
{
	m_action = action;
}


/**
 * This function is called when a new thread is started,
 * it initiates loading or saving the package list from/to the specified file.
 */
IJob::JobResult PortageML::performThread()
{
	// Check on a NULL list, which would be bad
	if( m_packages == NULL ) {
		kdDebug() << i18n( "PortageML debug output. %1 is the filename.",
			"Didn't start loading %1 because "
			"the PackageList object has not been set" )
				.arg( m_filename )
			<< endl;
		return Failure;
	}

	bool result;

	// load or save the file
	if( m_action == LoadFile )
		result = loadFile();
	else if( m_action == SaveFile )
		result = saveFile();

	if( result == true )
		return Success;
	else
		return Failure;
}

/**
 * Load a package list from an XML file in portageML format.
 * Any previous Package objects in the PackageList will be deleted.
 *
 * @return  false if there were errors loading the file, true otherwise
 */
bool PortageML::loadFile()
{
	m_packageCountAvailable = 0;
	m_packageCountInstalled = 0;

	QDateTime startTime = QDateTime::currentDateTime();
	QFile file( m_filename );

	if( !file.open( IO_ReadOnly ) )
	{
		kdDebug() << i18n( "PortageML debug output.",
			"Aborting: Couldn't open the file %1 for reading")
				.arg( m_filename )
			<< endl;
		return false;
	}

	QDomDocument doc( FILETYPESTRING );
	if( !doc.setContent( &file ) ) {
		// file doesn't have a portageML doctype
		file.close();
		kdDebug() << i18n( "PortageML debug output.",
			"Aborting: The file %1 doesn't have an appropriate doctype" )
				.arg( m_filename )
			<< endl;
		return false;
	}
	file.close();

	QDomElement root = doc.documentElement();
	if( loadTreeElement(root) == false ) {
		return false; // it will output an error message by itself then
	}
	else {
		// Inform main thread that loading has finished
		emitFinishedLoading();

		kdDebug() << i18n( "PortageML debug output. "
		                   "%1 is the filename, %2 are, well, the seconds.",
			"Finished loading the packages from %1 in %2 seconds" )
				.arg( m_filename )
				.arg( startTime.secsTo(QDateTime::currentDateTime()) )
			<< endl;
		return true;
	}
}

/**
 * Load a package tree from a XML data.
 * This function assumes that m_packages is not NULL.
 *
 * @param element  The element containing package tree data
 * @return  false if there are errors loading the tree element, true otherwise
 */
bool PortageML::loadTreeElement( const QDomElement& element )
{
	if( element.isNull() || element.tagName() != TREEELEMENTSTRING )
	{
		kdDebug() << i18n( "PortageML debug output.",
			"Aborting: The file %1 doesn't contain "
			"an appropriate tree element" )
				.arg( m_filename )
			<< endl;
		return false;
	}

	m_packages->clear();

	QDomNodeList packageElements =
		element.elementsByTagName( PACKAGEELEMENTSTRING );

	for( uint i = 0; i < packageElements.count(); i++ )
	{
		if( aborting() ) {
			kdDebug() << i18n( "PortageML debug output.",
				"Aborting the file loading job on request" )
				<< endl;
			return false;
		}
		loadPackageElement( packageElements.item(i).toElement() );
		m_packageCountAvailable++;

		// send a progress event
		if( (m_packageCountAvailable % 500) == 0 )
			emitPackagesScanned();
	}
	return true;
}

/**
 * Load a package from a XML data and add it to the package list.
 * This function assumes that m_packages is not NULL.
 *
 * @param element  The element containing package data
 * @return  true if the package was valid and has been added, false otherwise
 */
bool PortageML::loadPackageElement( const QDomElement& element )
{
	if( element.isNull() || element.tagName() != PACKAGEELEMENTSTRING )
	{
		kdDebug() << i18n( "PortageML debug output.",
			"Error that shouldn't happen (TM): "
			"PortageML::loadPackageElement(): "
			"The function parameter is not a package element" )
			<< endl;
		return false;
	}

	if( !element.hasAttribute("category")
	    || !element.hasAttribute("name") )
	{
		kdDebug() << i18n( "PortageML debug output.",
			"Error: The package element is missing one of the "
			"'name' or 'category' attributes. "
			"Continuing with the next package element." )
			<< endl;
		return false;
	}

	PortageCategory* category = new PortageCategory;
	category->loadFromUniqueName( element.attribute("category","") );

	m_package = m_packages->package( category, element.attribute("name","") );
	m_package->clear();

	QDomNodeList versionElements =
		element.elementsByTagName( VERSIONELEMENTSTRING );

	for( uint i = 0; i < versionElements.count(); i++ ) {
		loadVersionElement( versionElements.item(i).toElement() );
	}
	return true;
}

/**
 * Load version info from XML data and add it to the current package.
 * This function assumes that m_package is not NULL.
 *
 * @param element  The element containing version info.
 * @return  true if the version was valid and has been added, false otherwise.
 */
bool PortageML::loadVersionElement( const QDomElement& element )
{
	if( element.isNull() || element.tagName() != VERSIONELEMENTSTRING )
	{
		kdDebug() << i18n( "PortageML debug output.",
			"Error that shouldn't happen (TM): "
			"PortageML::loadVersionElement(): "
			"The function parameter is not a version element" )
			<< endl;
		return false;
	}

	if( !element.hasAttribute("version") )
	{
		kdDebug() << i18n( "PortageML debug output.",
			"Error: The version element is missing the 'version' "
			"attribute. Continuing with the next version element." )
			<< endl;
		return false;
	}

	QString versionString = element.attribute( "version", "" );

	// clean up before doing anything
	m_package->removeVersion( versionString );

	PortagePackageVersion* version = m_package->version( versionString );

	if( element.hasAttribute( "installed" )
		&& element.attribute( "installed", "" ) == "true" )
	{
		version->setInstalled( true );
		m_packageCountInstalled++;
	}
	if( element.hasAttribute( "overlay" )
		&& element.attribute( "overlay", "" ) == "true" )
	{
		version->setOverlay( true );
	}
	if( element.hasAttribute("date") )
	{
		version->setDate( element.attribute("date", "") );
	}
	// Can be extended with other attributes, like description
	// or the keyword list. As I don't need that now (will I ever?)
	// I don't implement it at the moment.
	return true;
}

/**
 * Save the portage tree to an XML file in portageML format.
 *
 * @return  false if there were errors saving the file, true otherwise.
 */
bool PortageML::saveFile()
{
	QDateTime startTime = QDateTime::currentDateTime();
	QDomDocument doc( FILETYPESTRING );
	QDomElement root = this->createTreeElement( doc );

	if( root.isNull() )
		return false;

	doc.appendChild( root );

	QFile file( m_filename );
	if( !file.open( IO_WriteOnly ) )
	{
		kdDebug() << i18n( "PortageML debug output.",
			"Aborting: Couldn't open the file %1 for writing" )
				.arg( m_filename )
			<< endl;
		return false;
	}

	QTextStream ts( &file );
	ts << doc.toString();
	file.close();

	// Inform main thread that saving has finished
	emitFinishedSaving();
	kdDebug() << i18n( "PortageML debug output. "
	                   "%1 is the filename, %2 are, well, the seconds.",
		"PortageML::saveFile(): "
		"Finished saving the tree to %1 in %2 seconds" )
			.arg( m_filename )
			.arg( startTime.secsTo(QDateTime::currentDateTime()) )
		<< endl;
	return true;
}

/**
 * Create a DOM element that contains all information about a package tree
 * and its packages. This function assumes that m_packages is not NULL.
 *
 * @param doc  The node will be created using
 *             this document's createElement() function.
 * @return  The created DOM element (which can be written to an XML file,
 *          for example). A null element (element.isNull() == true)
 *          if the thread has been aborted.
 */
QDomElement PortageML::createTreeElement( QDomDocument& doc )
{
	QDomElement element = doc.createElement( TREEELEMENTSTRING );

	//QValueList<Package*> packageValues = m_packages->values();
	QDomElement packageNode;

	for( PackageList::iterator packageIterator = m_packages->begin();
	     packageIterator != m_packages->end(); packageIterator++ )
	{
		if( aborting() ) {
			// return QDomElement::null; but there is no such constant

			kdDebug() << i18n( "PortageML debug output.",
			                   "Aborting the file saving job on request" )
				<< endl;
			return element.toDocument().toElement();
		}

		Package* pkg = *packageIterator;
		m_package = (PortagePackage*) pkg;
		packageNode = createPackageElement( doc );
		element.appendChild( packageNode );
	}
	return element;
}

/**
 * Create a DOM element that contains all information about a package
 * and its versions. This function assumes that m_package is not NULL.
 *
 * @param doc  The node will be created using
 *             this document's createElement() function.
 * @return  The created DOM element (which can be written to an XML file,
 *          for example).
 */
QDomElement PortageML::createPackageElement( QDomDocument& doc )
{
	QDomElement element = doc.createElement( PACKAGEELEMENTSTRING );

	QDomAttr attr = doc.createAttribute( "category" );
	attr.setValue( m_package->category()->uniqueName() );
	element.setAttributeNode( attr );

	attr = doc.createAttribute( "name" );
	attr.setValue( m_package->name() );
	element.setAttributeNode( attr );

	QDomElement versionElement;

	for( PortagePackage::versioniterator versionIterator = m_package->versionBegin();
	     versionIterator != m_package->versionEnd(); versionIterator++ )
	{
		PortagePackageVersion* version =
			(PortagePackageVersion*) (*versionIterator);

		versionElement = doc.createElement( VERSIONELEMENTSTRING );

		attr = doc.createAttribute( "version" );
		attr.setValue( version->version() );
		versionElement.setAttributeNode( attr );

		if( version->isInstalled() ) {
			attr = doc.createAttribute( "installed" );
			attr.setValue( "true" );
			versionElement.setAttributeNode( attr );
		}

		if( version->isOverlay() ) {
			attr = doc.createAttribute( "overlay" );
			attr.setValue( "true" );
			versionElement.setAttributeNode( attr );
		}

		if( version->date().isEmpty() == false ) {
			attr = doc.createAttribute( "date" );
			attr.setValue( version->date() );
			versionElement.setAttributeNode( attr );
		}
		// Can be extended with other attributes, like description
		// or the keyword list. As I don't need that now (will I ever?)
		// I don't implement it at the moment.

		element.appendChild( versionElement );
	}

	return element;
}


/**
 * From within the thread, emit a finishedLoading() signal to the main thread.
 */
void PortageML::emitFinishedLoading()
{
	FinishedFileEvent* event = new FinishedFileEvent();
	event->action = LoadFile;
	event->packages = m_packages;
	event->filename = m_filename;
	QApplication::postEvent( this, event );
}

/**
 * From within the thread, emit a finishedSaving() signal to the main thread.
 */
void PortageML::emitFinishedSaving()
{
	FinishedFileEvent* event = new FinishedFileEvent();
	event->action = SaveFile;
	event->packages = m_packages;
	event->filename = m_filename;
	QApplication::postEvent( this, event );
}

/**
 * From within the thread, emit a packagesScanned() signal to the main thread.
 */
void PortageML::emitPackagesScanned()
{
	PackagesScannedEvent* event  = new PackagesScannedEvent();
	event->packageCountAvailable = m_packageCountAvailable;
	event->packageCountInstalled = m_packageCountInstalled;
	QApplication::postEvent( this, event );
}

/**
 * Translates QCustomEvents into signals. This function is called from Qt
 * in the main thread, which guarantees safety for emitting signals.
 */
void PortageML::customEvent( QCustomEvent* event )
{
	switch( event->type() )
	{
	case (int) PackagesScannedEventType:
		emit packagesScanned(
			((PackagesScannedEvent*)event)->packageCountAvailable,
			((PackagesScannedEvent*)event)->packageCountInstalled
		);
		break;

	case (int) FinishedFileEventType:
		if( ((FinishedFileEvent*)event)->action == LoadFile )
			emit finishedLoading( m_packages, m_filename );
		else if( ((FinishedFileEvent*)event)->action == SaveFile )
			emit finishedSaving( m_packages, m_filename );
		break;

	default:
		ThreadedJob::customEvent( event );
		break;
	}
}

} // namespace
