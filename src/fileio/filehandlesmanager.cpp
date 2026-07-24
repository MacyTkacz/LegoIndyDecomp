#include <filesystem.h>

using namespace FileSystem;

std::unique_ptr<FileHandlesManager> FileHandlesManager::s_pInstance = nullptr;

// get singleton instance
FileHandlesManager& FileHandlesManager::Instance() {
    if (!s_pInstance.get())
        s_pInstance = std::make_unique<FileHandlesManager>();
    return *s_pInstance;
};

namespace LIJ { FileSystem::FileHandlesManager& _FileHandlesManager = FileSystem::FileHandlesManager::Instance(); } // global instance