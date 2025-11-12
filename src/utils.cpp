// src/utils.cpp
// Utility functions

#include "utils.hpp"
#include "objects.hpp"
#include "sha1.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

using namespace std;

namespace minigit {

namespace fs = filesystem;

string getFileHash(const string& filepath) {
    ifstream file(filepath, ios::binary);
    stringstream buffer;
    buffer << file.rdbuf();
    string content = buffer.str();

    string blob = "blob ";
    blob += to_string(content.length());
    blob += '\0';
    blob += content;

    SHA1 sha1;
    sha1.update(blob);
    return sha1.final();
}

void readTreeToMap(const string& hash, map<string, string>& files) {
    string raw_content = readObject(hash);
    
    if (raw_content.empty()) {
        return;
    }
    
    size_t null_pos = raw_content.find('\0');
    if (null_pos == string::npos) {
        return;
    }
    
    string content = raw_content.substr(null_pos + 1);
    stringstream content_stream(content);
    string line;
    
    while (getline(content_stream, line)) {
        stringstream line_stream(line);
        string mode, type, file_hash, filepath;
        line_stream >> mode >> type >> file_hash >> filepath;
        files[filepath] = file_hash;
    }
}

vector<string> getWorkingDirectoryFiles() {
    vector<string> files;
    vector<string> ignore_patterns;
    fs::path ignore_path = ".minigitignore";
    
    if (fs::exists(ignore_path)) {
        ifstream ignore_file(ignore_path);
        string line;
        while (getline(ignore_file, line)) {
            ignore_patterns.push_back(line);
        }
    }

    for (const auto& entry : fs::recursive_directory_iterator(".")) {
        bool ignored = false;
        string path_str = entry.path().string();
        if (path_str.rfind("./", 0) == 0) {
            path_str = path_str.substr(2);
        }

        for (const auto& pattern : ignore_patterns) {
            if (path_str.rfind(pattern, 0) == 0) {
                ignored = true;
                break;
            }
        }
        
        if (entry.is_regular_file() && !ignored && path_str.rfind(".minigit", 0) != 0) {
            files.push_back(path_str);
        }
    }
    
    return files;
}

} // namespace minigit
