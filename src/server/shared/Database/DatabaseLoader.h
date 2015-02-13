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

#ifndef DatabaseLoader_h__
#define DatabaseLoader_h__

#include "DatabaseWorkerPool.h"
#include "DatabaseEnv.h"
#include "Config.h"

#include <stack>
#include <functional>

class DatabaseLoader
{
    using Call = std::function <bool()> ;

public:
    DatabaseLoader(std::string const& logger, bool const autoSetup) :
        _logger(logger), _autoSetup(autoSetup) {}

    template <class T>
    DatabaseLoader& AddDatabase(DatabaseWorkerPool<T>& pool, std::string const& name);

    bool OpenDatabases();

    bool PopulateDatabases();

    bool UpdateDatabases();

    bool PrepareStatements();
    
private:
    static bool Process(std::stack<Call>& stack);

    bool const _autoSetup;
    std::string const _logger;
    std::stack<std::pair<Call, std::function<void()>>> _open;
    std::stack<std::function<void()>> _close;
    std::stack<Call> _populate, _update, _prepare;
};

#endif // DatabaseLoader_h__
