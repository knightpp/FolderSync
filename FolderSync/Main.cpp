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
#include <locale>
#include <exception>
#include "Duration.h"

#define LOG(x) std::wcout << x << L'\n';//std::endl;

namespace fs = std::filesystem;

class Directory {
	friend class File;
	friend class Folder;
	friend class SyncFolder;
private:
	Directory(const fs::directory_entry& d_entry) : d_entry(d_entry) {}

	fs::directory_entry d_entry;
};

class File : public Directory {
	//friend class SyncFolder;

public:
	File(const fs::directory_entry& d_entry) : Directory(d_entry)
	{
	}

	File() : Directory(fs::directory_entry::directory_entry())
	{

	}

	void remove() {
		if (!fs::remove(this->d_entry.path()))
			throw std::runtime_error("Cant remove file");
		this->d_entry = fs::directory_entry::directory_entry();
	}

	bool operator==(const File& rhs) const {
		/*bool rez = (rhs.d_entry.last_write_time() == this->d_entry.last_write_time()) &&
			(rhs.d_entry.file_size() == this->d_entry.file_size());*/
		bool rez = rhs.d_entry.file_size() == this->d_entry.file_size();
		//bool rez = rhs.d_entry == this->d_entry;
		return rez;
	}
};

class Folder : public Directory {
	friend class SyncFolder;
public:
	Folder(const fs::directory_entry& _entry) : Directory(_entry)
	{
		this->add_directory(_entry);
	}
	~Folder()
	{
		folders.clear();
	}

	inline bool compare_relative(const Folder* rhs, 
		const fs::path& rhs_base, const fs::path& lhs_base) const
	{ //too slow?
		return fs::relative(this->d_entry.path(), lhs_base) == fs::relative(rhs->d_entry.path(), rhs_base);
	}

	void remove() {
		if (!fs::remove_all(this->d_entry.path()))
			throw std::runtime_error("Cant remove folder");
		delete this;
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

	size_t count_folders() const {
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

	///make static???
	//returns all folders in a vector (w/o hierarchy)
	static std::vector<Folder*> get_folders(Folder* f) {

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
	std::vector<std::vector<File>> get_files() const {
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
private:
	std::vector<File> files;
	std::vector<Folder*> folders;
};

class SyncFolder {
	typedef std::vector<Folder*> Folders;
private:
	std::wstring basePath;

	inline Folders _get_next_level(const Folders& folders) {
		Folders rez; // allocate
		for (auto el : folders) {
			rez.insert(rez.end(), el->folders.begin(), el->folders.end());
		}
		return rez;
	}
	inline void _remove_file(const fs::path& path) {
		if (!fs::remove(path))
			throw std::runtime_error("Cant remove");
	}
	inline void _copy_file(const fs::path& path, const fs::path& base) {
		auto new_path = basePath + L"\\" + fs::relative(path, base).wstring();
		auto new_folder = basePath + L"\\" + fs::relative(path, base).remove_filename().wstring();
		fs::create_directories(new_folder);

		LOG(L"Copying " << new_path);

		if (!fs::copy_file(path, new_path, fs::copy_options::overwrite_existing)) {
			throw std::runtime_error("Cant copy");
		}
	}

public:
	Folder* rootFolder;
	SyncFolder(const std::wstring& path)
	{
		if (!fs::exists(path) && !fs::is_directory(path))
			throw std::runtime_error("Path isnt exists or not a directory");

		this->basePath = path;

		rootFolder = new Folder(
			fs::directory_entry::directory_entry(
				fs::path(path)));
	}
	~SyncFolder()
	{
		delete rootFolder;
	}

	size_t count_files() const {
		return rootFolder->count_files();
	}
	size_t count_folders() const {
		return rootFolder->count_folders();
	}

	void copy_new(const SyncFolder& from) {
		if (fs::equivalent(this->basePath, from.basePath))
			return;
		if (!fs::exists(this->basePath) && !fs::exists(from.basePath))
			return;
		if (from.count_files() == 0)
			return;
		// TODO:	check free disk space here

		#pragma region deleting_folders
		{
			Folders lhs = { rootFolder };
			Folders rhs = { from.rootFolder };
			Folders diff;

			auto lhsBase = basePath;
			auto rhsBase = from.basePath;
			
			std::set_difference(lhs.begin(), lhs.end(), 
				rhs.begin(), rhs.end(), diff.begin());

			/*if (!diff.empty()) {
				for (auto el : diff) {
					delete el;
					el = nullptr;
				}
			}*/
		}
#pragma endregion

		auto lhs_files = rootFolder->get_files();
		auto rhs_files = from.rootFolder->get_files();

		
		std::map<fs::path, const File*> map_lhs_files;
		std::map<fs::path, const File*> map_rhs_files; // improve?

		#pragma region build_maps
		for (const auto& v : rhs_files) {
			for (const auto& el : v) {
				map_rhs_files[fs::relative(el.d_entry.path(), from.basePath)] = &el;
			}
		}
		for (const auto& v : lhs_files) {
			for (const auto& el : v) {
				map_lhs_files[fs::relative(el.d_entry.path(), basePath)] = &el;
				// try to save to file, and load on next launch
			}
		}
#pragma endregion

		#pragma region copy_new_and_changed
		for (const auto& v : rhs_files) {
			for (const auto& el : v) {
				auto it = map_lhs_files.find(
					fs::relative(el.d_entry.path(), from.basePath));
				if (it != map_lhs_files.end()) {
					if (!(it->second->operator==(el))) {
						LOG(L"Changed file: " << el.d_entry.path().filename()
							<< L"\tSize: " << el.d_entry.file_size());
						_copy_file(el.d_entry.path(), from.basePath);
					}
				}
				else {
					LOG(L"New file: " << el.d_entry.path().filename()
						<< L"\tSize: " << el.d_entry.file_size());
					_copy_file(el.d_entry.path(), from.basePath);
				}
			}
		}
#pragma endregion

		#pragma region deleting_files
		for (auto& v : lhs_files) {
			for (auto& el : v) {
				auto it = map_rhs_files.find(
					fs::relative(el.d_entry.path(), this->basePath));
				if (it == map_rhs_files.end()) { // not found
					LOG(L"Removed file: " << el.d_entry.path().filename()
						<< L"\tSize: " << el.d_entry.file_size());
					el.remove();
				}
			}
		}
#pragma endregion

	}
};


/*
	make structure like Folder, but Directory and its hash will be hash of hashes of contents
*/

int main() {
	std::wcout.imbue(std::locale("en-US.65001"));
	std::wstring path = L"D:\\Users\\Danil\\Dropbox\\USB";
	std::wstring cfgPath = L"settings.cfg";
	{
		Duration d;
		LOG(L"Started");
		if (!fs::exists(cfgPath)) {
			LOG(L"Cfg file not found");
			LOG(L"Exiting");
			system("pause");
			return 1;
		}

		LOG(L"Cfg path: " << fs::absolute(cfgPath));

		std::ifstream settings(cfgPath);

		std::string from; // make wide?
		std::string dest;
		std::getline(settings, from);
		std::getline(settings, dest);

		settings.close();

		from = std::string(from.begin() + 6, from.end());
		dest = std::string(dest.begin() + 6, dest.end());


		try {
			SyncFolder f(std::wstring(from.begin(), from.end()));
			SyncFolder d(std::wstring(dest.begin(), dest.end()));

			//delete disappeared files, recursively copy folder is more effectively way?
			d.copy_new(f);
		}
		catch (std::exception & ex) {
			std::cout << ex.what() << std::endl;
		}


		LOG(L"Exiting");
	}

	system("pause");
	return 0;
}