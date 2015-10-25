/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
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

#ifndef SCRIPT_RELOADER_H
#define SCRIPT_RELOADER_H

#include "Define.h"

enum DeployMode : uint32
{
    DEPLOY_MODE_NONE                = 0,
    DEPLOY_MODE_INSTALL_MODULE      = 1,
    DEPLOY_MODE_INSTALL_ALL         = 2
};

class TRINITY_GAME_API ScriptReloadMgr
{
public:
    ScriptReloadMgr() { }
    virtual ~ScriptReloadMgr() { }

    /// Initializes the ScriptReloadMgr
    virtual void Initialize() { }

    /// Called periodically to check for updates
    /// Needs to be processed in a thread safe way
    virtual void Update() { }

    /// Unloads the ScriptReloadMgr
    virtual void Unload() { }

    static ScriptReloadMgr* instance();
};

#define sScriptReloadMgr ScriptReloadMgr::instance()

#endif // SCRIPT_RELOADER_H
