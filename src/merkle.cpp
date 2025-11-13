// src/merkle.cpp
// Merkle tree implementation for efficient tree verification

#include "merkle.hpp"
#include "utils.hpp"
#include "objects.hpp"
#include "sha1.hpp"
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <sstream>

using namespace std;

namespace minigit {

namespace fs = filesystem;

// Build Merkle tree from working directory
shared_ptr<MerkleNode> MerkleTree::buildFromWorkingDirectory() {
    return buildFromDirectory(".");
}

// Build tree recursively from directory
shared_ptr<MerkleNode> MerkleTree::buildFromDirectory(const string& dir_path) {
    auto node = make_shared<MerkleNode>(dir_path, false);
    
    if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
        return node;
    }
    
    vector<fs::path> entries;
    for (const auto& entry : fs::directory_iterator(dir_path)) {
        string path_str = entry.path().string();
        // Skip .minigit directory
        if (path_str.find(".minigit") != string::npos) {
            continue;
        }
        entries.push_back(entry.path());
    }
    
    // Sort entries for consistent ordering
    sort(entries.begin(), entries.end());
    
    for (const auto& entry : entries) {
        if (fs::is_directory(entry)) {
            auto child = buildFromDirectory(entry.string());
            node->children.push_back(child);
        } else if (fs::is_regular_file(entry)) {
            auto child = make_shared<MerkleNode>(entry.string(), true);
            child->hash = getFileHash(entry.string());
            node->children.push_back(child);
        }
    }
    
    // Calculate hash for this node
    node->hash = calculateMerkleHash(node);
    
    return node;
}

// Build Merkle tree from git tree object
shared_ptr<MerkleNode> MerkleTree::buildFromTreeObject(const string& tree_hash) {
    auto node = make_shared<MerkleNode>("", false);
    
    if (tree_hash.empty()) {
        return node;
    }
    
    try {
        auto tree_entries = readTree(tree_hash);
        
        for (const auto& [filepath, blob_hash] : tree_entries) {
            auto child = make_shared<MerkleNode>(filepath, true);
            child->hash = blob_hash;
            node->children.push_back(child);
        }
        
        node->hash = calculateMerkleHash(node);
    } catch (...) {
        // If tree doesn't exist, return empty node
    }
    
    return node;
}

// Calculate Merkle hash for a node
string MerkleTree::calculateMerkleHash(shared_ptr<MerkleNode> node) {
    if (!node) {
        return "";
    }
    
    // For files, use the file content hash
    if (node->is_file && !node->hash.empty()) {
        return node->hash;
    }
    
    // For directories, combine hashes of all children
    stringstream combined;
    combined << "merkle_dir ";
    
    // Sort children by path for consistent ordering
    vector<shared_ptr<MerkleNode>> sorted_children = node->children;
    sort(sorted_children.begin(), sorted_children.end(),
         [](const shared_ptr<MerkleNode>& a, const shared_ptr<MerkleNode>& b) {
             return a->path < b->path;
         });
    
    for (const auto& child : sorted_children) {
        if (!child->hash.empty()) {
            combined << child->path << ":" << child->hash << ";";
        }
    }
    
    // Hash the combined string
    SHA1 sha1;
    sha1.update(combined.str());
    return sha1.final();
}

// Compare two Merkle trees
map<string, string> MerkleTree::compareTrees(
    shared_ptr<MerkleNode> tree1,
    shared_ptr<MerkleNode> tree2
) {
    map<string, string> changes;
    compareTreesHelper(tree1, tree2, changes);
    return changes;
}

// Helper for comparing trees recursively
void MerkleTree::compareTreesHelper(
    shared_ptr<MerkleNode> node1,
    shared_ptr<MerkleNode> node2,
    map<string, string>& changes
) {
    if (!node1 && !node2) {
        return;
    }
    
    // Build maps of children by path
    map<string, shared_ptr<MerkleNode>> map1, map2;
    
    if (node1) {
        for (const auto& child : node1->children) {
            map1[child->path] = child;
        }
    }
    
    if (node2) {
        for (const auto& child : node2->children) {
            map2[child->path] = child;
        }
    }
    
    // Find added, deleted, and modified files
    for (const auto& [path, child1] : map1) {
        if (map2.find(path) == map2.end()) {
            changes[path] = "deleted";
        } else {
            auto child2 = map2[path];
            if (child1->hash != child2->hash) {
                changes[path] = "modified";
                
                // If both are directories, recurse
                if (!child1->is_file && !child2->is_file) {
                    compareTreesHelper(child1, child2, changes);
                }
            }
        }
    }
    
    // Find added files
    for (const auto& [path, child2] : map2) {
        if (map1.find(path) == map1.end()) {
            changes[path] = "added";
        }
    }
}

// Verify tree integrity
bool MerkleTree::verifyTree(const string& root_hash, shared_ptr<MerkleNode> tree) {
    if (!tree) {
        return root_hash.empty();
    }
    
    string computed_hash = calculateMerkleHash(tree);
    return computed_hash == root_hash;
}

// Get Merkle proof for a file
vector<string> MerkleTree::getMerkleProof(
    shared_ptr<MerkleNode> tree,
    const string& filepath
) {
    vector<string> proof;
    getMerkleProofHelper(tree, filepath, proof);
    return proof;
}

// Helper for getting Merkle proof
bool MerkleTree::getMerkleProofHelper(
    shared_ptr<MerkleNode> node,
    const string& filepath,
    vector<string>& proof
) {
    if (!node) {
        return false;
    }
    
    // If this is the target file
    if (node->is_file && node->path == filepath) {
        return true;
    }
    
    // If this is a directory, search children
    if (!node->is_file) {
        for (size_t i = 0; i < node->children.size(); ++i) {
            if (getMerkleProofHelper(node->children[i], filepath, proof)) {
                // Add sibling hashes to proof
                for (size_t j = 0; j < node->children.size(); ++j) {
                    if (i != j && !node->children[j]->hash.empty()) {
                        proof.push_back(node->children[j]->hash);
                    }
                }
                return true;
            }
        }
    }
    
    return false;
}

// Verify Merkle proof
bool MerkleTree::verifyMerkleProof(
    const string& filepath,
    const string& file_hash,
    const vector<string>& proof,
    const string& root_hash
) {
    // Start with the file hash
    string current_hash = file_hash;
    
    // Combine with proof hashes
    for (const auto& sibling_hash : proof) {
        stringstream combined;
        
        // Sort hashes for consistent ordering
        if (current_hash < sibling_hash) {
            combined << current_hash << sibling_hash;
        } else {
            combined << sibling_hash << current_hash;
        }
        
        SHA1 sha1;
        sha1.update(combined.str());
        current_hash = sha1.final();
    }
    
    return current_hash == root_hash;
}

// Print tree structure
void MerkleTree::printTree(shared_ptr<MerkleNode> node, int indent) {
    if (!node) {
        return;
    }
    
    string indent_str(indent * 2, ' ');
    
    if (node->is_file) {
        cout << indent_str << "📄 " << node->path 
             << " [" << node->hash.substr(0, 8) << "...]" << endl;
    } else {
        cout << indent_str << "📁 " << node->path 
             << " [" << node->hash.substr(0, 8) << "...]" << endl;
        
        for (const auto& child : node->children) {
            printTree(child, indent + 1);
        }
    }
}

} // namespace minigit
