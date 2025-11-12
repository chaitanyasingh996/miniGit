// src/repository.cpp
// Repository initialization and management

#include "repository.hpp"
#include "objects.hpp"
#include "index.hpp"
#include "utils.hpp"
#include <iostream>
#include <sstream>
#include <filesystem>
#include <fstream>

using namespace std;
namespace fs = filesystem;

namespace minigit {

void initRepository() {
    string repoPath = ".minigit";

    if (fs::exists(repoPath)) {
        cerr << "Reinitialization of an existing miniGit repository" << endl;
        return;
    }

    try {
        fs::create_directories(repoPath);
        fs::create_directories(repoPath + "/objects");
        fs::create_directories(repoPath + "/refs");
        fs::create_directories(repoPath + "/refs/heads");
        fs::create_directories(repoPath + "/refs/tags");

        ofstream head_file(repoPath + "/HEAD");
        head_file << "ref: refs/heads/main\n";

        cout << "Initialized empty miniGit repository in .minigit" << endl;

    } catch (const exception& e) {
        cerr << "Error creating repository: " << e.what() << endl;
    }
}

string getRefHash(const string& ref) {
    if (ref.empty()) return "";
    string refPath = ".minigit/" + ref;
    
    if (!fs::exists(refPath)) {
        return "";
    }

    ifstream file(refPath);
    string hash;
    getline(file, hash);

    return hash;
}

string getCurrentBranch() {
    string headPath = ".minigit/HEAD";
    
    if (!fs::exists(headPath)) {
        return "";
    }

    ifstream file(headPath);
    string head_content;
    getline(file, head_content);

    string ref_prefix = "ref: refs/heads/";
    if (head_content.rfind(ref_prefix, 0) == 0) {
        return head_content.substr(ref_prefix.length());
    }
    return ""; // Detached HEAD
}

bool isDetachedHead() {
    string headPath = ".minigit/HEAD";
    
    if (!fs::exists(headPath)) {
        return false;
    }

    ifstream file(headPath);
    string head_content;
    getline(file, head_content);

    return head_content.rfind("ref: ", 0) != 0;
}

string getHeadCommit() {
    string headPath = ".minigit/HEAD";
    
    if (!fs::exists(headPath)) {
        return "";
    }

    ifstream file(headPath);
    string head_content;
    getline(file, head_content);

    string ref_prefix = "ref: ";
    if (head_content.rfind(ref_prefix, 0) == 0) {
        return getRefHash(head_content.substr(ref_prefix.length()));
    } else {
        return head_content;
    }
}

string getMinigitDir() {
    return ".minigit";
}

// ============================================================================
// MERKLE TREE OPERATIONS
// ============================================================================

string getHeadTreeHash() {
    string commit_hash = getHeadCommit();
    if (commit_hash.empty()) {
        return "";
    }
    
    Commit commit = readCommit(commit_hash);
    return commit.tree;
}

bool repositoryHasChanges() {
    // Get the tree hash from HEAD
    string head_tree = getHeadTreeHash();
    if (head_tree.empty()) {
        // No commits yet, check if index has anything
        auto index = readIndex();
        return !index.empty();
    }
    
    // Get current index
    auto index = readIndex();
    
    // Read the tree from HEAD
    map<string, string> head_files;
    readTreeToMap(head_tree, head_files);
    
    // Compare index with HEAD tree
    if (index.size() != head_files.size()) {
        return true;
    }
    
    for (const auto& [filepath, entry] : index) {
        auto it = head_files.find(filepath);
        if (it == head_files.end() || it->second != entry.hash) {
            return true;
        }
    }
    
    return false;
}

string getBranchMerkleRoot(const string& branch_name) {
    string commit_hash = getRefHash("refs/heads/" + branch_name);
    if (commit_hash.empty()) {
        return "";
    }
    
    Commit commit = readCommit(commit_hash);
    return commit.tree;
}

bool branchesIdentical(const string& branch1, const string& branch2) {
    string root1 = getBranchMerkleRoot(branch1);
    string root2 = getBranchMerkleRoot(branch2);
    
    if (root1.empty() || root2.empty()) {
        return false;
    }
    
    return root1 == root2;
}