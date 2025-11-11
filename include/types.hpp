#ifndef MINIGIT_TYPES_HPP
#define MINIGIT_TYPES_HPP

#include <string>

using namespace std;

namespace minigit {

/**
 * @brief Represents an entry in the index (staging area)
 */
struct IndexEntry {
    string mode;  // File permissions (e.g., "100644")
    string hash;  // SHA-1 hash of the blob object
};

/**
 * @brief Represents a commit object
 */
struct Commit {
    string tree;      // SHA-1 hash of the tree object
    string parent;    // SHA-1 hash of the parent commit (empty for initial commit)
    string author;    // Author name, email, and timestamp
    string committer; // Committer name, email, and timestamp
    string message;   // Commit message
};

} // namespace minigit

#endif // MINIGIT_TYPES_HPP
