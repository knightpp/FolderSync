//
// Created by daniel on 29/08/2019.
//

#ifndef FOLDER_SYNC_FILE_H
#define FOLDER_SYNC_FILE_H

#include <filesystem>
#include "Directory.h"
class File : public Directory {
public:
    explicit File(fs::directory_entry dEntry = fs::directory_entry()) : Directory(std::move(dEntry)) {}

    void remove() {
        if (!fs::remove(this->d_entry.path()))
            throw std::runtime_error("Cant remove file");
        this->d_entry = fs::directory_entry();
    }

    bool operator==(const File& rhs) const {
        /*bool rez = (rhs.d_entry.last_write_time() == this->d_entry.last_write_time()) &&
            (rhs.d_entry.file_size() == this->d_entry.file_size());*/
        bool rez = rhs.d_entry.file_size() == this->d_entry.file_size();
        //bool rez = rhs.d_entry == this->d_entry;
        return rez;
    }
};

#endif //FOLDER_SYNC_FILE_H
