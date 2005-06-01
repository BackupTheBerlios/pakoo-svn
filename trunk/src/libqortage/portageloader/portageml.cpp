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

#include "../core/packageversion.h"
#include "../core/package.h"
#include "../core/portagetree.h"
#include "loadingevent.h"

#include <qfile.h>
#include <qdatetime.h>
#include <qapplication.h>

#define FILETYPESTRING    "portageML"
#define TREEELEMENTSTRING "portagetree"
#define PACKAGEELEMENTSTRING "package"
#define VERSIONELEMENTSTRING "version"


/**
 * Initialize this object.
 */
PortageML::PortageML()
{
	tree = NULL;
	package = NULL;
}


/**
 * Load a portage tree from an XML file in portageML format.
 * Any previous Package objects in the PortageTree will be deleted.
 *
 * @param portageTree  The PortageTree object that will be filled with packages.
 * @param filename     The file that contains the stored tree structure.
 *
 * @return  PortageLoaderBase::NoError is the package list was successfully loaded.
 *          PortageLoaderBase::OpenFileError if there was an error opening the XML file.
 *          PortageLoaderBase::FileTypeError if the file doesn't have a portageML
 *          doctype header. PortageLoaderBase::RootElementError
 *          if the root element is named anything but "portagetree".
 *          PortageLoaderBase::NullTreeError if the given tree is NULL.
 *          PortageLoaderBase::AbortedError if the thread has been aborted.
 *          PortageLoaderBase::AlreadyRunningError if any thread is
 *          currently loading or saving (but there may only be one at a time).
 */
PortageLoaderBase::Error PortageML::loadFile( PortageTree* portageTree,
                                                   const QString& filename )
{
	this->packageCount = 0;
	this->packageCountInstalled = 0;

	if( this->working == true )
		return AlreadyRunningError;
	else
		initProcessing();

	// Check on a NULL tree, which would be bad
	if( portageTree == NULL ) {
		finishProcessing();
		return NullTreeError;
	}
	tree = portageTree;

	QDateTime startTime = QDateTime::currentDateTime();
	QFile file( filename );

	if( !file.open( IO_ReadOnly ) ) {
		finishProcessing();
		return OpenFileError;
	}

	QDomDocument doc( FILETYPESTRING );
	if( !doc.setContent( &file ) ) {
		// file doesn't have a portageML doctype
		file.close();
		finishProcessing();
		return FileTypeError;
	}
	file.close();

	QDomElement root = doc.documentElement();
	PortageLoaderBase::Error result = loadTreeElement(root);

	finishProcessing();
	return result;
}

/**
 * Load a portage tree from a XML data.
 *
 * @param element  The element containing portage tree data.
 * @return  PortageLoaderBase::NoError is the package list was
 *          successfully loaded.
 *          PortageLoaderBase::NullTreeError if the given tree is NULL.
 *          PortageLoaderBase::AbortedError if the thread is being aborted.
 */
PortageLoaderBase::Error PortageML::loadTreeElement( const QDomElement& element )
{
	if( element.isNull() || element.tagName() != TREEELEMENTSTRING ) {
		return PortageLoaderBase::NullTreeError;
	}

	tree->clear();

	QDomNodeList packageElements =
		element.elementsByTagName( PACKAGEELEMENTSTRING );

	for( uint i = 0; i < packageElements.count(); i++ )
	{
		if( this->stop == true ) {
			return AbortedError;
		}
		loadPackageElement( packageElements.item(i).toElement() );
		packageCount++;

		// don't post events if not running as thread
		if( (packageCount % 500) == 0 && this->running() == true )
		{
			// send a progress event
			LoadingTreeProgressEvent *event = new LoadingTreeProgressEvent();
			event->packageCount = packageCount;
			event->method = LoadingTreeEvent::LoadFile;
			event->searchedTree = PortageTree::Mainline;
			QApplication::postEvent( receiver, event );
		}
	}
	return NoError;
}

/**
 * Load a package from a XML data and add it to the tree object.
 * This function assumes that this->tree is not NULL.
 *
 * @param element  The element containing package data.
 * @return  true if the package was valid and has been added, false otherwise.
 */
bool PortageML::loadPackageElement( const QDomElement& element )
{
	if( element.isNull() || element.tagName() != PACKAGEELEMENTSTRING )
		return false;

	if( ! ( element.hasAttribute("category")
	        && element.hasAttribute("subcategory")
	        && element.hasAttribute("name")
	      ) )
	{
		return false;
	}

	package = tree->package( element.attribute( "category", "" ),
								element.attribute( "subcategory", "" ),
								element.attribute( "name", "" ) );
	package->clear();

	QDomNodeList versionElements =
		element.elementsByTagName( VERSIONELEMENTSTRING );

	for( uint i = 0; i < versionElements.count(); i++ ) {
		loadVersionElement( versionElements.item(i).toElement() );
	}
	return true;
}

/**
 * Load version info from XML data and add it to the current package.
 * This function assumes that this->package is not NULL.
 *
 * @param element  The element containing version info.
 * @return  true if the version was valid and has been added, false otherwise.
 */
bool PortageML::loadVersionElement( const QDomElement& element )
{
	if( element.isNull() || element.tagName() != VERSIONELEMENTSTRING )
		return false;

	if( !element.hasAttribute("version") )
		return false;

	QString versionString = element.attribute( "version", "" );

	package->removeVersion( versionString );  // clean up before doing anything
	PackageVersion* version = package->version( versionString );

	if( element.hasAttribute( "installed" )
		&& element.attribute( "installed", "" ) == "true" )
	{
		version->installed = true;
		packageCountInstalled++;
	}
	if( element.hasAttribute( "overlay" )
		&& element.attribute( "overlay", "" ) == "true" )
	{
		version->overlay = true;
	}
	if( element.hasAttribute("date") )
	{
		version->date = element.attribute("date", "");
	}
	// Can be extended with other attributes, like description
	// or the keyword list. As I don't need that now (will I ever?)
	// I don't implement it at the moment.
	return true;
}

/**
 * Save the portage tree to an XML file in portageML format.
 *
 * @param portageTree  The PortageTree object whose packages will be written.
 * @param filename     The file that contains the stored tree structure.
 *
 * @return  PortageLoaderBase::NoError is the package list was
 *          successfully saved. PortageLoaderBase::OpenFileError if
 *          there was an error opening the XML file.
 *          PortageLoaderBase::NullTreeError if the given tree is NULL.
 *          PortageLoaderBase::AbortedError if the thread has been aborted.
 *          PortageLoaderBase::AlreadyRunningError if any thread is
 *          currently loading or saving (but there may only be one at a time).
 */
PortageLoaderBase::Error PortageML::saveFile( PortageTree* portageTree,
                                                   const QString& filename )
{
	if( working == true )
		return AlreadyRunningError;
	else
		initProcessing();

	// Check on a NULL tree, which would be bad
	if( portageTree == NULL ) {
		finishProcessing();
		return NullTreeError;
	}
	tree = portageTree;

	QDomDocument doc( FILETYPESTRING );
	QDomElement root = this->createTreeElement( doc );
	if( root.isNull() ) {
		finishProcessing();
		return AbortedError;
	}

	doc.appendChild( root );

	QFile file( filename );
	if( !file.open( IO_WriteOnly ) ) {
		finishProcessing();
		return OpenFileError;
	}

	QTextStream ts( &file );
	ts << doc.toString();
	file.close();

	finishProcessing();
	return NoError;
}

/**
 * Create a DOM element that contains all information about a portage tree
 * and its packages. This function assumes that this->tree is not NULL.
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

	PackageMap* packages = tree->packageMap();
	PackageMap::iterator packageIterator;
	QDomElement packageNode;
	for( packageIterator = packages->begin();
	     packageIterator != packages->end(); packageIterator++ )
	{
		if( this->stop == true ) {
			// return QDomElement::null; but there is no such constant
			return element.toDocument().toElement();
		}

		package = &(*packageIterator);
		packageNode = createPackageElement( doc );
		element.appendChild( packageNode );
	}
	return element;
}

/**
 * Create a DOM element that contains all information about a package
 * and its versions. This function assumes that this->package is not NULL.
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
	attr.setValue( package->category );
	element.setAttributeNode( attr );

	attr = doc.createAttribute( "subcategory" );
	attr.setValue( package->subcategory );
	element.setAttributeNode( attr );

	attr = doc.createAttribute( "name" );
	attr.setValue( package->name );
	element.setAttributeNode( attr );

	PackageVersionMap* versions = package->versionMap();
	PackageVersionMap::iterator versionIterator;
	QDomElement versionElement;

	for( versionIterator = versions->begin();
	     versionIterator != versions->end(); versionIterator++ )
	{
		versionElement = doc.createElement( VERSIONELEMENTSTRING );

		attr = doc.createAttribute( "version" );
		attr.setValue( (*versionIterator).version );
		versionElement.setAttributeNode( attr );

		if( (*versionIterator).installed == true ) {
			attr = doc.createAttribute( "installed" );
			attr.setValue( "true" );
			versionElement.setAttributeNode( attr );
		}

		if( (*versionIterator).overlay == true ) {
			attr = doc.createAttribute( "overlay" );
			attr.setValue( "true" );
			versionElement.setAttributeNode( attr );
		}

		if( (*versionIterator).date.isEmpty() == false ) {
			attr = doc.createAttribute( "date" );
			attr.setValue( (*versionIterator).date );
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
 * Start a thread that will be loading a portage tree from an XML file
 * in portageML format. While loading packages, the thread will post
 * LoadingTreeProgressEvent objects to the receiver, and when loading is
 * done, a LoadingTreeCompleteEvent will be posted and the thread exits.
 * The thread will not be started if it's already running.
 *
 * @param receiver     A QObject that receives loading status events.
 * @param portageTree  A pointer to an existing PortageTree object.
 *                     This tree will be cleared and filled with the
 *                     packages that are found in the XML file.
 * @param filename     The path of the file that shall be loaded.
 *
 * @returns  true if the thread was started, false if it's already running.
 */
bool PortageML::startLoadingFile( QObject* receiver,
                                  PortageTree* portageTree, const QString& filename )
{
	if( this->running() == true )
		return false;

	this->receiver = receiver;
	this->tree = portageTree;
	this->filename = filename;
	this->action = LoadFile;
	this->start();
	return true;
}

/**
 * Start a thread that will be saving a portage tree from an XML file
 * in portageML format. When saving is done, a SavingTreeCompleteEvent
 * will be posted and the thread then exits. The thread will not be
 * started if it's already running.
 *
 * @param receiver     A QObject that receives loading status events.
 * @param portageTree  A pointer to an existing PortageTree object.
 *                     This tree will be cleared and filled with the
 *                     packages that are found in the XML file.
 * @param filename     The path of the file that shall be loaded.
 *
 * @returns  true if the thread was started, false if it's already running.
 */
bool PortageML::startSavingFile( QObject* receiver,
                                 PortageTree* portageTree, const QString& filename )
{
	if( this->running() == true )
		return false;

	this->receiver = receiver;
	this->tree = portageTree;
	this->filename = filename;
	this->action = SaveFile;
	this->start();
	return true;
}

/**
 * The function that is called when a new thread is started.
 * It can not be called directly with portageMLObject->start().
 * That's not necessary, because PortageML has convenient
 * member functions to start the thread and set up the configuration
 * for it (these are startLoadingFile() and startSavingFile()).
 */
void PortageML::run()
{
	PortageLoaderBase::Error result;
	QDateTime startTime = QDateTime::currentDateTime();
	QObject* receiver = this->receiver;

	// load or save the file
	if( this->action == LoadFile ) {
		result = this->loadFile( this->tree, this->filename );

		// Inform main thread that loading has finished
		LoadingTreeCompleteEvent *event = new LoadingTreeCompleteEvent();
		event->error = result;
		event->packageCount = tree->packageCount();
		event->packageCountInstalled = packageCountInstalled;
		event->method = LoadingTreeEvent::LoadFile;
		event->secondsElapsed = startTime.secsTo( QDateTime::currentDateTime() );
		QApplication::postEvent( receiver, event );
	}
	else if( this->action == SaveFile ) {
		result = this->saveFile( this->tree, this->filename );

		// Inform main thread that saving has finished
		SavingTreeCompleteEvent *event = new SavingTreeCompleteEvent();
		event->error = result;
		event->secondsElapsed = startTime.secsTo( QDateTime::currentDateTime() );
		QApplication::postEvent( receiver, event );
	}
}
