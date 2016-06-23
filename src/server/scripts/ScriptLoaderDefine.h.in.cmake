/*
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

// This file was created automatically from your script configuration!
// Use CMake to reconfigure this file, never change it on your own!

#ifndef SCRIPT_LOADER_DEFINE_H
#define SCRIPT_LOADER_DEFINE_H

#cmakedefine TRINITY_IS_DYNAMIC_SCRIPTLOADER

#ifdef TRINITY_IS_DYNAMIC_SCRIPTLOADER
  #define TRINITY_DYNAMIC_SCRIPT_MODULE_NAME @TRINITY_DYNAMIC_SCRIPT_MODULE_NAME@
#endif // TRINITY_IS_DYNAMIC_SCRIPTLOADER

#define TRINITY_SCRIPT_LOADER_DECL         @TRINITY_SCRIPT_LOADER_DECL@
#define TRINITY_SCRIPT_LOADER_INVOKE       @TRINITY_SCRIPT_LOADER_INVOKE@

#endif // SCRIPT_LOADER_DEFINE_H
