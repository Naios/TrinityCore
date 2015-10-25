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

#include "ScriptReloadMgr.h"

#ifndef TRINITY_API_USE_DYNAMIC_LINKING

// Returns the empty implemented ScriptReloadMgr
ScriptReloadMgr* ScriptReloadMgr::instance()
{
    static ScriptReloadMgr instance;
    return &instance;
}

#else

#include <mutex>
#include <regex>
#include <vector>
#include <future>
#include <memory>
#include <atomic>
#include <type_traits>
#include <unordered_set>
#include <unordered_map>

#include <iostream>

#include <boost/process.hpp>
#include <boost/filesystem.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/system/system_error.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>

#include "Log.h"
#include "Config.h"
#include "ScriptMgr.h"
#include "GitRevision.h"

#include "efsw.hpp"

using namespace boost::process;
using namespace boost::process::initializers;
using namespace boost::iostreams;

namespace fs = boost::filesystem;

#ifdef _WIN32
    #include <windows.h>
#else // Posix
    #include <dlfcn.h>
#endif

// Returns "" on Windows and "lib" on posix.
char const* GetSharedLibraryPrefix()
{
#ifdef _WIN32
    return "";
#else // Posix
    return "lib";
#endif
}

// Returns "dll" on Windows and "so" on posix.
char const* GetSharedLibraryExtension()
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
        : _name() { }
    SharedLibraryUnloader(std::string const& name)
        : _name(name) { }
    ~SharedLibraryUnloader() { }

    void operator() (HandleType handle) const
    {
        // Unload the associated shared library.
#ifdef _WIN32
        FreeLibrary(handle);
#else // Posix
        dlclose(handle);
#endif

        TC_LOG_TRACE("scripts.hotswap", "Unloaded (syscall) the shared library \"%s\".", _name.c_str());
    }

private:
    std::string _name;
};

typedef std::unique_ptr<typename std::remove_pointer<HandleType>::type, SharedLibraryUnloader> HandleHolder;

static inline HandleHolder MakeEmptyHandleHolder()
{
    return HandleHolder(nullptr, SharedLibraryUnloader{ });
}

typedef char const* (*GetScriptModuleRevisionHashType)();
typedef void (*AddScriptsType)();
typedef char const* (*GetScriptModuleType)();

class ScriptModule
{
    void Consume(ScriptModule&& right)
    {
        _handle = std::move(right._handle);

        _getScriptModuleRevisionHash = right._getScriptModuleRevisionHash;
        right._getScriptModuleRevisionHash = nullptr;

        _addScripts = right._addScripts;
        right._addScripts = nullptr;

        _getScriptModule = right._getScriptModule;
        right._getScriptModule = nullptr;

        _path = std::move(right._path);
    }

    ScriptModule(HandleHolder handle, GetScriptModuleRevisionHashType getScriptModuleRevisionHash,
                 AddScriptsType addScripts, GetScriptModuleType getScriptModule, fs::path const& path)
        : _handle(std::forward<HandleHolder>(handle)), _getScriptModuleRevisionHash(getScriptModuleRevisionHash),
          _addScripts(addScripts), _getScriptModule(getScriptModule), _path(path) { }

public:
    ScriptModule()
        : _handle(MakeEmptyHandleHolder()), _getScriptModuleRevisionHash(nullptr),
          _addScripts(nullptr), _getScriptModule(nullptr) { }
    
    ~ScriptModule() { }

    ScriptModule(ScriptModule const&) = delete;
    ScriptModule(ScriptModule&& right)
        : _handle(MakeEmptyHandleHolder())
    {
        Consume(std::forward<ScriptModule>(right));
    }

    ScriptModule& operator= (ScriptModule const&) = delete;
    ScriptModule& operator= (ScriptModule&& right)
    {
        Consume(std::forward<ScriptModule>(right));
        return *this;
    }

    static ScriptModule CreateFromPath(fs::path const& path) /*noexcept(false)*/;

    char const* GetScriptModuleRevisionHash() const
    {
        return _getScriptModuleRevisionHash();
    }

    void AddScripts() const
    {
        return _addScripts();
    }

    char const* GetScriptModule() const
    {
        return _getScriptModule();
    }

    fs::path const& GetModulePath() const
    {
        return _path;
    }

private:
    HandleHolder _handle;

    GetScriptModuleRevisionHashType _getScriptModuleRevisionHash;
    AddScriptsType _addScripts;
    GetScriptModuleType _getScriptModule;

    fs::path _path;
};

template<typename Fn>
static inline bool GetFunctionFromSharedLibrary(HandleType handle, std::string const& name, Fn& fn)
{
#ifdef _WIN32
    fn = reinterpret_cast<Fn>(GetProcAddress(handle, name.c_str()));
#else // Posix
    fn = reinterpret_cast<Fn>(dlsym(handle, name.c_str()));
#endif
    return fn != nullptr;
}

// Load a shared library from the given path.
// Throws a std::runtime_exception on failure!
ScriptModule ScriptModule::CreateFromPath(fs::path const& path)
{
#ifdef _WIN32
    HandleType handle = LoadLibrary(path.generic_string().c_str());
#else // Posix
    HandleType handle = dlopen(path.c_str(), RTLD_LAZY);
#endif

    if (!handle)
    {
        TC_LOG_ERROR("scripts.hotswap", "Could not load the shared library \"%s\" for reading.", path.generic_string().c_str());
        throw std::runtime_error("failed to load the library");
    }

    HandleHolder holder(handle, SharedLibraryUnloader(path.generic_string()));

    GetScriptModuleRevisionHashType getScriptModuleRevisionHash;
    AddScriptsType addScripts;
    GetScriptModuleType getScriptModule;

    if (GetFunctionFromSharedLibrary(handle, "GetScriptModuleRevisionHash", getScriptModuleRevisionHash) &&
        GetFunctionFromSharedLibrary(handle, "AddScripts", addScripts) &&
        GetFunctionFromSharedLibrary(handle, "GetScriptModule", getScriptModule))
        return ScriptModule(std::move(holder), getScriptModuleRevisionHash, addScripts, getScriptModule, path);
    else
    {
        TC_LOG_ERROR("scripts.hotswap", "Could not extract all required functions from the shared library \"%s\"!", path.generic_string().c_str());
        throw std::runtime_error("failed to extract the functions");
    }
}

bool IsScriptModule(std::string const& name)
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

    void AddModuleToTrack(std::string const& module)
    {
        std::lock_guard<std::mutex> guard(_lock);
        _modules.insert(module);
    }

    void RemoveModuleToTrack(std::string const& module)
    {
        std::lock_guard<std::mutex> guard(_lock);
        _modules.erase(module);
    }

private:
    std::mutex _lock; // ToDo Improve this: it's not good to lock this handler
    std::unordered_set<std::string> _modules;
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

static inline std::string stringify(std::string const& str)
{
    return Trinity::StringFormat("\"%s\"", str);
}

// ScriptReloadMgr which is used when dynamic linking is enabled
class HotSwapScriptReloadMgr : public ScriptReloadMgr
{
    enum LibraryChangedState : uint8
    {
        LOAD,
        RELOAD,
        UNLOAD
    };

public:
    HotSwapScriptReloadMgr()
        : _libraryWatcher(0), _sourceWatcher(0), _last_time_module_changed(0) { }

    virtual ~HotSwapScriptReloadMgr() { }

    std::string const& GetCachedLibraryDirectory()
    {
        return _cachedLibraryDirectory;
    }

    std::string const& GetCachedSourceDirectory()
    {
        return _cachedSourceDirectory;
    }

    void Initialize() final override
    {
        if (!sWorld->getBoolConfig(CONFIG_HOTSWAP_ENABLED))
            return;

        _cachedLibraryDirectory = fs::absolute(
            sConfigMgr->GetStringDefault("HotSwap.ScriptDir", "scripts")).generic_string();

        if (!fs::exists(_cachedLibraryDirectory) || !fs::is_directory(_cachedLibraryDirectory))
        {
            TC_LOG_ERROR("scripts.hotswap", "Library directory \"%s\" doesn't exist!.", _cachedLibraryDirectory.c_str());
            return;
        }

        _cachedSourceDirectory = fs::absolute(
            "src/server/scripts",
            sConfigMgr->GetSourceDirectory()).generic_string();

        // Create the cache directory
        fs::path const cache_path = fs::absolute(
            ".cache",
            static_cast<HotSwapScriptReloadMgr*>(sScriptReloadMgr)->GetCachedLibraryDirectory());

        boost::system::error_code code;
        if ((!fs::exists(cache_path, code) || (fs::remove_all(cache_path, code) > 0)) &&
             !fs::create_directory(cache_path, code))
        {
            TC_LOG_ERROR("scripts.hotswap", "Couldn't create the cache directory \"%s\".", cache_path.generic_string().c_str());
            return;
        }

        (void)code;

        InitializeDefaultLibraries();
        InitializeFileWatchers();
    }

    void Update() final override
    {
        DispatchCompileScriptModules();
        DispatchScriptModuleChanges();
    }

    void Unload() final override
    {
        if (_libraryWatcher)
            _fileWatcher.removeWatch(_libraryWatcher);

        if (_sourceWatcher)
            _fileWatcher.removeWatch(_sourceWatcher);

        // If a build is in progress wait for it to finish
        if (_build_job)
        {
            _build_job->wait();
            _build_job.reset();
        }
    }

    void LoadScriptModule(fs::path const& path)
    {
        UpdateModuleEntry(path, LOAD);
    }

    void ReloadScriptModule(fs::path const& path)
    {
        UpdateModuleEntry(path, RELOAD);
    }

    void UnloadScriptModule(fs::path const& path)
    {
        UpdateModuleEntry(path, UNLOAD);
    }

    void CompileScriptModule(std::string const& name)
    {
        std::lock_guard<std::mutex> guard(_modules_changed_lock);
        _modules_changed.insert(name);
        _last_time_module_changed = getMSTime();
    }

private:
    // Loads all shared libraries which are contained in the
    // scripts directory on startup.
    void InitializeDefaultLibraries()
    {
        fs::path const libraryDirectory(GetCachedLibraryDirectory());
        fs::directory_iterator const dir_end;

        uint32 count = 0;

        // Iterate through all shared libraries in the script directory and load it
        for (fs::directory_iterator dir_itr(libraryDirectory); dir_itr != dir_end ; ++dir_itr)
            if (fs::is_regular_file(dir_itr->path()) && IsScriptModule(dir_itr->path().filename().generic_string()))
            {
                TC_LOG_INFO("scripts.hotswap", ">> Loading script module \"%s\"...", dir_itr->path().filename().generic_string().c_str());
                ProcessLoadScriptModule(dir_itr->path());
                ++count;
            }

        TC_LOG_INFO("scripts.hotswap", "Loaded %u script modules.", count);
    }

    // Initialize all enabled file watchers.
    // Needs to be called after InitializeDefaultLibraries()!
    void InitializeFileWatchers()
    {
        if ((_libraryWatcher = _fileWatcher.addWatch(GetCachedLibraryDirectory(), &libraryUpdateListener, false)))
            TC_LOG_INFO("scripts.hotswap", ">> Library reloader is listening on \"%s\".", GetCachedLibraryDirectory().c_str());
        else
            TC_LOG_ERROR("scripts.hotswap", "Failed to initialize the library reloader on \"%s\".", GetCachedLibraryDirectory().c_str());

        if (sWorld->getBoolConfig(CONFIG_HOTSWAP_RECOMPILER_ENABLED))
        {
            std::string const scriptsPath = GetCachedSourceDirectory();

            if ((_sourceWatcher = _fileWatcher.addWatch(scriptsPath, &sourceUpdateListener, true)))
                TC_LOG_INFO("scripts.hotswap", ">> Source recompiler is recursively listening on \"%s\".", scriptsPath.c_str());
            else
                TC_LOG_ERROR("scripts.hotswap", "Failed to initialize the script reloader on \"%s\".", scriptsPath.c_str());
        }

        _fileWatcher.watch();
    }

    void UpdateModuleEntry(fs::path const& path, LibraryChangedState state)
    {
        _last_time_library_changed = getMSTime();

        std::lock_guard<std::mutex> guard(_libraries_changed_lock);

        auto const itr = _libraries_changed.find(path);
        if (itr != _libraries_changed.end())
            itr->second = state;
        else
            _libraries_changed.insert(std::make_pair(path, state));
    }

    void DispatchScriptModuleChanges()
    {
        if ((_last_time_module_changed != 0) &&
            (GetMSTimeDiffToNow(_last_time_library_changed) < (1 * IN_MILLISECONDS)))
            return;

        decltype(_libraries_changed) libraries_changed;
        {
            std::lock_guard<std::mutex> guard(_libraries_changed_lock);

            if (_libraries_changed.empty())
                return;

            // Transfer _libraries_changed to the local scope
            std::swap(libraries_changed, _libraries_changed);
        }

        _last_time_library_changed = 0;

        for (auto&& path : libraries_changed)
        {
            switch (path.second)
            {
                case LOAD:
                    ProcessLoadScriptModule(path.first);
                    break;
                case RELOAD:
                    ProcessReloadScriptModule(path.first);
                    break;
                case UNLOAD:
                    ProcessUnloadScriptModule(path.first);
                    break;
            }
        }
    }

    void ProcessLoadScriptModule(fs::path const& path)
    {
        ASSERT(_running_script_module_names.find(path) == _running_script_module_names.end());

        fs::path const cache_path = fs::absolute(
            ".cache/" + path.filename().generic_string(),
            static_cast<HotSwapScriptReloadMgr*>(sScriptReloadMgr)->GetCachedLibraryDirectory());

        {
            boost::system::error_code code;
            fs::copy_file(path, cache_path, fs::copy_option::overwrite_if_exists, code);

            if (code)
            {
                TC_LOG_ERROR("scripts.hotswap", "Failed to create cache entry of module \"%s\"!", path.filename().generic_string().c_str());
                return;
            }
        }

        ScriptModule module;

        try
        {
            module = ScriptModule::CreateFromPath(cache_path);
        }
        catch (...)
        {
            TC_LOG_ERROR("scripts.hotswap", "Failed to load the script module \"%s\"!", path.filename().generic_string().c_str());
            return;
        }

        std::string module_revision(module.GetScriptModuleRevisionHash());
        if (module_revision.size() >= 7)
            module_revision = module_revision.substr(0, 7);

        std::string const module_name = module.GetScriptModule();
        TC_LOG_INFO("scripts.hotswap", "Loaded script module \"%s\" (\"%s\" - %s)...",
            path.filename().generic_string().c_str(), module_name.c_str(), module_revision.c_str());

        if (module_revision.empty())
        {
            TC_LOG_WARN("scripts.hotswap", "Script module \"%s\" has an empty revision hash!", path.filename().generic_string().c_str());
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
                TC_LOG_WARN("scripts.hotswap", "Script module \"%s\" has a different revision hash! Binary incompatibility could lead to unknown behaviour!", path.filename().generic_string().c_str());
            }
        }

        {
            auto const itr = _running_script_modules.find(module_name);
            if (itr != _running_script_modules.end())
            {
                TC_LOG_ERROR("scripts.hotswap", "Attempt to load a module twice \"%s\" (loaded module is at %s)!",
                    path.generic_string().c_str(), itr->second.GetModulePath().generic_string().c_str());

                return;
            }
        }

        sScriptMgr->BeginScriptContext(module_name);
        // TODO Enable this
        // module.AddScripts();       
        sScriptMgr->FinishScriptContext();

        sourceUpdateListener.AddModuleToTrack(module_name);

        // Store the module
        _running_script_modules.insert(std::make_pair(module_name, std::move(module)));
        _running_script_module_names.insert(std::make_pair(path, module_name));
    }

    void ProcessReloadScriptModule(fs::path const& path)
    {
        ProcessUnloadScriptModule(path);
        ProcessLoadScriptModule(path);
    }

    void ProcessUnloadScriptModule(fs::path const& path)
    {
        auto const itr = _running_script_module_names.find(path);
        if (itr == _running_script_module_names.end())
            return;

        // Unload the script context
        sScriptMgr->ReleaseScriptContext(itr->second);

        sourceUpdateListener.RemoveModuleToTrack(itr->second);

        TC_LOG_INFO("scripts.hotswap", "Unloaded script module \"%s\" (\"%s\")...",
            path.filename().generic_string().c_str(), itr->second.c_str());

        // Unload the script module
        _running_script_modules.erase(itr->second);
        _running_script_module_names.erase(itr);
    }

    struct BuildJobResult
    {
        enum BuildJobType
        {
            BUILD_JOB_COMPILE,
            BUILD_JOB_INSTALL,
        };

        BuildJobResult() : success(false) { }

        BuildJobResult(BuildJobType type_, std::string const& name_, std::string const& project_, bool success_)
            : type(type_), name(name_), project(project_), success(success_) { }

        BuildJobType type;

        std::string name;

        std::string project;

        bool success;
    };

    // Process changes on the script source files
    // Starts a build job which recompiles the changed sources
    void DispatchCompileScriptModules()
    {
        if (_build_job)
        {
            // Check build job
            if (_build_job->wait_for(std::chrono::seconds(3 /*wait some time*/)) == std::future_status::ready)
            {
                BuildJobResult result = _build_job->get();

                // Install the target
                if (result.success)
                {
                    if (result.type == BuildJobResult::BUILD_JOB_COMPILE)
                    {
                        TC_LOG_INFO("scripts.hotswap", "Last build of %s job succeeded, continue with installing...", result.name.c_str());

                        _build_job = Trinity::make_unique<std::future<BuildJobResult>>(std::async([=]
                        {
                            TC_LOG_INFO("scripts.hotswap", ">> Started asynchronous install job...");

                            std::vector<std::string> const args =
                            {
                                "cmake",
                                "-DCOMPONENT=" + stringify(result.project),
                                "-P", stringify(fs::absolute("cmake_install.cmake", sConfigMgr->GetBuildDirectory()).generic_string())
                            };

                            TC_LOG_TRACE("scripts.hotswap", ">> Invoking install \"%s\"", boost::algorithm::join(args, " ").c_str());

                            boost::process::pipe outPipe = create_pipe();
                            boost::process::pipe errPipe = create_pipe();

                            child c = execute(run_exe(
                                        boost::filesystem::absolute(sConfigMgr->GetCMakeCommand()).generic_string()),
                                        set_args(args),
                                        bind_stdout(file_descriptor_sink(outPipe.sink, close_handle)),
                                        bind_stderr(file_descriptor_sink(errPipe.sink, close_handle)));

                            file_descriptor_source stdFD(outPipe.source, close_handle);
                            file_descriptor_source errFD(errPipe.source, close_handle);

                            stream<file_descriptor_source> stdStream(stdFD);
                            stream<file_descriptor_source> errStream(errFD);

                            copy(stdStream, std::cout);
                            copy(errStream, std::cerr);

                            // TC_LOG_INFO("scripts.hotswap", "[Build] %s", out.str().c_str());
                            // TC_LOG_ERROR("scripts.hotswap", "[Build] %s", err.str().c_str());

                            auto const ret = wait_for_exit(c);

                            auto new_result = result;
                            new_result.type = BuildJobResult::BUILD_JOB_INSTALL;
                            new_result.success = (ret == EXIT_SUCCESS);

                            return new_result;
                        }));
                    }
                    else
                    {
                        TC_LOG_INFO("scripts.hotswap", "Installed module %s.", result.name.c_str());
                    }
                }
                else
                {
                    // No success
                    TC_LOG_ERROR("scripts.hotswap", "Last build for module %s failed!", result.name.c_str());
                }
            }
            else
            {
                // Build is in progress, wait for it to finish
                return;
            }

            _build_job.reset();
        }

        // Avoid burst updates through waiting for a short time after changes
        if ((_last_time_module_changed != 0) && (GetMSTimeDiffToNow(_last_time_module_changed) < (1 * IN_MILLISECONDS)))
            return;

        std::string module;

        {
            std::lock_guard<std::mutex> guard(_modules_changed_lock);

            // If the changed modules are empty do nothing
            if (_modules_changed.empty())
                return;

            // Compile only 1 module at the same time
            auto const itr = _modules_changed.begin();
            module = *itr;
            _modules_changed.erase(itr);
        }

        _last_time_module_changed = 0;

        TC_LOG_INFO("scripts.hotswap", "Recompiling Module \"%s\"...", module.c_str());

        // Calculate the project name of the script module
        std::string module_project = "scripts_" + module;
        std::transform(module_project.begin(), module_project.end(), module_project.begin(), ::tolower);

        // Spawn the asynchronous task
        _build_job = Trinity::make_unique<std::future<BuildJobResult>>(std::async([=]
        {
            TC_LOG_INFO("scripts.hotswap", ">> Started asynchronous build job...");

            std::vector<std::string> const args =
            {
                "cmake",
                "--build", stringify(sConfigMgr->GetBuildDirectory()),
                "--target", stringify(module_project),
                "--config", "Release"
            };

            TC_LOG_TRACE("scripts.hotswap", ">> Invoking build \"%s\"", boost::algorithm::join(args, " ").c_str());

            boost::process::pipe outPipe = create_pipe();
            boost::process::pipe errPipe = create_pipe();

            child c = execute(run_exe(
                        boost::filesystem::absolute(sConfigMgr->GetCMakeCommand()).generic_string()),
                        set_args(args),
                        bind_stdout(file_descriptor_sink(outPipe.sink, close_handle)),
                        bind_stderr(file_descriptor_sink(errPipe.sink, close_handle)));

            file_descriptor_source stdFD(outPipe.source, close_handle);
            file_descriptor_source errFD(errPipe.source, close_handle);

            stream<file_descriptor_source> stdStream(stdFD);
            stream<file_descriptor_source> errStream(errFD);

            copy(stdStream, std::cout);
            copy(errStream, std::cerr);

            auto const ret = wait_for_exit(c);

            return BuildJobResult(BuildJobResult::BUILD_JOB_COMPILE, module, module_project, (ret == EXIT_SUCCESS));
        }));

        // Call recursively
        DispatchScriptModuleChanges();
    }

    // Cached config variables for fast access
    std::string _cachedLibraryDirectory;
    std::string _cachedSourceDirectory;

    // File watcher instance
    efsw::FileWatcher _fileWatcher;

    // Library watcher id's
    efsw::WatchID _libraryWatcher;
    efsw::WatchID _sourceWatcher;

    // Mutex responsible for _libraries_changed
    std::mutex _libraries_changed_lock;
    std::unordered_map<fs::path, LibraryChangedState> _libraries_changed;
    std::atomic<uint32> _last_time_library_changed;

    // Contains all running script modules
    // The associated shared libraries are unloaded immediately
    // on loosing ownership through RAII.
    std::unordered_map<std::string /*module name*/, ScriptModule> _running_script_modules;
    std::unordered_map<fs::path, std::string /*module name*/>  _running_script_module_names;

    // Mutex responsible for _modules_changed
    std::mutex _modules_changed_lock;
    std::unordered_set<std::string> _modules_changed;
    std::atomic<uint32> _last_time_module_changed;

    // Terminate the build early
    // std::atomic_bool _terminate_early;

    // Optional build job future
    std::unique_ptr<std::future<BuildJobResult>> _build_job;
};

void LibraryUpdateListener::handleFileAction(efsw::WatchID /*watchid*/, std::string const& /*dir*/,
    std::string const& filename, efsw::Action action, std::string oldFilename)
{
    TC_LOG_TRACE("scripts.hotswap", "Library listener detected change of file \"%s\".", filename.c_str());

    fs::path const& path = fs::absolute(
        filename,
        static_cast<HotSwapScriptReloadMgr*>(sScriptReloadMgr)->GetCachedLibraryDirectory());

    if (!IsScriptModule(filename))
    {
        // Catch renaming to an invalid name
        if (!oldFilename.empty() && (action == efsw::Actions::Moved) && IsScriptModule(oldFilename))
            static_cast<HotSwapScriptReloadMgr*>(sScriptReloadMgr)->UnloadScriptModule(path);

        return;
    }

    // Catch renaming to a valid name
    if (!oldFilename.empty() && (action == efsw::Actions::Moved) && !IsScriptModule(oldFilename))
        action = efsw::Actions::Add;

    switch (action)
    {
        case efsw::Actions::Add:
            TC_LOG_TRACE("scripts.hotswap", ">> Loading \"%s\"...", path.generic_string().c_str());
            static_cast<HotSwapScriptReloadMgr*>(sScriptReloadMgr)->LoadScriptModule(path);
            break;
        case efsw::Actions::Delete:
            TC_LOG_TRACE("scripts.hotswap", ">> Unloading \"%s\"...", path.generic_string().c_str());
            static_cast<HotSwapScriptReloadMgr*>(sScriptReloadMgr)->UnloadScriptModule(path);
            break;
        case efsw::Actions::Modified:
            TC_LOG_TRACE("scripts.hotswap", ">> Reloading \"%s\"...", path.generic_string().c_str());
            static_cast<HotSwapScriptReloadMgr*>(sScriptReloadMgr)->ReloadScriptModule(path);
            break;
        case efsw::Actions::Moved:
            TC_LOG_TRACE("scripts.hotswap", ">> Moving \"%s\" -> \"%s\"...", path.generic_string().c_str(), oldFilename.c_str());
            static_cast<HotSwapScriptReloadMgr*>(sScriptReloadMgr)->UnloadScriptModule(fs::absolute(
                oldFilename, static_cast<HotSwapScriptReloadMgr*>(sScriptReloadMgr)->GetCachedLibraryDirectory()));
            static_cast<HotSwapScriptReloadMgr*>(sScriptReloadMgr)->LoadScriptModule(path);
            break;
        default:
            ABORT();
            break;
    }
}

void SourceUpdateListener::handleFileAction(efsw::WatchID /*watchid*/, std::string const& /*dir*/,
    std::string const& filename, efsw::Action /*action*/, std::string /*oldFilename*/)
{
    fs::path const path = fs::absolute(
        filename,
        static_cast<HotSwapScriptReloadMgr*>(sScriptReloadMgr)->GetCachedSourceDirectory());

    // Check if the file is a possible source file.   
    if(!fs::is_regular_file(path) || !path.has_extension())
        return;

    // Check if the file is a C/C++ source file.
    std::string const extension = path.extension().generic_string();
    static std::regex const regex("^\\.(h|(hpp)|c|(cc)|(cpp))$");
    if (!std::regex_match(extension, regex))
          return;

    // The file watcher returns the path relative to the watched directory
    // Check has a root directory
    fs::path const relative(filename);
    if (relative.begin() == relative.end())
        return;

    fs::path const root_directory = fs::absolute(
        *relative.begin(),
        static_cast<HotSwapScriptReloadMgr*>(sScriptReloadMgr)->GetCachedSourceDirectory());

    if (!fs::is_directory(root_directory))
        return;

    std::string const module = root_directory.filename().generic_string();

    {
        std::lock_guard<std::mutex> guard(_lock);

        if (_modules.find(module) == _modules.end())
        {
            TC_LOG_TRACE("scripts.hotswap", "File %s (Module \"%s\") doesn't belong to an observed module, skipped!", filename.c_str(), module.c_str());
            return;
        }
    }

    TC_LOG_TRACE("scripts.hotswap", "Detected source change on module \"%s\", queued for recompilation...", module.c_str());

    static_cast<HotSwapScriptReloadMgr*>(sScriptReloadMgr)->CompileScriptModule(module);
}

// Returns the full hot swap implemented ScriptReloadMgr
ScriptReloadMgr* ScriptReloadMgr::instance()
{
    static HotSwapScriptReloadMgr instance;
    return &instance;
}

#endif // #ifndef TRINITY_API_USE_DYNAMIC_LINKING
