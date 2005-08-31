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

#ifndef PAKOOACTIONVIEW_H
#define PAKOOACTIONVIEW_H

#include <qwidget.h>

/**
 * @short A class that manages (contains, shows/hides, ...) widgets of Pakoo's action view.
 *
 * The ActionView is a collection of widgets that are dynamically added
 * and removed. Each one of them represents a specific action, such as
 * syncing the package repository, installing a package, or similar.
 */
class ActionView : public QWidget
{
Q_OBJECT

public:
    ActionView( QWidget* parent = 0, const char* name = 0 );
};

#endif // PAKOOACTIONVIEW_H
