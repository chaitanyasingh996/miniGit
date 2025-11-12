#ifndef MINIGIT_REPOSITORY_HPP
#define MINIGIT_REPOSITORY_HPP

#include <string>

using namespace std;

namespace minigit {

/**
 * @brief Initialize a new miniGit repository
 */
void initRepository();

/**
 * @brief Get the current HEAD commit hash
 * @return The commit hash, or empty string if no commits exist
 */
string getHeadCommit();

/**
 * @brief Get the current branch name
 * @return The branch name, or empty string if in detached HEAD state
 */
string getCurrentBranch();

/**
 * @brief Check if HEAD is in detached state
 * @return true if detached, false if on a branch
 */
bool isDetachedHead();

/**
 * @brief Get the path to the .minigit directory
 * @return Path to .minigit directory
 */
string getMinigitDir();

/**
 * @brief Get hash of a reference (branch or tag)
 * @param ref Reference path (e.g., "refs/heads/main")
 * @return Hash or empty string if not found
 */
string getRefHash(const string& ref);

/**
 * @brief Check if working directory has uncommitted changes
 * @return true if changes detected, false otherwise
 */
bool repositoryHasChanges();

/**
 * @brief Get Merkle tree root hash for current HEAD
 * @return Tree hash or empty string if no commits
 */
string getHeadTreeHash();

/**
 * @brief Compare two branches by their Merkle tree roots
 * @param branch1 First branch name
 * @param branch2 Second branch name
 * @return true if branches have identical content, false otherwise
 */
bool branchesIdentical(const string& branch1, const string& branch2);

/**
 * @brief Get Merkle root hash for a specific branch
 * @param branch_name Branch name
 * @return Tree hash or empty string if branch doesn't exist
 */
string getBranchMerkleRoot(const string& branch_name);

/**
 * @brief Verify integrity of entire repository
 * @return true if all hashes valid, false if corruption detected
 */
bool verifyRepositoryIntegrity();

/**
 * @brief Verify integrity of a single commit and its tree
 * @param commit_hash Commit hash to verify
 * @return true if valid, false if corrupted
 */
bool verifyCommit(const string& commit_hash);

} // namespace minigit

#endif // MINIGIT_REPOSITORY_HPP
