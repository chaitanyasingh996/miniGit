#ifndef MINIGIT_REPOSITORY_HPP
#define MINIGIT_REPOSITORY_HPP

#include <string>

using namespace std;

namespace minigit
{
    /**
     * @brief Check if repository has uncommitted changes
     * @return true if index differs from HEAD tree, false otherwise
     */
    bool repositoryHasChanges();

    /**
     * @brief Get the tree hash from current HEAD commit
     * @return Tree hash or empty string if no commits exist
     */
    string getHeadTreeHash();

    /**
     * @brief Get the current HEAD commit hash
     * @return Commit hash or empty string if no commits exist
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
     * @brief Get list of untracked files in working directory
     * @return Vector of file paths not in index
     */
    vector<string> getUntrackedFiles();

    /**
     * @brief Initialize a new miniGit repository
     */
    void initRepository();

    /**
     * @brief Get the path to the .minigit directory
     * @return Path to .minigit directory
     */
    string getMinigitDir();

    /**
     * @brief Check if a miniGit repository exists (internal check)
     * @return true if .minigit directory exists, false otherwise
     */
    bool isRepository();

    /**
     * @brief Get hash of a reference (branch or tag)
     * @param ref Reference path (e.g., "refs/heads/main")
     * @return Hash or empty string if not found
     */
    string getRefHash(const string &ref);

} // namespace minigit

#endif // MINIGIT_REPOSITORY_HPP
