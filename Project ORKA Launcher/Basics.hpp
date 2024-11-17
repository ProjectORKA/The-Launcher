#pragma once

#include <array>
#include <ctime>
#include <thread>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <filesystem>

using Bool = bool;
using SizeT = size_t;
using String = std::string;
using Thread = std::thread;
using Path = std::filesystem::path;
template <typename T>
using Vector = std::vector<T>;

inline Path getCurrentPath()
{
    return std::filesystem::current_path();
}

inline void createDirectory(const Path& path)
{
    create_directory(path);
}

inline void setCurrentPath(Path::iterator::reference path)
{
    std::cout << "Changing active path to: " << path << "\n";
    current_path(path);
}
