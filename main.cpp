#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <exception>
#include "Duration.h"
#include "SyncFolder.h"
#include "cxxopts/include/cxxopts.hpp"

#ifdef _DEBUG
#  define LOGD(x) std::cout << x << std::endl
#else
#  define LOGD(x) do {} while (0)
#endif

#define LOG(x) std::wcout << x << L'\n';

namespace fs = std::filesystem;

int main(int argc, char** argv) {
    const std::string defaultConfigPath("settings.cfg");
    std::string cfgPath;
    std::string from; // make wide?
    std::string dest;


    cxxopts::Options options("FolderSync", "Sync your folders");
    options.add_options()
            ("c,config", "Path to config file", cxxopts::value<std::string>()->default_value(defaultConfigPath))
            ("d,dest", "Path to dest folder", cxxopts::value<std::string>())
            ("f,from", "Path to from folder", cxxopts::value<std::string>())
            ("h,help", "Show this help")
            ;
    auto result = options.parse(argc, argv);


    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        system("pause");
        return 0;
    }else if (result.count("dest") +
              result.count("from") == 2) {
        dest = result["dest"].as<std::string>();
        from = result["from"].as<std::string>();
    }else{
        cfgPath = result["config"].as<std::string>();
        LOG(L"Cfg path: " << fs::absolute(cfgPath));
        if (!fs::exists(cfgPath)) {
            std::cout << options.help() << std::endl;
            LOG(L"Cfg file not found");
            LOG(L"Exiting");
            system("pause");
            return 1;
        }
        std::ifstream settings(cfgPath);
        std::getline(settings, from);
        std::getline(settings, dest);

        LOGD("Config file 0 line: " << from);
        LOGD("Config file 1 line: " << dest);
        from = std::string(from.begin() + 6, from.end());
        dest = std::string(dest.begin() + 6, dest.end());
        settings.close();
    }

    Duration d;
    try {
        SyncFolder f(std::wstring(from.begin(), from.end()));
        SyncFolder d(std::wstring(dest.begin(), dest.end()));

        //delete disappeared files, recursively copy folder is more effectively way?
        d.copy_new(f);
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }
    LOG(L"Exiting");
    return 0;
}