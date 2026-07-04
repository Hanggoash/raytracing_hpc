#ifndef RAYTRACING_HPC_FILE_UTILS_H
#define RAYTRACING_HPC_FILE_UTILS_H

#include <cerrno>
#include <string>

#ifdef _WIN32
#include <direct.h>
#include <sys/stat.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

inline std::string normalizeSeparators(std::string path) {
    for (char& ch : path) {
        if (ch == '\\') {
            ch = '/';
        }
    }
    return path;
}

inline bool directoryExists(const std::string& directory) {
    if (directory.empty()) {
        return true;
    }

#ifdef _WIN32
    struct _stat info;
    if (_stat(directory.c_str(), &info) != 0) {
        return false;
    }
    return (info.st_mode & _S_IFDIR) != 0;
#else
    struct stat info;
    if (stat(directory.c_str(), &info) != 0) {
        return false;
    }
    return S_ISDIR(info.st_mode);
#endif
}

inline bool createSingleDirectory(const std::string& directory) {
    if (directory.empty() || directoryExists(directory)) {
        return true;
    }

#ifdef _WIN32
    int code = _mkdir(directory.c_str());
#else
    int code = mkdir(directory.c_str(), 0755);
#endif

    return code == 0 || errno == EEXIST;
}

inline bool ensureDirectory(const std::string& rawDirectory) {
    std::string directory = normalizeSeparators(rawDirectory);
    if (directory.empty() || directoryExists(directory)) {
        return true;
    }

    std::string current;
    std::size_t index = 0;

    if (directory.size() >= 2 && directory[1] == ':') {
        current = directory.substr(0, 2);
        index = 2;
        if (directory.size() > 2 && directory[2] == '/') {
            current += '/';
            index = 3;
        }
    } else if (!directory.empty() && directory[0] == '/') {
        current = "/";
        index = 1;
    }

    while (index <= directory.size()) {
        std::size_t nextSlash = directory.find('/', index);
        std::string part = directory.substr(index, nextSlash - index);

        if (!part.empty()) {
            if (!current.empty() && current.back() != '/') {
                current += '/';
            }
            current += part;
            if (!createSingleDirectory(current)) {
                return false;
            }
        }

        if (nextSlash == std::string::npos) {
            break;
        }
        index = nextSlash + 1;
    }

    return directoryExists(directory);
}

inline std::string parentDirectory(const std::string& rawPath) {
    std::string path = normalizeSeparators(rawPath);
    std::size_t slash = path.find_last_of('/');
    if (slash == std::string::npos) {
        return "";
    }
    return path.substr(0, slash);
}

inline bool ensureParentDirectory(const std::string& filePath) {
    return ensureDirectory(parentDirectory(filePath));
}

#endif
