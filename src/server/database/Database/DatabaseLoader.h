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
#include "DBUpdater.h"

#include <stack>
#include <functional>

// A helper class to initiate all database worker pools,
// handles updating, delays preparing of statements and cleans up on failure.
class TRINITY_DATABASE_API DatabaseLoader
{
public:
    DatabaseLoader(std::string const& logger, uint32 const defaultUpdateMask);

    // Register a database to the loader (lazy implemented)
    template <class T>
    DatabaseLoader& AddDatabase(DatabaseWorkerPool<T>& pool, std::string const& name)
    {
        bool const updatesEnabledForThis = DBUpdater<T>::IsEnabled(_updateFlags);

        _open.push(std::make_pair([this, name, updatesEnabledForThis, &pool]() -> bool
        {
            std::string const dbString = sConfigMgr->GetStringDefault(name + "DatabaseInfo", "");
            if (dbString.empty())
            {
                TC_LOG_ERROR(_logger.c_str(), "Database %s not specified in configuration file!", name.c_str());
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
            if (uint32 error = pool.Open())
            {
                // Database does not exist
                if ((error == ER_BAD_DB_ERROR) && updatesEnabledForThis && _autoSetup)
                {
                    // Try to create the database and connect again if auto setup is enabled
                    if (DBUpdater<T>::Create(pool) && (!pool.Open()))
                        error = 0;
                }

                // If the error wasn't handled quit
                if (error)
                {
                    TC_LOG_ERROR("sql.driver", "\nDatabasePool %s NOT opened. There were errors opening the MySQL connections. Check your SQLDriverLogFile "
                        "for specific errors. Read wiki at http://collab.kpsn.org/display/tc/TrinityCore+Home", name.c_str());

                    return false;
                }
            }
            return true;
        },
        [&pool]()
        {
            pool.Close();
        }));

        // Populate and update only if updates are enabled for this pool
        if (updatesEnabledForThis)
        {
            _populate.push([this, name, &pool]() -> bool
            {
                if (!DBUpdater<T>::Populate(pool))
                {
                    TC_LOG_ERROR(_logger.c_str(), "Could not populate the %s database, see log for details.", name.c_str());
                    return false;
                }
                return true;
            });

            _update.push([this, name, &pool]() -> bool
            {
                if (!DBUpdater<T>::Update(pool))
                {
                    TC_LOG_ERROR(_logger.c_str(), "Could not update the %s database, see log for details.", name.c_str());
                    return false;
                }
                return true;
            });
        }

        _prepare.push([this, name, &pool]() -> bool
        {
            if (!pool.PrepareStatements())
            {
                TC_LOG_ERROR(_logger.c_str(), "Could not prepare statements of the %s database, see log for details.", name.c_str());
                return false;
            }
            return true;
        });

        return *this;
    }

    // Load all databases
    bool Load();

    enum DatabaseTypeFlags
    {
        DATABASE_NONE       = 0,

        DATABASE_LOGIN      = 1,
        DATABASE_CHARACTER  = 2,
        DATABASE_WORLD      = 4,
        DATABASE_HOTFIX     = 8,

        DATABASE_MASK_ALL   = DATABASE_LOGIN | DATABASE_CHARACTER | DATABASE_WORLD | DATABASE_HOTFIX
    };

private:
    bool OpenDatabases();
    bool PopulateDatabases();
    bool UpdateDatabases();
    bool PrepareStatements();

    using Predicate = std::function<bool()>;

    static bool Process(std::stack<Predicate>& stack);

    std::string const _logger;
    bool const _autoSetup;
    uint32 const _updateFlags;

    std::stack<std::pair<Predicate, std::function<void()>>> _open;
    std::stack<std::function<void()>> _close;
    std::stack<Predicate> _populate, _update, _prepare;
};

#endif // DatabaseLoader_h__
