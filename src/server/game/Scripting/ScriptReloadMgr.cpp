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

#include "ScriptReloadMgr.h"
#include "Errors.h"

#ifndef TRINITY_API_USE_DYNAMIC_LINKING

// This method should never be called
std::shared_ptr<ModuleReference>
    ScriptReloadMgr::AcquireModuleReferenceOfContext(std::string const& context)
{
    WPAbort();
}

// Returns the empty implemented ScriptReloadMgr
ScriptReloadMgr* ScriptReloadMgr::instance()
{
    static ScriptReloadMgr instance;
    return &instance;
}

#else

#include <algorithm>
#include <regex>
#include <vector>
#include <future>
#include <memory>
#include <fstream>
#include <type_traits>
#include <unordered_set>
#include <unordered_map>

#include <boost/filesystem.hpp>
#include <boost/system/system_error.hpp>

#include "efsw/efsw.hpp"

#include "Log.h"
#include "Config.h"
#include "BuiltInConfig.h"
#include "ScriptMgr.h"
#include "StartProcess.h"
#include "MPSCQueue.h"
#include "GitRevision.h"

namespace fs = boost::filesystem;

#ifdef _WIN32
    #include <windows.h>
#else // Posix
    #include <dlfcn.h>
#endif

// Promote the sScriptReloadMgr to a HotSwapScriptReloadMgr
// in this compilation unit.
#undef sScriptReloadMgr
#define sScriptReloadMgr static_cast<HotSwapScriptReloadMgr*>(ScriptReloadMgr::instance())

// Returns "" on Windows and "lib" on posix.
static char const* GetSharedLibraryPrefix()
{
#ifdef _WIN32
    return "";
#else // Posix
    return "lib";
#endif
}

// Returns "dll" on Windows and "so" on posix.
static char const* GetSharedLibraryExtension()
{
#ifdef _WIN32
    return "dll";
#else // Posix
    return "so";
#endif
}

#ifdef _WIN32
typedef HMODULE HandleType;
#else // Posix
typedef void* HandleType;
#endif

class SharedLibraryUnloader
{
public:
    SharedLibraryUnloader()
        : _path() { }
    explicit SharedLibraryUnloader(fs::path const& path)
        : _path(path) { }

    void operator() (HandleType handle) const
    {
        // Unload the associated shared library.
#ifdef _WIN32
        bool success = (FreeLibrary(handle) != 0);
#else // Posix
        bool success = (dlclose(handle) == 0);
#endif

        if (!success)
        {
            TC_LOG_ERROR("scripts.hotswap", "Failed to close the shared library \"%s\".",
                _path.generic_string().c_str());

            return;
        }

        boost::system::error_code error;
        if (fs::remove(_path, error))
        {
            TC_LOG_TRACE("scripts.hotswap", "Lazy unloaded and deleted the shared library \"%s\".",
                _path.generic_string().c_str());
        }
        else
        {
            TC_LOG_ERROR("scripts.hotswap", "Failed to delete the shared library \"%s\".",
                _path.generic_string().c_str());
        }

        (void)error;
    }

private:
    fs::path _path;
};

typedef std::unique_ptr<typename std::remove_pointer<HandleType>::type, SharedLibraryUnloader> HandleHolder;

typedef char const* (*GetScriptModuleRevisionHashType)();
typedef void (*AddScriptsType)();
typedef char const* (*GetScriptModuleType)();
typedef char const* (*GetBuildDirectiveType)();

class ScriptModule
    : public ModuleReference
{
public:
    explicit ScriptModule(HandleHolder handle, GetScriptModuleRevisionHashType getScriptModuleRevisionHash,
                 AddScriptsType addScripts, GetScriptModuleType getScriptModule,
                 GetBuildDirectiveType getBuildDirective, fs::path const& path)
        : _handle(std::forward<HandleHolder>(handle)), _getScriptModuleRevisionHash(getScriptModuleRevisionHash),
          _addScripts(addScripts), _getScriptModule(getScriptModule),
          _getBuildDirective(getBuildDirective), _path(path) { }

    ScriptModule(ScriptModule const&) = delete;
    ScriptModule(ScriptModule&& right) = delete;

    ScriptModule& operator= (ScriptModule const&) = delete;
    ScriptModule& operator= (ScriptModule&& right) = delete;

    static Optional<std::shared_ptr<ScriptModule>> CreateFromPath(fs::path const& path);

    char const* GetScriptModuleRevisionHash() const override
    {
        return _getScriptModuleRevisionHash();
    }

    void AddScripts() const
    {
        return _addScripts();
    }

    char const* GetScriptModule() const override
    {
        return _getScriptModule();
    }

    char const* GetBuildDirective() const
    {
        return _getBuildDirective();
    }

    fs::path const& GetModulePath() const override
    {
        return _path;
    }

private:
    HandleHolder _handle;

    GetScriptModuleRevisionHashType _getScriptModuleRevisionHash;
    AddScriptsType _addScripts;
    GetScriptModuleType _getScriptModule;
    GetBuildDirectiveType _getBuildDirective;

    fs::path _path;
};

template<typename Fn>
static bool GetFunctionFromSharedLibrary(HandleType handle, std::string const& name, Fn& fn)
{
#ifdef _WIN32
    fn = reinterpret_cast<Fn>(GetProcAddress(handle, name.c_str()));
#else // Posix
    fn = reinterpret_cast<Fn>(dlsym(handle, name.c_str()));
#endif
    return fn != nullptr;
}

// Load a shared library from the given path.
Optional<std::shared_ptr<ScriptModule>> ScriptModule::CreateFromPath(fs::path const& path)
{
#ifdef _WIN32
    HandleType handle = LoadLibrary(path.generic_string().c_str());
#else // Posix
    HandleType handle = dlopen(path.c_str(), RTLD_LAZY);
#endif

    if (!handle)
    {
        TC_LOG_ERROR("scripts.hotswap", "Could not load the shared library \"%s\" for reading.",
            path.generic_string().c_str());
        return boost::none;
    }

    // Use RAII to release the library on failure.
    HandleHolder holder(handle, SharedLibraryUnloader(path));

    GetScriptModuleRevisionHashType getScriptModuleRevisionHash;
    AddScriptsType addScripts;
    GetScriptModuleType getScriptModule;
    GetBuildDirectiveType getBuildDirective;

    if (GetFunctionFromSharedLibrary(handle, "GetScriptModuleRevisionHash", getScriptModuleRevisionHash) &&
        GetFunctionFromSharedLibrary(handle, "AddScripts", addScripts) &&
        GetFunctionFromSharedLibrary(handle, "GetScriptModule", getScriptModule) &&
        GetFunctionFromSharedLibrary(handle, "GetBuildDirective", getBuildDirective))
        return std::make_shared<ScriptModule>(std::move(holder), getScriptModuleRevisionHash,
            addScripts, getScriptModule, getBuildDirective, path);
    else
    {
        TC_LOG_ERROR("scripts.hotswap", "Could not extract all required functions from the shared library \"%s\"!",
            path.generic_string().c_str());

        return boost::none;
    }
}

static bool HasValidScriptModuleName(std::string const& name)
{
    // Detects scripts_NAME.dll's / .so's
    static std::regex const regex(
        Trinity::StringFormat("^%s[sS]cripts_[a-zA-Z0-9_]+\\.%s$",
            GetSharedLibraryPrefix(),
            GetSharedLibraryExtension()));

    return std::regex_match(name, regex);
}

class LibraryUpdateListener : public efsw::FileWatchListener
{
public:
    LibraryUpdateListener() { }
    virtual ~LibraryUpdateListener() { }

    void handleFileAction(efsw::WatchID /*watchid*/, std::string const& dir,
        std::string const& filename, efsw::Action action, std::string oldFilename = "") final override;
};

static LibraryUpdateListener libraryUpdateListener;

class SourceUpdateListener : public efsw::FileWatchListener
{
public:
    SourceUpdateListener() { }
    virtual ~SourceUpdateListener() { }

    void handleFileAction(efsw::WatchID /*watchid*/, std::string const& dir,
        std::string const& filename, efsw::Action action, std::string oldFilename = "") final override;
};

static SourceUpdateListener sourceUpdateListener;

namespace std
{
    template <>
    struct hash<fs::path>
    {
        hash<string> hasher;

        std::size_t operator()(fs::path const& key) const
        {
            return hasher(key.generic_string());
        }
    };
}

/// Quotes the given string
static std::string CMakeStringify(std::string const& str)
{
    return Trinity::StringFormat("\"%s\"", str);
}

/// Invokes a synchronous CMake command action
static int InvokeCMakeCommand(std::vector<std::string> const& args)
{
    return Trinity::StartProcess(BuiltInConfig::GetCMakeCommand(), args, "scripts.hotswap");
}

/// Invokes an asynchronous CMake command action
static std::shared_ptr<Trinity::AsyncProcessResult> InvokeAsyncCMakeCommand(std::vector<std::string> args)
{
    return Trinity::StartAsyncProcess(BuiltInConfig::GetCMakeCommand(),
                                      std::move(args), "scripts.hotswap");
}

/// Calculates the C++ project name of the given module which is
/// the lowercase string of scripts_${module}.
static std::string CalculateScriptModuleProjectName(std::string const& module)
{
    std::string module_project = "scripts_" + module;
    std::transform(module_project.begin(), module_project.end(),
                   module_project.begin(), ::tolower);

    return module_project;
}

/// ScriptReloadMgr which is used when dynamic linking is enabled
///
/// This class manages shared library loading/unloading through watching
/// the script module directory. Loaded shared libraries are mirrored
/// into a .cache subdirectory to allow lazy unloading as long as
/// the shared library is still used which is useful for scripts
/// which can't be instantly replaced like spells or instances.
/// Several modules which reference different versions can be kept loaded
/// to serve scripts of different versions to entities and spells.
///
/// Also this class invokes rebuilds as soon as the source of loaded
/// scripts change and installs the modules correctly through CMake.
class HotSwapScriptReloadMgr final
    : public ScriptReloadMgr
{
    friend class ScriptReloadMgr;

    /// Reflects a queued change on a shared library or shared library
    /// which is waiting for processing
    enum class ChangeStateRequest : uint8
    {
        CHANGE_REQUEST_ADDED,
        CHANGE_REQUEST_MODIFIED,
        CHANGE_REQUEST_REMOVED
    };

    /// Reflects a running job of an invoked asynchronous external process
    enum class BuildJobType : uint8
    {
        BUILD_JOB_NONE,
        BUILD_JOB_RERUN_CMAKE,
        BUILD_JOB_COMPILE,
        BUILD_JOB_INSTALL,
    };

    // Represents a job which was invoked through a source or shared library change
    class BuildJob
    {
        // Script module which is processed in the current running job
        std::string script_module_name_;
        // The C++ project name of the module which is processed
        std::string script_module_project_name_;
        // The build directive of the current module which is processed
        // like "Release" or "Debug". The build directive from the
        // previous same module is used if there was any.
        std::string script_module_build_directive_;

        // Type of the current running job
        BuildJobType type_;
        // The async process result of the current job
        std::shared_ptr<Trinity::AsyncProcessResult> async_result_;

    public:
        explicit BuildJob(std::string script_module_name, std::string script_module_project_name,
                 std::string script_module_build_directive)
            : script_module_name_(std::move(script_module_name)),
              script_module_project_name_(std::move(script_module_project_name)),
              script_module_build_directive_(std::move(script_module_build_directive)),
              type_(BuildJobType::BUILD_JOB_NONE) { }

        bool IsValid() const
        {
            return type_ != BuildJobType::BUILD_JOB_NONE;
        }

        std::string const& GetModuleName() const { return script_module_name_; }

        std::string const& GetProjectName() const { return script_module_project_name_; }

        std::string const& GetBuildDirective() const { return script_module_build_directive_; }

        BuildJobType GetType() const { return type_; }

        std::shared_ptr<Trinity::AsyncProcessResult> const& GetProcess() const
        {
            ASSERT(async_result_, "Tried to access an empty process handle!");
            return async_result_;
        }

        /// Updates the current running job with the given type and async result
        void UpdateCurrentJob(BuildJobType type,
                              std::shared_ptr<Trinity::AsyncProcessResult> async_result)
        {
            ASSERT(type != BuildJobType::BUILD_JOB_NONE, "None isn't allowed here!");
            ASSERT(async_result, "The async result must not be empty!");

            type_ = type;
            async_result_ = std::move(async_result);
        }
    };

    /// Base class for lockfree asynchronous messages to the script reloader
    class ScriptReloaderMessage
    {
    public:
        virtual ~ScriptReloaderMessage() { }

        /// Invoke this function to run a message thread safe on the reloader
        virtual void operator() (HotSwapScriptReloadMgr* reloader) = 0;
    };

    /// Implementation class which wraps functional types and dispatches
    /// it in the overwritten implementation of the reloader messages.
    template<typename T>
    class ScriptReloaderMessageImplementation
        : public ScriptReloaderMessage
    {
        T dispatcher_;

    public:
        explicit ScriptReloaderMessageImplementation(T dispatcher)
            : dispatcher_(std::move(dispatcher)) { }

        void operator() (HotSwapScriptReloadMgr* reloader) final override
        {
            dispatcher_(reloader);
        }
    };

    /// Uses the given functional type and creates a asynchronous reloader
    /// message on the heap, which requires deletion.
    template<typename T>
    auto MakeMessage(T&& dispatcher)
        -> ScriptReloaderMessageImplementation<typename std::decay<T>::type>*
    {
        return new ScriptReloaderMessageImplementation<typename std::decay<T>::type>
            (std::forward<T>(dispatcher));
    }

public:
    HotSwapScriptReloadMgr()
        : _libraryWatcher(0), _sourceWatcher(0),
         _unique_library_name_counter(0), _last_time_library_changed(0),
         _last_time_sources_changed(0), terminate_early(false) { }

    virtual ~HotSwapScriptReloadMgr()
    {
        // Delete all messages
        ScriptReloaderMessage* message;
        while (_messages.Dequeue(message))
            delete message;
    }

    /// Returns the absolute path to the script module directory
    static fs::path GetLibraryDirectory()
    {
        return fs::absolute(sConfigMgr->GetStringDefault("HotSwap.ScriptDir", "scripts"));
    }

    /// Returns the absolute path to the scripts directory in the source tree.
    static fs::path GetSourceDirectory()
    {
        return fs::absolute("src/server/scripts", BuiltInConfig::GetSourceDirectory());
    }

    /// Initializes the file watchers and loads all existing shared libraries
    /// into the running server.
    void Initialize() final override
    {
        if (!sWorld->getBoolConfig(CONFIG_HOTSWAP_ENABLED))
            return;

        {
            auto const library_directory = GetLibraryDirectory();
            if (!fs::exists(library_directory) || !fs::is_directory(library_directory))
            {
                TC_LOG_ERROR("scripts.hotswap", "Library directory \"%s\" doesn't exist!.",
                    library_directory.generic_string().c_str());
                return;
            }
        }

        // Get the cache directory path
        fs::path const cache_path = []
        {
            auto path = fs::absolute(sScriptReloadMgr->GetLibraryDirectory());
            path /= ".cache";
            return path;
        }();

        // We use the boost filesystem function versions which accept
        // an error code to prevent it from throwing exceptions.
        boost::system::error_code code;
        if ((!fs::exists(cache_path, code) || (fs::remove_all(cache_path, code) > 0)) &&
             !fs::create_directory(cache_path, code))
        {
            TC_LOG_ERROR("scripts.hotswap", "Couldn't create the cache directory \"%s\".",
                cache_path.generic_string().c_str());
            return;
        }

        // Used to silent compiler warnings
        (void)code;

        // Correct the CMake prefix when needed
        if (sWorld->getBoolConfig(CONFIG_HOTSWAP_PREFIX_CORRECTION_ENABLED))
            DoCMakePrefixCorrectionIfNeeded();

        InitializeDefaultLibraries();
        InitializeFileWatchers();
    }

    /// Needs to be called periodically from the worldserver loop
    /// to invoke queued actions like module loading/unloading and script
    /// compilation.
    /// This method should be invoked from a thread safe point to
    /// prevent misbehavior.
    void Update() final override
    {
        // Consume all messages
        ScriptReloaderMessage* message;
        while (_messages.Dequeue(message))
        {
            (*message)(this);
            delete message;
        }

        DispatchRunningBuildJobs();
        DispatchModuleChanges();
    }

    /// Unloads the manager and cancels all runnings jobs immediately
    void Unload() final override
    {
        if (_libraryWatcher)
        {
            _fileWatcher.removeWatch(_libraryWatcher);
            _libraryWatcher = 0;
        }

        if (_sourceWatcher)
        {
            _fileWatcher.removeWatch(_sourceWatcher);
            _sourceWatcher = 0;
        }

        // If a build is in progress cancel it
        if (_build_job)
        {
            _build_job->GetProcess()->Terminate();
            _build_job.reset();
        }

        // Release all strong references to script modules
        // to trigger unload actions as early as possible,
        // otherwise the worldserver will crash on exit.
        _running_script_modules.clear();
    }

    /// Queue's a thread safe message to the reloader which is executed on
    /// the next world server update tick.
    template<typename T>
    void QueueMessage(T&& message)
    {
       _messages.Enqueue(MakeMessage(std::forward<T>(message)));
    }

    /// Queues an action which marks the given shared library as changed
    /// which will add, unload or reload it at the next world update tick.
    /// This method is thread safe.
    void QueueSharedLibraryChanged(fs::path const& path)
    {
         _last_time_library_changed = getMSTime();
         _libraries_changed.insert(path);
    }

    /// Queues a notification that a source file was added
    /// This method is thread unsafe.
    void QueueAddSourceFile(std::string const& module_name, fs::path const& path)
    {
        UpdateSourceChangeRequest(module_name, path, ChangeStateRequest::CHANGE_REQUEST_ADDED);
    }

    /// Queues a notification that a source file was modified
    /// This method is thread unsafe.
    void QueueModifySourceFile(std::string const& module_name, fs::path const& path)
    {
        UpdateSourceChangeRequest(module_name, path, ChangeStateRequest::CHANGE_REQUEST_MODIFIED);
    }

    /// Queues a notification that a source file was removed
    /// This method is thread unsafe.
    void QueueRemoveSourceFile(std::string const& module_name, fs::path const& path)
    {
        UpdateSourceChangeRequest(module_name, path, ChangeStateRequest::CHANGE_REQUEST_REMOVED);
    }

    /// Returns true when the given module name is tracked
    bool HasModuleToTrack(std::string const& module) const
    {
        return _modules_to_track.find(module) == _modules_to_track.end();
    }

private:
    // Loads all shared libraries which are contained in the
    // scripts directory on startup.
    void InitializeDefaultLibraries()
    {
        fs::path const libraryDirectory(GetLibraryDirectory());
        fs::directory_iterator const dir_end;

        uint32 count = 0;

        // Iterate through all shared libraries in the script directory and load it
        for (fs::directory_iterator dir_itr(libraryDirectory); dir_itr != dir_end ; ++dir_itr)
            if (fs::is_regular_file(dir_itr->path()) && HasValidScriptModuleName(dir_itr->path().filename().generic_string()))
            {
                TC_LOG_INFO("scripts.hotswap", "Loading script module \"%s\"...",
                    dir_itr->path().filename().generic_string().c_str());

                // Don't swap the script context to do bulk loading
                ProcessLoadScriptModule(dir_itr->path(), false);
                ++count;
            }

        TC_LOG_INFO("scripts.hotswap", ">> Loaded %u script modules.", count);
    }

    // Initialize all enabled file watchers.
    // Needs to be called after InitializeDefaultLibraries()!
    void InitializeFileWatchers()
    {
        _libraryWatcher = _fileWatcher.addWatch(GetLibraryDirectory().generic_string(), &libraryUpdateListener, false);
        if (_libraryWatcher)
        {
            TC_LOG_INFO("scripts.hotswap", ">> Library reloader is listening on \"%s\".",
                GetLibraryDirectory().generic_string().c_str());
        }
        else
        {
            TC_LOG_ERROR("scripts.hotswap", "Failed to initialize the library reloader on \"%s\".",
                GetLibraryDirectory().generic_string().c_str());
        }

        std::string const scriptsPath = GetSourceDirectory().generic_string();

        _sourceWatcher = _fileWatcher.addWatch(scriptsPath, &sourceUpdateListener, true);
        if (_sourceWatcher)
        {
            TC_LOG_INFO("scripts.hotswap", ">> Source recompiler is recursively listening on \"%s\".",
                scriptsPath.c_str());
        }
        else
        {
            TC_LOG_ERROR("scripts.hotswap", "Failed to initialize the script reloader on \"%s\".",
                scriptsPath.c_str());
        }

        _fileWatcher.watch();
    }

    /// Updates the current state of the given source path
    void UpdateSourceChangeRequest(std::string const& module_name,
                                   fs::path const& path,
                                   ChangeStateRequest state)
    {
        _last_time_sources_changed = getMSTime();

        // Write when there is no module with the given name known
        auto module_itr = _sources_changed.find(module_name);

        // When the file was modified it's enough to mark the module as
        // dirty by initializing the associated map.
        if (module_itr == _sources_changed.end())
            module_itr = _sources_changed.insert(std::make_pair(
                module_name, decltype(_sources_changed)::mapped_type{})).first;

        // Leave when the file was just modified as explained above
        if (state == ChangeStateRequest::CHANGE_REQUEST_MODIFIED)
            return;

        // Insert when the given path isn't existent
        auto const itr = module_itr->second.find(path);
        if (itr == module_itr->second.end())
        {
            module_itr->second.insert(std::make_pair(path, state));
            return;
        }

        ASSERT((itr->second == ChangeStateRequest::CHANGE_REQUEST_ADDED)
               || (itr->second == ChangeStateRequest::CHANGE_REQUEST_REMOVED),
               "Stored value is invalid!");

        ASSERT((state == ChangeStateRequest::CHANGE_REQUEST_ADDED)
               || (state == ChangeStateRequest::CHANGE_REQUEST_REMOVED),
               "The given state is invalid!");

        ASSERT(state != itr->second,
               "Tried to apply a state which is stored already!");

        module_itr->second.erase(itr);
    }

    /// Called periodically on the worldserver tick to process all
    /// load/unload/reload requests of shared libraries.
    void DispatchModuleChanges()
    {
        // When there are no libraries to change return
        if (_libraries_changed.empty())
            return;

        // Wait some time after changes to catch bulk changes
        if (GetMSTimeDiffToNow(_last_time_library_changed) < 500)
            return;

        for (auto const& path : _libraries_changed)
        {
            bool const is_running =
                _running_script_module_names.find(path) != _running_script_module_names.end();

            bool const exists = fs::exists(path);

            if (is_running)
            {
                if (exists)
                    ProcessReloadScriptModule(path);
                else
                    ProcessUnloadScriptModule(path);
            }
            else if (exists)
                ProcessLoadScriptModule(path);
        }

        _libraries_changed.clear();
    }

    void ProcessLoadScriptModule(fs::path const& path, bool swap_context = true)
    {
        ASSERT(_running_script_module_names.find(path) == _running_script_module_names.end(),
               "Can't load a module which is running already!");

        // Create the cache path and increment the library counter to use an unique name for each library
        fs::path cache_path = fs::absolute(sScriptReloadMgr->GetLibraryDirectory());
        cache_path /= ".cache";
        cache_path /= Trinity::StringFormat("%s.%u%s",
            path.stem().generic_string().c_str(),
            _unique_library_name_counter++,
            path.extension().generic_string().c_str());

        if ([&]
            {
                boost::system::error_code code;
                fs::copy_file(path, cache_path, fs::copy_option::fail_if_exists, code);
                return code;
            }())
        {
            TC_LOG_FATAL("scripts.hotswap", ">> Failed to create cache entry for module \"%s\"!",
                path.filename().generic_string().c_str());

            // Find a better solution for this but it's much better
            // to start the core without scripts
            std::this_thread::sleep_for(std::chrono::seconds(5));
            ABORT();
            return;
        }

        auto module = ScriptModule::CreateFromPath(cache_path);
        if (!module)
        {
            TC_LOG_FATAL("scripts.hotswap", ">> Failed to load script module \"%s\"!",
                path.filename().generic_string().c_str());

            // Find a better solution for this but it's much better
            // to start the core without scripts
            std::this_thread::sleep_for(std::chrono::seconds(5));
            ABORT();
            return;
        }

        // Limit the git revision hash to 7 characters.
        std::string module_revision((*module)->GetScriptModuleRevisionHash());
        if (module_revision.size() >= 7)
            module_revision = module_revision.substr(0, 7);

        std::string const module_name = (*module)->GetScriptModule();
        TC_LOG_INFO("scripts.hotswap", ">> Loaded script module \"%s\" (\"%s\" - %s).",
            path.filename().generic_string().c_str(), module_name.c_str(), module_revision.c_str());

        if (module_revision.empty())
        {
            TC_LOG_WARN("scripts.hotswap", ">> Script module \"%s\" has an empty revision hash!",
                path.filename().generic_string().c_str());
        }
        else
        {
            // Trim the revision hash
            std::string my_revision_hash = GitRevision::GetHash();
            std::size_t const trim = std::min(module_revision.size(), my_revision_hash.size());
            my_revision_hash = my_revision_hash.substr(0, trim);
            module_revision = module_revision.substr(0, trim);

            if (my_revision_hash != module_revision)
            {
                TC_LOG_WARN("scripts.hotswap", ">> Script module \"%s\" has a different revision hash! "
                    "Binary incompatibility could lead to unknown behaviour!", path.filename().generic_string().c_str());
            }
        }

        {
            auto const itr = _running_script_modules.find(module_name);
            if (itr != _running_script_modules.end())
            {
                TC_LOG_ERROR("scripts.hotswap", ">> Attempt to load a module twice \"%s\" (loaded module is at %s)!",
                    path.generic_string().c_str(), itr->second->GetModulePath().generic_string().c_str());

                return;
            }
        }

        sScriptMgr->SetScriptContext(module_name);
        (*module)->AddScripts();
        TC_LOG_TRACE("scripts.hotswap", ">> Registered all scripts of module %s.", module_name.c_str());

        if (swap_context)
            sScriptMgr->SwapScriptContext();

        _modules_to_track.insert(module_name);

        // Store the module
        _known_modules_build_directives.insert(std::make_pair(module_name, (*module)->GetBuildDirective()));
        _running_script_modules.insert(std::make_pair(module_name, std::move(*module)));
        _running_script_module_names.insert(std::make_pair(path, module_name));
    }

    void ProcessReloadScriptModule(fs::path const& path)
    {
        ProcessUnloadScriptModule(path, false);
        ProcessLoadScriptModule(path);
    }

    void ProcessUnloadScriptModule(fs::path const& path, bool finish = true)
    {
        auto const itr = _running_script_module_names.find(path);

        ASSERT(itr != _running_script_module_names.end(),
               "Can't unload a module which isn't running!");

        // Unload the script context
        sScriptMgr->ReleaseScriptContext(itr->second);

        if (finish)
            sScriptMgr->SwapScriptContext();

        _modules_to_track.erase(itr->second);

        TC_LOG_INFO("scripts.hotswap", "Released script module \"%s\" (\"%s\")...",
            path.filename().generic_string().c_str(), itr->second.c_str());

        // Unload the script module
        auto ref = _running_script_modules.find(itr->second);
        ASSERT(ref != _running_script_modules.end() &&
               "Expected the script reference to be present!");

        // Yield a message when there are other owning references to
        // the module which prevents it from unloading.
        // The module will be unloaded once all scripts provided from the module
        // are destroyed.
        if (!ref->second.unique())
        {
            TC_LOG_INFO("scripts.hotswap",
                "Script module %s is still used by %lu scripts. "
                "Will lazy unload the module once all scripts stopped using it.",
                ref->second->GetScriptModule(), ref->second.use_count() - 1);
        }

        // Remove the owning reference from the reloader
        _running_script_modules.erase(ref);
        _running_script_module_names.erase(itr);
    }

    /// Called periodically on the worldserver tick to process all recompile
    /// requests. This method invokes one build or install job at the time
    void DispatchRunningBuildJobs()
    {
        if (_build_job)
        {
            // Terminate the current build job when an associated source was changed
            // while compiling and the terminate early option is enabled.
            if (sWorld->getBoolConfig(CONFIG_HOTSWAP_EARLY_TERMINATION_ENABLED))
            {
                if (!terminate_early && _sources_changed.find(_build_job->GetModuleName()) != _sources_changed.end())
                {
                    /*
                    FIXME: Currently crashes the server
                    TC_LOG_INFO("scripts.hotswap", "Terminating the running build of module \"%s\"...",
                                _build_job->GetModuleName().c_str());

                    _build_job->GetProcess()->Terminate();
                    _build_job.reset();

                    // Continue with the default execution path
                    DispatchRunningBuildJobs();
                    return;
                    */

                    terminate_early = true;
                    return;
                }
            }

            // Wait for the current build job to finish, if the job finishes in time
            // evaluate it and continue with the next one.
            if (_build_job->GetProcess()->GetFutureResult().
                    wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                ProcessReadyBuildJob();
            else
                return; // Return when the job didn't finish in time

            // Skip this cycle when the previous job scheduled a new one
            if (_build_job)
                return;
        }

        // Avoid burst updates through waiting for a short time after changes
        if ((_last_time_sources_changed != 0) &&
            (GetMSTimeDiffToNow(_last_time_sources_changed) < 500))
            return;

        // If the changed sources are empty do nothing
        if (_sources_changed.empty())
            return;
        
        // Find all source files of a changed script module and removes
        // it from the changed source list, invoke the build afterwards.
        bool rebuild_buildfiles;
        auto module_name = [&]
        {
            auto itr = _sources_changed.begin();
            auto name = itr->first;
            rebuild_buildfiles = !itr->second.empty();
            _sources_changed.erase(itr);
            return name;
        }();

        // Erase the added delete history all modules when we
        // invoke a cmake rebuild since we add all
        // added files of other modules to the build as well
        if (rebuild_buildfiles)
        {
            for (auto& entry : _sources_changed)
                entry.second.clear();
        }

        ASSERT(!module_name.empty(),
               "The current module name is invalid!");

        TC_LOG_INFO("scripts.hotswap", "Recompiling Module \"%s\"...",
            module_name.c_str());

        // Calculate the project name of the script module
        auto project_name = CalculateScriptModuleProjectName(module_name);

        // Find the best build directive for the module
        auto build_directive = [&] () -> std::string
        {
            auto directive = sConfigMgr->GetStringDefault("HotSwap.ReCompilerBuildType", "");
            if (!directive.empty())
                return std::move(directive);

            auto const itr = _known_modules_build_directives.find(module_name);
            if (itr != _known_modules_build_directives.end())
                return itr->second;
            else // If no build directive of the module was found use the one from the game library
                return _BUILD_DIRECTIVE;
        }();

        // Initiate the new build job
        _build_job = BuildJob(std::move(module_name),
            std::move(project_name), std::move(build_directive));

        // Rerun CMake when we need to recreate the build files
        if (rebuild_buildfiles
            && sWorld->getBoolConfig(CONFIG_HOTSWAP_BUILD_FILE_RECREATION_ENABLED))
            DoRerunCMake();
        else
            DoCompileCurrentProcessedModule();
    }

    void ProcessReadyBuildJob()
    {
        ASSERT(_build_job->IsValid(), "Invalid build job!");

        // Retrieve the result
        auto const error = _build_job->GetProcess()->GetFutureResult().get();       

        if (terminate_early)
        {
            _build_job.reset();
            terminate_early = false;
            return;
        }

        switch (_build_job->GetType())
        {
            case BuildJobType::BUILD_JOB_RERUN_CMAKE:
            {
                if (!error)
                {
                    TC_LOG_INFO("scripts.hotswap", ">> Successfully updated the build files!");
                }
                else
                {
                    TC_LOG_INFO("scripts.hotswap", ">> Failed to update the build files at \"%s\", "
                                "it's possible that recently added sources are not included "
                                "in your next builds, rerun CMake manually.",
                                BuiltInConfig::GetBuildDirectory().c_str());
                }
                // Continue with building the changes sources
                DoCompileCurrentProcessedModule();
                return;
            }
            case BuildJobType::BUILD_JOB_COMPILE:
            {
                if (!error) // Build was successful
                {
                    if (sWorld->getBoolConfig(CONFIG_HOTSWAP_INSTALL_ENABLED))
                    {
                        // Continue with the installation when it's enabled
                        TC_LOG_INFO("scripts.hotswap",
                                    ">> Successfully build module %s, continue with installing...",
                                    _build_job->GetModuleName().c_str());

                        DoInstallCurrentProcessedModule();
                        return;
                    }

                    // Skip the installation because it's disabled in config
                    TC_LOG_INFO("scripts.hotswap",
                        ">> Successfully build module %s, skipped the installation.",
                        _build_job->GetModuleName().c_str());
                }
                else // Build wasn't successful
                {
                    TC_LOG_ERROR("scripts.hotswap",
                        ">> The build of module %s failed! See the log for details.",
                        _build_job->GetModuleName().c_str());
                }
                break;
            }
            case BuildJobType::BUILD_JOB_INSTALL:
            {
                if (!error)
                {
                    // Installation was successful
                    TC_LOG_INFO("scripts.hotswap", ">> Successfully installed module %s.",
                        _build_job->GetModuleName().c_str());
                }
                else
                {
                    // Installation wasn't successful
                    TC_LOG_INFO("scripts.hotswap",
                        ">> The installation of module %s failed! See the log for details.",
                        _build_job->GetModuleName().c_str());
                }
                break;
            }
            default:
                break;
        }

        // Clear the current job
        _build_job.reset();
    }

    /// Reruns CMake asynchronously over the build directory
    void DoRerunCMake()
    {
        ASSERT(_build_job, "There isn't any active build job!");

        TC_LOG_INFO("scripts.hotswap", "Rerunning CMake because there were sources added or removed...");

        _build_job->UpdateCurrentJob(BuildJobType::BUILD_JOB_RERUN_CMAKE, 
            InvokeAsyncCMakeCommand({
                "cmake",
                CMakeStringify(BuiltInConfig::GetBuildDirectory())
            }));
    }

    /// Invokes a new build of the current active module job
    void DoCompileCurrentProcessedModule()
    {
        ASSERT(_build_job, "There isn't any active build job!");

        TC_LOG_INFO("scripts.hotswap", "Starting asynchronous build job for module %s...",
                    _build_job->GetModuleName().c_str());

        _build_job->UpdateCurrentJob(BuildJobType::BUILD_JOB_COMPILE,
            InvokeAsyncCMakeCommand({
                "cmake",
                "--build",  CMakeStringify(BuiltInConfig::GetBuildDirectory()),
                "--target", CMakeStringify(_build_job->GetProjectName()),
                "--config", CMakeStringify(_build_job->GetBuildDirective())
            }));
    }

    /// Invokes a new asynchronous install of the current active module job
    void DoInstallCurrentProcessedModule()
    {
        ASSERT(_build_job, "There isn't any active build job!");

        TC_LOG_INFO("scripts.hotswap", "Starting asynchronous install job for module %s...",
                    _build_job->GetModuleName().c_str());

        _build_job->UpdateCurrentJob(BuildJobType::BUILD_JOB_INSTALL,
            InvokeAsyncCMakeCommand({
                "cmake",
                "-DCOMPONENT=" + CMakeStringify(_build_job->GetProjectName()),
                "-DBUILD_TYPE=" + CMakeStringify(_build_job->GetBuildDirective()),
                "-P", CMakeStringify(fs::absolute("cmake_install.cmake",
                    BuiltInConfig::GetBuildDirectory()).generic_string())
            }));
    }

    /// Sets the CMAKE_INSTALL_PREFIX variable in the CMake cache
    /// to point to the current worldserver position,
    /// since most users will forget this.
    void DoCMakePrefixCorrectionIfNeeded()
    {
        TC_LOG_INFO("scripts.hotswap", "Correcting your CMAKE_INSTALL_PREFIX in \"%s\"...",
                    BuiltInConfig::GetBuildDirectory().c_str());

        auto const cmake_cache_path = fs::absolute("CMakeCache.txt",
            BuiltInConfig::GetBuildDirectory());

        // Stop when the CMakeCache wasn't found
        if (![&]
        {
            boost::system::error_code error;
            if (!fs::exists(cmake_cache_path, error))
            {
                TC_LOG_ERROR("scripts.hotswap", ">> CMake cache \"%s\" doesn't exist, "
                    "set the \"BuildDirectory\" option in your worldserver.conf to point"
                    "to your build directory!",
                    cmake_cache_path.generic_string().c_str());

                return false;
            }
            else
                return true;
        }())
            return;

        TC_LOG_TRACE("scripts.hotswap", "Checking CMake cache (\"%s\") "
                     "for the correct CMAKE_INSTALL_PREFIX location...",
                     cmake_cache_path.generic_string().c_str());

        std::string cmake_cache_content;
        {
            std::ifstream in(cmake_cache_path.generic_string());
            if (!in.is_open())
            {
                TC_LOG_ERROR("scripts.hotswap", ">> Failed to read the CMake cache at \"%s\"!",
                    cmake_cache_path.generic_string().c_str());

                return;
            }

            std::ostringstream ss;
            ss << in.rdbuf();
            cmake_cache_content = ss.str();

            in.close();
        }

        static std::string const prefix_key = "CMAKE_INSTALL_PREFIX:PATH=";

        // Extract the value of CMAKE_INSTALL_PREFIX
        auto begin = cmake_cache_content.find(prefix_key);
        if (begin != std::string::npos)
        {
            begin += prefix_key.length();
            auto const end = cmake_cache_content.find("\n", begin);
            if (end != std::string::npos)
            {
                fs::path const value = cmake_cache_content.substr(begin, end - begin);
                if (value != fs::current_path())
                {
                    // Prevent correction of the install prefix
                    // when we are starting the core from inside the build tree
                    bool const is_in_path = [&]
                    {
                        fs::path base = BuiltInConfig::GetBuildDirectory();
                        fs::path branch = value;
                        while (!branch.empty())
                        {
                            if (base == branch)
                                return true;

                            branch = branch.remove_leaf();
                        }

                        return false;
                    }();

                    if (is_in_path)
                        return;

                    TC_LOG_INFO("scripts.hotswap", ">> Found outdated CMAKE_INSTALL_PREFIX (\"%s\")...",
                        value.generic_string().c_str());
                }
                else
                {
                    TC_LOG_INFO("scripts.hotswap", ">> CMAKE_INSTALL_PREFIX is equal to the current path of execution.");
                    return;
                }
            }
        }

        TC_LOG_INFO("scripts.hotswap", "Invoking CMake cache correction...");

        auto const error = InvokeCMakeCommand({
            "cmake",
            "-DCMAKE_INSTALL_PREFIX:PATH=" + CMakeStringify(fs::current_path().generic_string()),
            CMakeStringify(BuiltInConfig::GetBuildDirectory())
        });

        if (error)
        {
            TC_LOG_ERROR("scripts.hotswap", ">> Failed to update the CMAKE_INSTALL_PREFIX! "
                "This could lead to unexpected behaviour!");
        }
        else
        {
            TC_LOG_ERROR("scripts.hotswap", ">> Successfully corrected your CMAKE_INSTALL_PREFIX variable"
                         "to point at your current path of execution.");
        }
    }

    // File watcher instance and watcher ID's
    efsw::FileWatcher _fileWatcher;
    efsw::WatchID _libraryWatcher;
    efsw::WatchID _sourceWatcher;

    // Unique library name counter which is used to
    // generate unique names for every shared library version.
    uint32 _unique_library_name_counter;

    // Queue which is used for thread safe message processing
    MPSCQueue<ScriptReloaderMessage> _messages;

    // Change requests to load or unload shared libraries
    std::unordered_set<fs::path /*path*/> _libraries_changed;
    // The timestamp which indicates the last time a library was changed
    uint32 _last_time_library_changed;

    // Contains all running script modules
    // The associated shared libraries are unloaded immediately
    // on loosing ownership through RAII.
    std::unordered_map<std::string /*module name*/, std::shared_ptr<ScriptModule>> _running_script_modules;
    // Container which maps the path of a shared library to it's module name
    std::unordered_map<fs::path, std::string /*module name*/>  _running_script_module_names;
    // Container which maps the module name to it's last known build directive
    std::unordered_map<std::string /*module name*/, std::string /*build directive*/> _known_modules_build_directives;

    // Modules which were changed and are queued for recompilation
    std::unordered_map<std::string /*module*/,
        std::unordered_map<fs::path /*path*/, ChangeStateRequest /*state*/>> _sources_changed;
    // Tracks the time since the last module has changed to avoid burst updates
    uint32 _last_time_sources_changed;

    // Script modules which are tracked by the reloader
    // Any changes to modules not listed in this set are discarded.
    std::unordered_set<std::string> _modules_to_track;

    // Represents the current build job which is in progress
    Optional<BuildJob> _build_job;

    // Is true when the build job dispatcher should stop after
    // the current job has finished
    bool terminate_early;
};

void LibraryUpdateListener::handleFileAction(efsw::WatchID watchid, std::string const& dir,
    std::string const& filename, efsw::Action action, std::string oldFilename)
{
    // TC_LOG_TRACE("scripts.hotswap", "Library listener detected change on possible module \"%s\".", filename.c_str());

    // Split moved actions into a delete and an add action
    if (action == efsw::Action::Moved)
    {
        ASSERT(!oldFilename.empty(), "Old filename doesn't exist!");
        handleFileAction(watchid, dir, oldFilename, efsw::Action::Delete);
        handleFileAction(watchid, dir, filename, efsw::Action::Add);
        return;
    }

    sScriptReloadMgr->QueueMessage([=](HotSwapScriptReloadMgr* reloader) mutable
    {
        auto const path = fs::absolute(
            filename,
            sScriptReloadMgr->GetLibraryDirectory());

        if (!HasValidScriptModuleName(filename))
            return;

        switch (action)
        {
            case efsw::Actions::Add:
                TC_LOG_TRACE("scripts.hotswap", ">> Loading \"%s\"...", path.generic_string().c_str());
                reloader->QueueSharedLibraryChanged(path);
                break;
            case efsw::Actions::Delete:
                TC_LOG_TRACE("scripts.hotswap", ">> Unloading \"%s\"...", path.generic_string().c_str());
                reloader->QueueSharedLibraryChanged(path);
                break;
            case efsw::Actions::Modified:
                TC_LOG_TRACE("scripts.hotswap", ">> Reloading \"%s\"...", path.generic_string().c_str());
                reloader->QueueSharedLibraryChanged(path);
                break;
            default:
                WPAbort();
                break;
        }
    });
}

/// Returns true when the given path has a known C++ file extension
static bool HasCXXSourceFileExtension(fs::path const& path)
{
    static std::regex const regex("^\\.(h|hpp|c|cc|cpp)$");
    return std::regex_match(path.extension().generic_string(), regex);
}

void SourceUpdateListener::handleFileAction(efsw::WatchID watchid, std::string const& dir,
    std::string const& filename, efsw::Action action, std::string oldFilename)
{
    // TC_LOG_TRACE("scripts.hotswap", "Source listener detected change on possible file \"%s\".", filename.c_str());

    // Skip the file change notification if the recompiler is disabled
    if (!sWorld->getBoolConfig(CONFIG_HOTSWAP_RECOMPILER_ENABLED))
        return;

    // Split moved actions into a delete and an add action
    if (action == efsw::Action::Moved)
    {
        ASSERT(!oldFilename.empty(), "Old filename doesn't exist!");
        handleFileAction(watchid, dir, oldFilename, efsw::Action::Delete);
        handleFileAction(watchid, dir, filename, efsw::Action::Add);
        return;
    }

    auto const path = fs::absolute(
        filename,
        sScriptReloadMgr->GetSourceDirectory());

    // Check if the file is a C/C++ source file.
    if (!path.has_extension() || !HasCXXSourceFileExtension(path))
        return;

    // The file watcher returns the path relative to the watched directory
    // Check has a root directory
    fs::path const relative(filename);
    if (relative.begin() == relative.end())
        return;

    fs::path const root_directory = fs::absolute(
        *relative.begin(),
        sScriptReloadMgr->GetSourceDirectory());

    if (!fs::is_directory(root_directory))
        return;

    std::string const module = root_directory.filename().generic_string();

    /// Thread safe part
    sScriptReloadMgr->QueueMessage([=](HotSwapScriptReloadMgr* reloader)
    {
        if (reloader->HasModuleToTrack(module))
        {
            TC_LOG_TRACE("scripts.hotswap", "File %s (Module \"%s\") doesn't belong "
                "to an observed module, skipped!", filename.c_str(), module.c_str());

            return;
        }

        TC_LOG_TRACE("scripts.hotswap", "Detected source change on module \"%s\", "
            "queued for recompilation...", module.c_str());

        switch (action)
        {
            case efsw::Actions::Add:
                reloader->QueueAddSourceFile(module, path);
                break;
            case efsw::Actions::Delete:
                reloader->QueueRemoveSourceFile(module, path);
                break;
            case efsw::Actions::Modified:
                reloader->QueueModifySourceFile(module, path);
                break;
            default:
                WPAbort();
                break;
        }
    });
}

// Returns the module reference of the given context
std::shared_ptr<ModuleReference>
    ScriptReloadMgr::AcquireModuleReferenceOfContext(std::string const& context)
{
    auto const itr = sScriptReloadMgr->_running_script_modules.find(context);
    if (itr != sScriptReloadMgr->_running_script_modules.end())
        return itr->second;
    else
        return { };
}

// Returns the full hot swap implemented ScriptReloadMgr
ScriptReloadMgr* ScriptReloadMgr::instance()
{
    static HotSwapScriptReloadMgr instance;
    return &instance;
}

#endif // #ifndef TRINITY_API_USE_DYNAMIC_LINKING
