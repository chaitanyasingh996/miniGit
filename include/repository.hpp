#ifndef MINIGIT_REPOSITORY_HPP
#define MINIGIT_REPOSITORY_HPP

#include <string>

using namespace std;

namespace minigit
{

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
    string getRefHash(const string& ref);

} // namespace minigit

#endif // MINIGIT_REPOSITORY_HPP
