//
// Created by daniel on 29/08/2019.
//

#include "SyncFolder.h"

SyncFolder::SyncFolder(const std::wstring &path) {
    if (!fs::exists(path) && !fs::is_directory(path))
        throw std::runtime_error("Path isn't exists or not a directory");

    this->basePath = path;

    rootFolder = new Folder(fs::directory_entry(fs::path(path)));
}

SyncFolder::~SyncFolder() {
    delete rootFolder;
}

bool SyncFolder::_compare_relative(const Folder *rhs, const Folder *lhs, const fs::path &rhs_base,
                                   const fs::path &lhs_base) const {
    return fs::relative(lhs->d_entry.path(), lhs_base) == fs::relative(rhs->d_entry.path(), rhs_base);
}

size_t SyncFolder::count_files() const {
    return rootFolder->count_files();
}

size_t SyncFolder::count_folders() const {
    return rootFolder->count_folders();
}

void SyncFolder::copy_new(const SyncFolder &from) {

    Duration d("copy_new():");
    if (fs::equivalent(this->basePath, from.basePath))
        return;
    if (!fs::exists(this->basePath) && !fs::exists(from.basePath))
        return;
    if (from.count_files() == 0)
        return;
    //TODO:	check free disk space here

#pragma region deleting_folders
    {
        // добавить в стек указатель на массив из РХС и ЛХС ...
        //&this->rootFolder->folders, &from.rootFolder->folders


        //std::stack<std::pair<Folders*, Folders*>> stack;
        //stack.push({ &this->rootFolder->folders, &from.rootFolder->folders });
        //Folders diff;

        //while (!stack.empty())
        //{
        //	auto& pair = stack.top();
        //	std::set_difference(pair.first->begin(), pair.first->end(),
        //		pair.second->begin(), pair.second->end(), diff.begin(),
        //		[this, &from](Folder* lhs, Folder* rhs) {
        //			return this->_compare_relative(lhs, rhs, from.basePath, this->basePath);
        //		});
        //	if (!diff.empty()) {
        //		for (auto el : diff) {
        //			rootFolder->folders.erase(std::find(rootFolder->folders.begin(), // find return null
        //				rootFolder->folders.end(), el));
        //			delete el;
        //		}
        //	}
        //}

        /*Folders lhs = { rootFolder };
        Folders rhs = { from.rootFolder };
        Folders diff;

        auto lhsBase = basePath;
        auto rhsBase = from.basePath;

        while (!rhs.empty())
        {
            std::set_difference(lhs.begin(), lhs.end(),
                rhs.begin(), rhs.end(), diff.begin(), [this, &from](Folder* lhs, Folder* rhs) {
                    return this->_compare_relative(lhs, rhs, from.basePath, this->basePath);
                });

            if (!diff.empty()) {
                for (auto el : diff) {
                    lhs.erase(std::find(lhs.begin(), lhs.end(), el));
                    delete el;
                    el = nullptr;
                }
            }
            Folders temp;
            for (auto el : lhs) {
                temp.insert(temp.end(), el->folders.begin(), el->folders.end());
            }
            lhs = std::move(temp);
            for (auto el : rhs) {
                temp.insert(temp.end(), el->folders.begin(), el->folders.end());
            }
            rhs = std::move(temp);
        }*/
    }


    auto lhs_files = rootFolder->get_files();
    auto rhs_files = from.rootFolder->get_files();

    std::unordered_map<std::wstring, const File*> umap_lhs_files;
    std::unordered_map<std::wstring, const File*> umap_rhs_files;
    umap_lhs_files.reserve(lhs_files.size());
    umap_rhs_files.reserve(rhs_files.size());

//  std::map<fs::path, const File*> map_lhs_files;
//  std::map<fs::path, const File*> map_rhs_files; // improve?

#pragma region build_maps
    for (const auto& v : rhs_files) {
        for (const auto& el : v) {
            // map_rhs_files[fs::relative(el.d_entry.path(), from.basePath)] = &el;
            umap_rhs_files.insert(std::make_pair(std::move(fs::relative(el.d_entry.path(), from.basePath).wstring()), &el));
        }
    }
    for (const auto& v : lhs_files) {
        for (const auto& el : v) {
            //map_lhs_files[fs::relative(el.d_entry.path(), basePath)] = &el;
            umap_lhs_files[std::move(fs::relative(el.d_entry.path(), basePath).wstring())] = &el;
            // try to save to file, and load on next launch
        }
    }
#pragma endregion

#pragma region copy_new_and_changed
    for (const auto& v : rhs_files) {
        for (const auto& el : v) {
            auto it = umap_lhs_files.find(
                    fs::relative(el.d_entry.path(), from.basePath).wstring());
            if (it != umap_lhs_files.end()) {
                if (!(it->second->operator==(el))) {
                    /*         LOG(L"Changed file: " << el.d_entry.path().filename()
                                                   << L"\tSize: " << el.d_entry.file_size());*/
                    _copy_file(el.d_entry.path(), from.basePath);
                }
            }
            else {
                /*  LOG(L"New file: " << el.d_entry.path().filename()
                                    << L"\tSize: " << el.d_entry.file_size());*/
                _copy_file(el.d_entry.path(), from.basePath);
            }
        }
    }
#pragma endregion

#pragma region deleting_files
    for (auto& v : lhs_files) {
        for (auto& el : v) {
            auto it = umap_rhs_files.find(
                    fs::relative(el.d_entry.path(), this->basePath).wstring());
            if (it == umap_rhs_files.end()) { // not found
                /*LOG(L"Removed file: " << el.d_entry.path().filename()
                                      << L"\tSize: " << el.d_entry.file_size());*/
                el.remove();
            }
        }
    }
#pragma endregion

}

void SyncFolder::_copy_file(const fs::path &path, const fs::path &base) const {
    auto new_path = basePath + L"/" + fs::relative(path, base).wstring();
    auto new_folder = basePath + L"/" + fs::relative(path, base).remove_filename().wstring();
    fs::create_directories(new_folder);

    // LOG(L"Copying " << new_path);

    if (!fs::copy_file(path, new_path, fs::copy_options::overwrite_existing)) {
        throw std::runtime_error("Cant copy");
    }
}
