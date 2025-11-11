#ifndef MINIGIT_OBJECTS_HPP
#define MINIGIT_OBJECTS_HPP

#include "types.hpp"
#include <string>
#include <vector>

using namespace std;

namespace minigit {

/**
 * @brief Calculate SHA-1 hash of content
 * @param content The content to hash
 * @return The SHA-1 hash as a hexadecimal string
 */
string calculateHash(const string& content);

/**
 * @brief Write a blob object to the object database
 * @param filepath Path to the file
 * @return SHA-1 hash of the blob
 */
string hashObject(const string& filepath);

/**
 * @brief Read and display the contents of an object
 * @param hash The SHA-1 hash of the object
 */
void catFile(const string& hash);

/**
 * @brief Write a tree object from the current index
 * @return SHA-1 hash of the tree object
 */
string writeTree();

/**
 * @brief Write a commit object
 * @param tree_hash SHA-1 hash of the tree
 * @param parent_hash SHA-1 hash of the parent commit (empty for initial commit)
 * @param message Commit message
 * @return SHA-1 hash of the commit object
 */
string writeCommit(const string& tree_hash, const string& parent_hash, const string& message);

/**
 * @brief Read a commit object from the object database
 * @param hash SHA-1 hash of the commit
 * @return Commit object
 */
Commit readCommit(const string& hash);

/**
 * @brief Read a tree object and get the list of files
 * @param tree_hash SHA-1 hash of the tree
 * @return Vector of pairs (filepath, blob_hash)
 */
vector<pair<string, string>> readTree(const string& tree_hash);

/**
 * @brief Write an object to the object database
 * @param content The object content (with header)
 * @return SHA-1 hash of the object
 */
string writeObject(const string& content);

/**
 * @brief Read an object from the object database
 * @param hash SHA-1 hash of the object
 * @return The object content (with header)
 */
string readObject(const string& hash);

/**
 * @brief Read blob content (without header)
 * @param hash SHA-1 hash of the blob
 * @return The blob content
 */
string readBlobContent(const string& hash);

} // namespace minigit

#endif // MINIGIT_OBJECTS_HPP
