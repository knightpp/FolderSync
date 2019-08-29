//
// Created by daniel on 29/08/2019.
//

#ifndef FOLDER_SYNC_FOLDER_H
#define FOLDER_SYNC_FOLDER_H

#include <stack>
#include <vector>
#include "File.h"
#include "Directory.h"

class Folder : public Directory {
    friend class SyncFolder;

private:
    std::vector<File> files;
    std::vector<Folder*> folders;

public:
    explicit Folder(const fs::directory_entry& _entry);
    ~Folder();

    bool operator==(const Folder& rhs) const;

    void remove();

    void add_file(const fs::directory_entry& _entry);

    void add_folder(const fs::directory_entry& _entry);

    void add_directory(const fs::directory_entry& _entry);

    size_t count_folders() const;
    size_t count_files() const;

    /**
     * Recursively get all folders.
     * @param f folder to look recursively in
     * @return vector of Folder(s)
     */
    static std::vector<Folder*> get_folders(Folder* f);
    /**
     * Recursively get all files.
     * @return vector of vectors of File
     */
    [[nodiscard]] std::vector<std::vector<File>> get_files() const;

};
#endif //FOLDER_SYNC_FOLDER_H
