#include "Misc/Paths.hpp"
#include <filesystem>
#include <stdexcept>
#include <libloaderapi.h>

namespace fs = std::filesystem;

std::string Paths::GetCurrentExecutablePath()
{
    char path[1024] = {0};

#if defined(_WIN32)
    // Windows
    GetModuleFileNameA(nullptr, path, sizeof(path));
#elif defined(__linux__)
    // Linux
    readlink("/proc/self/exe", path, sizeof(path) - 1);
#elif defined(__APPLE__)
    // macOS
    uint32_t size = sizeof(path);
    _NSGetExecutablePath(path, &size);
#else
    throw std::runtime_error("Unsupported platform");
#endif

    return std::string(path);
}

std::string Paths::GetAssetPath()
{
    // 获取当前可执行文件所在目录
    fs::path exePath = fs::canonical(GetCurrentExecutablePath());
    fs::path binDir = exePath.parent_path();

    // 向上查找最多 5 层目录，寻找 CMakeLists.txt 所在目录（即项目根目录）
    fs::path projectRoot = binDir;
    for (int i = 0; i < 5; ++i)
    {
        if (fs::exists(projectRoot / "CMakeLists.txt"))
        {
            break;
        }
        projectRoot = projectRoot.parent_path();
    }

    if (!fs::exists(projectRoot / "CMakeLists.txt"))
    {
        throw std::runtime_error("Could not find project root directory.");
    }

    fs::path assetPath = projectRoot / "resources";

    if (!fs::exists(assetPath))
    {
        throw std::runtime_error("Asset directory not found: " + assetPath.string());
    }

    return assetPath.string();
}

std::string Paths::GetAssetFullPath(const std::string &relativePath)
{
    return GetAssetPath() + "/" + relativePath;
}
