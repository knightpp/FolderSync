//
// Created by daniel on 29/08/2019.
//

#ifndef FOLDER_SYNC_SYNCFOLDER_H
#define FOLDER_SYNC_SYNCFOLDER_H

#include <unordered_map>
#include <vector>
#include "Folder.h"
#include "Duration.h"

class SyncFolder {
    typedef std::vector<Folder*> Folders;
private:
    inline void _copy_file(const fs::path& path, const fs::path& base) const;

public:
    Folder* rootFolder;
    std::wstring basePath;
    explicit SyncFolder(const std::wstring& path);

    ~SyncFolder();


    inline bool _compare_relative(const Folder* rhs, const Folder* lhs,
                                  const fs::path& rhs_base, const fs::path& lhs_base) const;
    [[nodiscard]] size_t count_files() const;

    [[nodiscard]] size_t count_folders() const;

    void copy_new(const SyncFolder& from);

};


#endif //FOLDER_SYNC_SYNCFOLDER_H
