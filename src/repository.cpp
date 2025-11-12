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

bool verifyCommit(const string& commit_hash) {
    if (commit_hash.empty()) {
        return true;
    }
    
    // Read the commit object
    string commit_content = readObject(commit_hash);
    if (commit_content.empty()) {
        cerr << "Error: Commit object " << commit_hash << " not found" << endl;
        return false;
    }
    
    // Verify commit hash
    string calculated_hash = calculateHash(commit_content);
    if (calculated_hash != commit_hash) {
        cerr << "Error: Commit " << commit_hash << " hash mismatch!" << endl;
        cerr << "  Expected: " << commit_hash << endl;
        cerr << "  Got:      " << calculated_hash << endl;
        return false;
    }
    
    // Parse the commit to get tree hash
    Commit commit = readCommit(commit_hash);
    if (commit.tree.empty()) {
        cerr << "Error: Commit " << commit_hash << " has no tree" << endl;
        return false;
    }
    
    // Verify tree exists and is valid
    string tree_content = readObject(commit.tree);
    if (tree_content.empty()) {
        cerr << "Error: Tree " << commit.tree << " not found" << endl;
        return false;
    }
    
    // Verify tree hash
    string calculated_tree_hash = calculateHash(tree_content);
    if (calculated_tree_hash != commit.tree) {
        cerr << "Error: Tree " << commit.tree << " hash mismatch!" << endl;
        return false;
    }
    
    // Verify all blobs in the tree
    map<string, string> files;
    readTreeToMap(commit.tree, files);
    
    for (const auto& [filepath, blob_hash] : files) {
        string blob_content = readObject(blob_hash);
        if (blob_content.empty()) {
            cerr << "Error: Blob " << blob_hash << " for file " << filepath << " not found" << endl;
            return false;
        }
        
        // Verify blob hash
        string calculated_blob_hash = calculateHash(blob_content);
        if (calculated_blob_hash != blob_hash) {
            cerr << "Error: Blob " << blob_hash << " for file " << filepath << " hash mismatch!" << endl;
            return false;
        }
    }
    
    return true;
}

bool verifyRepositoryIntegrity() {
    cout << "Verifying repository integrity..." << endl;
    
    string head_commit = getHeadCommit();
    if (head_commit.empty()) {
        cout << "No commits to verify." << endl;
        return true;
    }
    
    int commits_verified = 0;
    int objects_verified = 0;
    string current_hash = head_commit;
    
    while (!current_hash.empty()) {
        cout << "Verifying commit " << current_hash.substr(0, 7) << "..." << endl;
        
        if (!verifyCommit(current_hash)) {
            cerr << "✗ Repository integrity check FAILED!" << endl;
            return false;
        }
        
        commits_verified++;
        
        // Count objects in this commit
        Commit commit = readCommit(current_hash);
        map<string, string> files;
        readTreeToMap(commit.tree, files);
        objects_verified += files.size() + 1; // files + tree
        
        // Move to parent
        current_hash = commit.parent;
    }
    
    cout << "✓ Repository integrity verified!" << endl;
    cout << "  Commits verified: " << commits_verified << endl;
    cout << "  Objects verified: " << objects_verified << endl;
    
    return true;
}

} // namespace minigit
