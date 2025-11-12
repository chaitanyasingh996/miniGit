#ifndef MINIGIT_MERKLE_HPP
#define MINIGIT_MERKLE_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>

using namespace std;

namespace minigit {

/**
 * @brief Represents a node in the Merkle tree
 */
struct MerkleNode {
    string hash;              // Hash of this node
    string path;              // File/directory path
    bool is_file;             // True if file, false if directory
    vector<shared_ptr<MerkleNode>> children;  // Child nodes (for directories)
    
    MerkleNode(const string& p, bool file = true) 
        : path(p), is_file(file) {}
};

/**
 * @brief Merkle tree for efficient directory structure verification
 */
class MerkleTree {
public:
    /**
     * @brief Construct a Merkle tree from current working directory
     */
    static shared_ptr<MerkleNode> buildFromWorkingDirectory();
    
    /**
     * @brief Construct a Merkle tree from a git tree object
     * @param tree_hash SHA-1 hash of the tree object
     */
    static shared_ptr<MerkleNode> buildFromTreeObject(const string& tree_hash);
    
    /**
     * @brief Calculate the Merkle root hash for a node and its children
     * @param node The node to calculate hash for
     * @return The computed hash
     */
    static string calculateMerkleHash(shared_ptr<MerkleNode> node);
    
    /**
     * @brief Compare two Merkle trees and find differences
     * @param tree1 First tree
     * @param tree2 Second tree
     * @return Map of changed paths with their status (modified/added/deleted)
     */
    static map<string, string> compareTrees(
        shared_ptr<MerkleNode> tree1, 
        shared_ptr<MerkleNode> tree2
    );
    
    /**
     * @brief Verify integrity of a directory tree using Merkle root
     * @param root_hash Expected Merkle root hash
     * @param tree The tree to verify
     * @return True if tree matches the hash
     */
    static bool verifyTree(const string& root_hash, shared_ptr<MerkleNode> tree);
    
    /**
     * @brief Get the Merkle proof path for a specific file
     * @param tree The tree
     * @param filepath Path to the file
     * @return Vector of hashes forming the proof path
     */
    static vector<string> getMerkleProof(
        shared_ptr<MerkleNode> tree, 
        const string& filepath
    );
    
    /**
     * @brief Verify a file's inclusion in the tree using Merkle proof
     * @param filepath The file path
     * @param file_hash Hash of the file
     * @param proof The Merkle proof path
     * @param root_hash The Merkle root hash
     * @return True if proof is valid
     */
    static bool verifyMerkleProof(
        const string& filepath,
        const string& file_hash,
        const vector<string>& proof,
        const string& root_hash
    );
    
    /**
     * @brief Print the Merkle tree structure (for debugging)
     * @param node Root node
     * @param indent Indentation level
     */
    static void printTree(shared_ptr<MerkleNode> node, int indent = 0);
    
private:
    /**
     * @brief Build tree recursively from working directory
     * @param dir_path Directory path
     * @return Merkle node for the directory
     */
    static shared_ptr<MerkleNode> buildFromDirectory(const string& dir_path);
    
    /**
     * @brief Helper for comparing trees recursively
     */
    static void compareTreesHelper(
        shared_ptr<MerkleNode> node1,
        shared_ptr<MerkleNode> node2,
        map<string, string>& changes
    );
    
    /**
     * @brief Helper for getting Merkle proof
     */
    static bool getMerkleProofHelper(
        shared_ptr<MerkleNode> node,
        const string& filepath,
        vector<string>& proof
    );
};

} // namespace minigit

#endif // MINIGIT_MERKLE_HPP
