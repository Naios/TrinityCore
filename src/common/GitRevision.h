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

#ifndef __GITREVISION_H__
#define __GITREVISION_H__

#include <string>
#include "Define.h"

namespace GitRevision
{
    TRINITY_COMMON_API char const* GetHash();
    TRINITY_COMMON_API char const* GetDate();
    TRINITY_COMMON_API char const* GetBranch();
    TRINITY_COMMON_API char const* GetCMakeCommand();
    TRINITY_COMMON_API char const* GetBuildDirectory();
    TRINITY_COMMON_API char const* GetSourceDirectory();
    TRINITY_COMMON_API char const* GetMySQLExecutable();
    TRINITY_COMMON_API char const* GetFullDatabase();
    TRINITY_COMMON_API char const* GetHotfixesDatabase();
    TRINITY_COMMON_API char const* GetFullVersion();
    TRINITY_COMMON_API char const* GetCompanyNameStr();
    TRINITY_COMMON_API char const* GetLegalCopyrightStr();
    TRINITY_COMMON_API char const* GetFileVersionStr();
    TRINITY_COMMON_API char const* GetProductVersionStr();
}

#endif
