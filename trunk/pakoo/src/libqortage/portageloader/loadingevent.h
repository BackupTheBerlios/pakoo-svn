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

#ifndef PORTAGEEVENT_H
#define PORTAGEEVENT_H

#include "../core/portagetree.h"
#include "portageloaderbase.h"
#include "portagetreescanner.h"
#include "packagescanner.h"

#include <qevent.h>

enum PortageEventType {
	LoadingTreeProgress = QEvent::User + 14325,
	LoadingTreeComplete = QEvent::User + 14326,
	LoadingTreePartiallyComplete = QEvent::User + 14327,
	SavingTreeComplete = QEvent::User + 14328,
	LoadingPackageComplete = QEvent::User + 14329
};

class LoadingTreeEvent : public QCustomEvent
{
public:
	LoadingTreeEvent( int type );

	enum Method {
		LoadFile,
		ScanPortageTree
	};
};


class LoadingTreeProgressEvent : public LoadingTreeEvent
{
public:
	LoadingTreeProgressEvent();

	//! Number of packages that have been found so far.
	int packageCount;
	//! The tree that is currently searched (either Mainline, Overlay or Installed).
	PortageTree::Trees searchedTree;
	//! Either LoadingTreeEvent::LoadFile or LoadingTreeEvent::ScanPortageTree.
	LoadingTreeEvent::Method method;
};


class LoadingTreeCompleteEvent : public LoadingTreeEvent
{
public:
	LoadingTreeCompleteEvent();

	//! Number of packages in the whole portage tree.
	int packageCount;
	//! Number of installed packages in the whole portage tree.
	int packageCountInstalled;
	//! Time that has been needed to search all trees.
	int secondsElapsed;
	//! Either LoadingTreeEvent::LoadFile or LoadingTreeEvent::ScanPortageTree.
	LoadingTreeEvent::Method method;

	/**
	 * The return value, depending on the loading method.
	 * (PortageConstants::NoError if everything went smooth.)
	 */
	PortageLoaderBase::Error error;
};


class LoadingTreePartiallyCompleteEvent : public LoadingTreeEvent
{
public:
	LoadingTreePartiallyCompleteEvent();

	//! Number of packages in the whole portage tree.
	int packageCount;
	//! Time that has been needed to search this specific tree.
	int secondsElapsed;
	//! The tree that has been searched (either Mainline, Overlay or Installed).
	PortageTree::Trees searchedTree;
	//! Either LoadingTreeEvent::LoadFile or LoadingTreeEvent::ScanPortageTree.
	LoadingTreeEvent::Method method;
};


class SavingTreeCompleteEvent : public QCustomEvent
{
public:
	SavingTreeCompleteEvent();

	//! Time that has been needed to search all trees.
	int secondsElapsed;

	/**
	 * The return value, depending on the loading method.
	 * (PortageLoaderBase::NoError if everything went smooth.)
	 */
	PortageLoaderBase::Error error;
};


class LoadingPackageCompleteEvent : public QCustomEvent
{
public:
	LoadingPackageCompleteEvent();

	//! Specifies if a single package or a whole category has been searched.
	PackageScanner::Action action;
	/**
	 * The return value of the loading attempt.
	 * (PortageLoaderBase::NoError if everything went smooth.)
	 */
	PortageLoaderBase::Error error;
	/**
	 * A pointer to the package that has been scanned.
	 * NULL if action == PackageScanner::scanCategory.
	 */
	Package* package;
	//! A pointer to the PackageScanner object that has done the work.
	PackageScanner* packageScanner;
};

#endif // PORTAGEEVENT_H
