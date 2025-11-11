// src/index.cpp
// Index (staging area) management

#include "index.hpp"
#include "objects.hpp"
#include <sstream>
#include <iostream>
#include <filesystem>
#include <fstream>

using namespace std;
namespace fs = filesystem;

namespace minigit {

map<string, IndexEntry> readIndex() {
    map<string, IndexEntry> index;
    string indexPath = ".minigit/index";

    if (!fs::exists(indexPath)) {
        return index;
    }

    ifstream file(indexPath);
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string mode, hash, filepath;
        ss >> mode >> hash >> filepath;
        index[filepath] = {mode, hash};
    }

    return index;
}

void writeIndex(const map<string, IndexEntry>& index) {
    string indexPath = ".minigit/index";
    ofstream file(indexPath);

    for (const auto& [filepath, entry] : index) {
        file << entry.mode << " " << entry.hash << " " << filepath << endl;
    }
}

void addToIndex(const string& filepath) {
    if (!fs::exists(filepath)) {
        cerr << "fatal: pathspec '" << filepath << "' did not match any files" << endl;
        return;
    }

    auto index = readIndex();
    string hash = hashObject(filepath);
    index[filepath] = {"100644", hash};
    writeIndex(index);

    cout << "Added file: \"" << filepath << "\"" << endl;
}

void clearIndex() {
    string indexPath = ".minigit/index";
    ofstream file(indexPath);
    file << "";
}

} // namespace minigit
