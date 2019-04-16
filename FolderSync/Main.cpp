#include <string>
#include <iostream>
#include <filesystem>
#include <memory>
#include <vector>
#include <algorithm>
#include <stack>
#include <map>
#include <numeric>
#include <fstream>

#define LOG(x) std::wcerr << x << L'\n';

namespace fs = std::filesystem;

class Directory{
	friend class File;
	friend class Folder;
	friend class SyncFolder;
private:
	Directory(const fs::directory_entry& d_entry) : d_entry(d_entry){}

	fs::directory_entry d_entry;
};

class File : public Directory{
	
public:
	File(const fs::directory_entry& d_entry) : Directory(d_entry)
	{
	}

	bool operator==(const File& rhs) const{
		bool rez = (rhs.d_entry.last_write_time() == this->d_entry.last_write_time()) &&
			(rhs.d_entry.file_size() == this->d_entry.file_size());
		return rez;
	}
};

class Folder : public Directory{
public:
	Folder(const fs::directory_entry& _entry) : Directory(_entry)
	{
		this->add_directory(_entry);
	}
	~Folder()
	{
		folders.clear();
	}
	
	void add_file(const fs::directory_entry& _entry) {
		files.push_back(File(_entry));
	};

	void add_folder(const fs::directory_entry& _entry) {
		folders.push_back(new Folder(_entry));
	};

	void add_directory(const fs::directory_entry& _entry) {
		for (const auto& entry : fs::directory_iterator(_entry.path())) {
			if (entry.is_directory()) {
				this->add_folder(entry);
			}
			else if (entry.is_regular_file())
				this->add_file(entry);
		}
	}

	size_t count_folders() const{ 
		size_t size = folders.size();
		for (auto f : folders) {
			size += f->count_folders();
		}
		return size;
	};
	size_t count_files() const {
		size_t size = files.size();
		for (auto f : folders) {
			size += f->count_files();
		}
		return size;
	};

	std::vector<std::vector<File>> get_files() const { // make static?
		std::vector<std::vector<File>> rez;
		std::stack<const Folder*> temp;
		temp.push(this);

		while (!temp.empty())
		{
			if(!temp.top()->files.empty())
				rez.push_back(temp.top()->files);

			auto fld = temp.top()->folders; temp.pop();
			for (auto el : fld)
				temp.push(el);

		}
		return rez;
	}
private:
	std::vector<File> files;
	std::vector<Folder*> folders;
};

class SyncFolder {
	std::wstring basePath;

public:
	Folder* rootFolder;
	SyncFolder(const std::wstring& path)
	{
		if (!fs::exists(path) && !fs::is_directory(path))
			throw "Path isnt exists or not a directory";
		
		this->basePath = path;


		rootFolder = new Folder(
			fs::directory_entry::directory_entry(
				fs::path(path)));
	}
	~SyncFolder()
	{
		delete rootFolder;
	}

	size_t count_files() const{
		return rootFolder->count_files();
	}
	size_t count_folders() const {
		return rootFolder->count_folders();
	}



	void copyNew(const SyncFolder& from) {
		if (this->basePath == from.basePath)
			return;
		if (from.count_files() == 0)
			return;
		// TODO:	check free disk space here
		//			compare files, copy modified
		auto lhs = rootFolder->get_files();
		auto rhs = from.rootFolder->get_files();

		std::map<fs::path,const File*> map_lhs;

		for (const auto& v : lhs) {
			for (const auto& el : v) {
				map_lhs[fs::relative(el.d_entry.path(), basePath)] = &el; // dont need to rebuild map everytime
				// try to save to file, and load on next launch
			}
		}

		for (const auto& v : rhs) {
			for (const auto& el : v) {
				auto it = map_lhs.find(fs::relative(el.d_entry.path(), from.basePath));
				if (it != map_lhs.end()) { 
					// compare, and copy new if needed
					std::cout << it->second->d_entry.path().filename() << "\teq: " << it->second->operator==(el) << "\n";
				}
				else {
					auto new_path = basePath + L"\\" + fs::relative(el.d_entry.path(), from.basePath).wstring();
					auto new_folder = basePath + L"\\" + fs::relative(el.d_entry.path(), from.basePath).remove_filename().wstring();
					fs::create_directories(new_folder);
			
					LOG(L"Copying " << el.d_entry.path().filename());
					fs::copy(el.d_entry.path(), new_path, fs::copy_options::overwrite_existing);
				}
			}
		}


	}
};


int main() {
	std::wstring path = L"D:\\Users\\Danil\\Dropbox\\USB";
	
	std::ifstream settings("settings.cfg");

	std::string from;
	std::string dest;
	std::getline(settings, from);
	std::getline(settings, dest);
	
	from = std::string(from.begin() + 6, from.end());
	dest = std::string(dest.begin() + 6, dest.end());

	settings.close();

	SyncFolder f(std::wstring(from.begin(), from.end()));
	SyncFolder d(std::wstring(dest.begin(), dest.end()));

	//delete disappeared files, recursively copy folder is more effectively way?
	d.copyNew(f);
	//SyncFolder f1(L"D:\\Users\\Danil\\Dropbox\\Folder1");
	//SyncFolder f2(L"D:\\Users\\Danil\\Dropbox\\Folder2");
	
	return 0;
}