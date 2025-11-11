// objects.cpp - Commit 5 Version (60% complete - write functions only)
// Member 1: Object Storage System - Write Operations

#include "objects.hpp"
#include "sha1.hpp"
#include "utils.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;
namespace fs = filesystem;

// Get the .minigit directory path
string getMinigitDir() {
    return ".minigit";
}

// Write raw content to object storage and return its hash
string writeObject(const string& content) {
    // Calculate SHA-1 hash of content
    SHA1 sha1;
    sha1.update(content);
    string hash = sha1.final();
    
    // Create object path: .minigit/objects/ab/cdef123456...
    string objects_dir = getMinigitDir() + "/objects";
    string dir_path = objects_dir + "/" + hash.substr(0, 2);
    string file_path = dir_path + "/" + hash.substr(2);
    
    // Skip if object already exists (content-addressable storage deduplication)
    if (fs::exists(file_path)) {
        return hash;
    }
    
    // Create directory structure if needed
    fs::create_directories(dir_path);
    
    // Write content to object file
    ofstream ofs(file_path);
    if (!ofs) {
        cerr << "Error: Failed to write object " << hash << endl;
        return "";
    }
    ofs << content;
    ofs.close();
    
    return hash;
}

// Hash a file and store it as a blob object
string hashObject(const string& filepath) {
    // Read file content
    ifstream ifs(filepath, ios::binary);
    if (!ifs) {
        cerr << "Error: Cannot read file " << filepath << endl;
        return "";
    }
    
    stringstream buffer;
    buffer << ifs.rdbuf();
    string content = buffer.str();
    ifs.close();
    
    // Create blob object format: "blob <size>\0<content>"
    string blob_content = "blob " + to_string(content.size()) + '\0' + content;
    
    // Write to object storage
    return writeObject(blob_content);
}

// Create a tree object from the current index
string writeTree() {
    string index_path = getMinigitDir() + "/index";
    
    // Check if index exists
    if (!fs::exists(index_path)) {
        return "";
    }
    
    // Read index entries
    ifstream ifs(index_path);
    if (!ifs) {
        cerr << "Error: Cannot read index" << endl;
        return "";
    }
    
    // Build tree content from index
    stringstream tree_content;
    string line;
    while (getline(ifs, line)) {
        if (line.empty()) continue;
        
        // Index format: mode hash path
        stringstream ss(line);
        string mode, hash, path;
        ss >> mode >> hash;
        getline(ss, path);
        if (!path.empty() && path[0] == ' ') {
            path = path.substr(1);
        }
        
        // Tree entry format: "mode path\0hash"
        tree_content << mode << " " << path << '\0' << hash << '\n';
    }
    ifs.close();
    
    string tree_data = tree_content.str();
    if (tree_data.empty()) {
        return "";
    }
    
    // Create tree object format: "tree <size>\0<entries>"
    string tree_object = "tree " + to_string(tree_data.size()) + '\0' + tree_data;
    
    // Write to object storage
    return writeObject(tree_object);
}

// Create a commit object
string writeCommit(const string& tree_hash, const string& parent_hash, const string& message) {
    // Build commit content
    stringstream commit_content;
    commit_content << "tree " << tree_hash << "\n";
    
    if (!parent_hash.empty()) {
        commit_content << "parent " << parent_hash << "\n";
    }
    
    // Get current timestamp and author info
    auto now = time(nullptr);
    commit_content << "author miniGit User <user@minigit.local> " << now << " +0000\n";
    commit_content << "committer miniGit User <user@minigit.local> " << now << " +0000\n";
    commit_content << "\n";
    commit_content << message << "\n";
    
    string commit_data = commit_content.str();
    
    // Create commit object format: "commit <size>\0<content>"
    string commit_object = "commit " + to_string(commit_data.size()) + '\0' + commit_data;
    
    // Write to object storage
    return writeObject(commit_object);
}
