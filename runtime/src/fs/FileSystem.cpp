//
// Created by ryen on 12/5/24.
//

#include "MERuntime/fs/FileSystem.h"

namespace ME::fs {
    void Initialize() {
        vfs = std::make_shared<vfspp::VirtualFileSystem>();

        nativeFs = std::make_unique<vfspp::NativeFileSystem>("../assets");
        nativeFs->Initialize();

        vfs->AddFileSystem("/", nativeFs);
    }

    vfspp::IFilePtr OpenFile(const std::string& path, vfspp::IFile::FileMode mode) {
        return vfs->OpenFile(vfspp::FileInfo(path), mode);
    }

    vfspp::IFilePtr OpenFile(const vfspp::FileInfo& info, vfspp::IFile::FileMode mode) {
        return vfs->OpenFile(info, mode);
    }
}
