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

#define LOG(x) std::wcout << x << L'\n';//std::endl;

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
			throw std::runtime_error("Cant remove");
		this->d_entry = fs::directory_entry::directory_entry();
	}

	bool operator==(const File& rhs) const{
		/*bool rez = (rhs.d_entry.last_write_time() == this->d_entry.last_write_time()) &&
			(rhs.d_entry.file_size() == this->d_entry.file_size());*/
		bool rez = rhs.d_entry.file_size() == this->d_entry.file_size();
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

	std::vector<std::vector<File>> get_files() const {
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

private:
	void _removeFile(const fs::path& path) {
		if (!fs::remove(path))
			throw std::runtime_error("Cant remove");
	}

	void _copyFile(const fs::path& path, const fs::path& base) {
		auto new_path = basePath + L"\\" + fs::relative(path, base).wstring();
		auto new_folder = basePath + L"\\" + fs::relative(path, base).remove_filename().wstring();
		fs::create_directories(new_folder);

		std::wcout << L"Copying " << new_path << std::endl;
	//	LOG(L"Copying " << new_path);
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

	size_t count_files() const{
		return rootFolder->count_files();
	}
	size_t count_folders() const {
		return rootFolder->count_folders();
	}



	void copyNew(const SyncFolder& from) {
		if (fs::equivalent(this->basePath, from.basePath))
			return;
		if (!fs::exists(this->basePath) && !fs::exists(from.basePath))
			return;
		if (from.count_files() == 0)
			return;
		// TODO:	check free disk space here
		auto lhs = rootFolder->get_files();
		auto rhs = from.rootFolder->get_files();

		std::map<fs::path, File> map_lhs;
		std::map<fs::path, File> map_rhs; // improve?

		#pragma region build_maps
		for (const auto& v : rhs) {
			for (const auto& el : v) {
				map_rhs[fs::relative(el.d_entry.path(), from.basePath)] = el;
			}
		}
		for (const auto& v : lhs) {
			for (const auto& el : v) {
				map_lhs[fs::relative(el.d_entry.path(), basePath)] = el;
				// try to save to file, and load on next launch
			}
		}
		#pragma endregion

		for (const auto& v : rhs) {
			for (const auto& el : v) {
				auto it = map_lhs.find(fs::relative(el.d_entry.path(), from.basePath));
				if (it != map_lhs.end()) {
					// compare, and copy new if needed
					if (!(it->second.operator==(el))) {
						LOG(L"Changed file: " << el.d_entry.path().filename() << L"\tSize: " << el.d_entry.file_size());
						_copyFile(el.d_entry.path(), from.basePath);
					}
				}
				else {
					LOG(L"New file: " << el.d_entry.path().filename() << L"\tSize: " << el.d_entry.file_size());
					_copyFile(el.d_entry.path(), from.basePath);
				}
			}
		}

		for (auto& v : lhs) {
			for (auto& el : v) {
				auto it = map_rhs.find(fs::relative(el.d_entry.path(), this->basePath));
				if (it == map_rhs.end()) { // not found
					LOG(L"Removed file: " << el.d_entry.path().filename() << L"\tSize: " << el.d_entry.file_size());
					el.remove();
				}
			}
		}
	}
};


int main() {
	std::wcout.imbue(std::locale("en-US.65001"));
	std::wstring path = L"D:\\Users\\Danil\\Dropbox\\USB";
	std::wstring cfgPath = L"settings.cfg";

	LOG(L"Started");
	if (!fs::exists(cfgPath)) {
		LOG(L"Cfg file not found");
		LOG(L"Exiting");
	}

	LOG(L"Cfg path: " << fs::absolute(cfgPath));

	std::ifstream settings(cfgPath);

	std::string from;
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
		d.copyNew(f);
	}
	catch (std::exception& ex) {
		std::cout << ex.what() << std::endl;
	}

	LOG(L"Exiting");
	system("pause");
	return 0;
}