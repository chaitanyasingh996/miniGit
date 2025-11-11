// src/index.cpp
// Index (staging area) management

#include "index.hpp"
#include "objects.hpp"
#include <sstream>
#include <iostream>
#include <filesystem>
#include <fstream>
// Simple in-memory cache to avoid repeated file reads
static map<string, IndexEntry> g_index_cache;
static bool g_cache_loaded = false;

map<string, IndexEntry> readIndex()
{
    if (g_cache_loaded)
    {
        return g_index_cache;
    }

    map<string, IndexEntry> index;
    string indexPath = ".minigit/index";

    if (!fs::exists(indexPath))
    {
        g_index_cache = index;
        g_cache_loaded = true;
        return index;
    }

    ifstream file(indexPath);
    string line;
    while (getline(file, line))
    {
        stringstream ss(line);
        string mode, hash, filepath;
        ss >> mode >> hash >> filepath;
        index[filepath] = {mode, hash};
    }

    g_index_cache = index;
    g_cache_loaded = true;
    return index;
}

void writeIndex(const map<string, IndexEntry> &index)
{
    string indexPath = ".minigit/index";
    ofstream file(indexPath);

    for (const auto &[filepath, entry] : index)
    {
        file << entry.mode << " " << entry.hash << " " << filepath << endl;
    }

    // update cache
    g_index_cache = index;
    g_cache_loaded = true;
}

void addToIndex(const string &filepath)
{
    if (!fs::exists(filepath))
    {
        cerr << "fatal: pathspec '" << filepath << "' did not match any files" << endl;
        return;
    }

    // Use cache to reduce file I/O
    auto index = readIndex();
    string hash = hashObject(filepath);
    index[filepath] = {"100644", hash};

    // persist change
    writeIndex(index);

    cout << "Added file: \"" << filepath << "\"" << endl;
}

void clearIndex()
{
    string indexPath = ".minigit/index";
    ofstream file(indexPath);
    file << "";

    g_index_cache.clear();
    g_cache_loaded = true;
}

cout << "Added file: \"" << filepath << "\"" << endl;
}

void clearIndex()
{
    string indexPath = ".minigit/index";
    ofstream file(indexPath);
    file << "";
}

} // namespace minigit
