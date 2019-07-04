#include <string>
#include <iostream>
#include <filesystem>
#include <memory>
#include <vector>
#include <algorithm>
#include <map>
#include <numeric>
#include <fstream>
#include <sstream>
#include <iterator>

#define LOG(x) std::wcerr << x << L'\n';

namespace fs = std::filesystem;
using std::vector;

int main() {
	const fs::path baseSource(L"D:\\Desktop\\TEMP\\vk");
	const fs::path baseDest(L"D:\\Desktop\\TEMP\\vk2");
	vector<fs::directory_entry> vfilesSource;
	vector<fs::directory_entry> vfilesDest;
	std::map<fs::path, fs::directory_entry> m;
	std::map<fs::path, fs::directory_entry> m2;
	
	for (const auto& entry : fs::recursive_directory_iterator(baseSource)) {
		if (entry.is_regular_file())
			vfilesSource.push_back(entry);
	}

	for (const auto& entry : fs::recursive_directory_iterator(baseDest)) {
		if (entry.is_regular_file())
			vfilesDest.push_back(entry);
	}
	
	for (auto& el : vfilesSource) {
		m.insert(std::make_pair(fs::relative(el.path(), baseSource), el));
	}

	for (auto& el : vfilesDest) {
		m2.insert(std::make_pair(fs::relative(el.path(), baseDest), el));
	}

	for (const auto& item : vfilesDest) {
		static int counter = 0;
		counter++;
		//std::wcout << fs::relative(item.path(), baseDest).filename() << std::endl;
		auto rel = fs::relative(item.path(), baseDest);
		auto found = m.find(rel);
		if (found == m.end()) {
			//deleting files
			std::cout << counter << ") Deleting removed file << " << item.path().filename() << '\n';
		}
	}

	for (auto item : vfilesSource) {
		static int counter = 0;
		counter++;
		//std::wcout << fs::relative(item.path(), baseDest).filename() << std::endl;
		auto rel = fs::relative(item.path(), baseSource);
		auto found = m2.find(rel);
		if (found != m2.end()) {
			//compare files
			std::cout << counter << ") Compare similar files >> " << found->second.path().filename() << '\n';
		}
		else {
			//deleting files
			std::cout << counter << ") Copying new file << " << item.path().filename() << '\n';
		}
	}

	std::wcin.get();
	return 0;
}