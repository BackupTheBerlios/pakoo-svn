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

#include "portageloaderbase.h"

/**
 * Initialize this object.
 */
PortageLoaderBase::PortageLoaderBase()
{
	stop = false;
	working = false;
	receiver = NULL;
}


/**
 * Set this->stop to false and this->working to true.
 * Should be called at the beginning of each member function that is in danger
 * of multithreading. Functions that use initProcessing should really make
 * sure to call finishProcessing() when the function exits.
 *
 * When using initProcessing() and finishProcessing(), a function can check
 * on another running thread by looking at the 'working' member variable.
 */
void PortageLoaderBase::initProcessing()
{
	stop = false;
	working = true;
}

/**
 * Reset the stop, working and receiver member variables.
 * Should be called before exiting functions that have used initProcessing().
 */
void PortageLoaderBase::finishProcessing()
{
	stop = false;
	working = false;
	receiver = NULL;
}

/**
 * Determine if a thread belonging to this object is already running.
 * Just calls QThread::running(). This function only returns true if
 * the thread has been started (by object->start*()), but doesn't
 * return true if the main thread is currently running one of its
 * member functions.
 */
bool PortageLoaderBase::running()
{
	return QThread::running();
}

/**
 * Wait for the thread to end. Just calls QThread::wait().
 */
bool PortageLoaderBase::wait()
{
	return QThread::wait();
}

/**
 * Tell the running thread to abort loading or saving. It will soon stop,
 * and the completion event's "error" member is then set to
 * PortageLoaderBase::AbortedError.
 */
void PortageLoaderBase::abort()
{
	stop = true;
}
