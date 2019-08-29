//
// Created by daniel on 29/08/2019.
//
#ifndef FOLDER_SYNC_DIRECTORY_H
#define FOLDER_SYNC_DIRECTORY_H
#include <filesystem>
namespace fs = std::filesystem;
class Directory {

    friend class File;
    friend class Folder;
    friend class SyncFolder;

private:
    explicit Directory(fs::directory_entry  d_entry) : d_entry(std::move(d_entry)) {}
    fs::directory_entry d_entry;
};

#endif //FOLDER_SYNC_DIRECTORY_H
