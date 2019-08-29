//
// Created by daniel on 29/08/2019.
//

#include "Folder.h"

Folder::Folder(const fs::directory_entry& _entry) : Directory(_entry)
{
    this->add_directory(_entry);
}

Folder::~Folder()
{
    for (auto p : folders)
        delete p;
}

bool Folder::operator==(const Folder &rhs) const {
    return files == rhs.files;
}

void Folder::remove() {
    if (!fs::remove_all(this->d_entry.path()))
        throw std::runtime_error("Cant remove folder");
    delete this;
}

void Folder::add_file(const fs::directory_entry &_entry) {
    files.emplace_back(File(_entry));
}

void Folder::add_folder(const fs::directory_entry &_entry) {
    folders.push_back(new Folder(_entry));
}

void Folder::add_directory(const fs::directory_entry &_entry) {
    for (const auto& entry : fs::directory_iterator(_entry.path())) {
        if (entry.is_directory()) {
            this->add_folder(entry);
        }
        else if (entry.is_regular_file())
            this->add_file(entry);
    }
}

size_t Folder::count_folders() const {
    size_t size = folders.size();
    for (auto f : folders) {
        size += f->count_folders();
    }
    return size;
}

size_t Folder::count_files() const {
    size_t size = files.size();
    for (auto f : folders) {
        size += f->count_files();
    }
    return size;
}

std::vector<Folder *> Folder::get_folders(Folder *f) {
    std::vector<Folder*> rez(f->count_folders());
    std::stack<Folder*> temp;
    temp.push(f);
    auto last = rez.begin();
    while (!temp.empty())
    {
        if (!temp.top()->folders.empty()) {
            auto beg = temp.top()->folders.begin();
            auto end = temp.top()->folders.end();
            last = std::copy(beg, end, last);
            //rez.insert(rez.end(), temp.top()->folders.begin(), temp.top()->folders.end());
        }
        auto fld = temp.top()->folders; temp.pop();
        for (auto el : fld)
            temp.push(el);
    }
    return rez;
}

std::vector<std::vector<File>> Folder::get_files() const {
    //!-!-!-!-!-!-!-!-!// return one vector
    std::vector<std::vector<File>> rez;
    std::stack<const Folder*> temp;
    temp.push(this);
    while (!temp.empty())
    {
        if (!temp.top()->files.empty())
            rez.push_back(temp.top()->files);
        auto fld = temp.top()->folders; temp.pop();
        for (auto el : fld)
            temp.push(el);
    }
    return rez;
}



