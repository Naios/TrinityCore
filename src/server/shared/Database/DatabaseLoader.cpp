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

#include "DatabaseLoader.h"

template <class T>
DatabaseLoader& DatabaseLoader::AddDatabase<T>(DatabaseWorkerPool<T>& pool, std::string const& name)
{
    _load.push(std::make_pair(
    [&]()
    {
        std::string const dbString = sConfigMgr->GetStringDefault(name + "DatabaseInfo", "");
        if (dbString.empty())
        {
            TC_LOG_ERROR(_logger.c_str(), "%s database not specified in configuration file", name.c_str());
            return false;
        }

        uint8 const asyncThreads = uint8(sConfigMgr->GetIntDefault(name + "Database.WorkerThreads", 1));
        if (asyncThreads < 1 || asyncThreads > 32)
        {
            TC_LOG_ERROR(_logger.c_str(), "%s database: invalid number of worker threads specified. "
                "Please pick a value between 1 and 32.", name.c_str());
            return false;
        }

        uint8 const synchThreads = uint8(sConfigMgr->GetIntDefault(name + "Database.SynchThreads", 1));
        pool.SetConnectionInfo(dbString, asyncThreads, synchThreads);
        if (!pool.Open())
        {
            // Try to create the database and connect again if auto setup is enabled
            if (!(autoSetup && DBUpdater<T>::Create(pool) && pool.Open()))
            {
                TC_LOG_ERROR(_logger.c_str(), "Cannot connect to %s database.", name.c_str());
                return false;
            }
        }
    },
    [&]()
    {
        pool.Close();
    }));

    _populate.push([&]()
    {
        if (!DBUpdater<T>::Populate(pool))
        {
            TC_LOG_ERROR(_logger.c_str(), "Could not populate the %s database, see log for details.", name.c_str());
            return false;
        }
        return true;
    });

    _update.push([&]()
    {
        if (!DBUpdater<T>::Update(pool))
        {
            TC_LOG_ERROR(_logger.c_str(), "Could not update the %s database, see log for details.", name.c_str());
            return false;
        }
        return true;
    });

    _prepare.push([&]()
    {
        if (!DBUpdater<T>::PrepareStatements(pool))
        {
            TC_LOG_ERROR(_logger.c_str(), "Could not prepare statements of the %s database, see log for details.", name.c_str());
            return false;
        }
        return true;
    });

    return *this;
}

bool DatabaseLoader::LoadDatabases()
{
    while (!_load.empty())
    {
        std::pair<Call, std::function<void()>> const load = _load.top();
        if (load.first())
            _unload.push(load.second);
        else
        {
            // Close all loaded databases
            while (!_unload.empty())
            {
                _unload.top()();
                _unload.pop();
            }
            return false;
        }

        _load.pop();
    }
}

bool DatabaseLoader::Process(std::stack<Call>& stack)
{
    while (!stack.empty())
    {
        if (!stack.top()())
            return false;

        stack.pop();
    }
}

bool DatabaseLoader::PopulateDatabases()
{
    return Process(_populate);
}

bool DatabaseLoader::UpdateDatabases()
{
    return Process(_update);
}

bool DatabaseLoader::PrepareStatements()
{
    return Process(_prepare);
}
