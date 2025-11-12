#ifndef MINIGIT_COMMANDS_HPP
#define MINIGIT_COMMANDS_HPP

namespace minigit {

/**
 * @brief Handle the 'init' command
 */
void handleInit();

/**
 * @brief Handle the 'hash-object' command
 * @param argc Argument count
 * @param argv Argument vector
 */
void handleHashObject(int argc, char* argv[]);

/**
 * @brief Handle the 'cat-file' command
 * @param argc Argument count
 * @param argv Argument vector
 */
void handleCatFile(int argc, char* argv[]);

/**
 * @brief Handle the 'add' command
 * @param argc Argument count
 * @param argv Argument vector
 */
void handleAdd(int argc, char* argv[]);

/**
 * @brief Handle the 'commit' command
 * @param argc Argument count
 * @param argv Argument vector
 */
void handleCommit(int argc, char* argv[]);

/**
 * @brief Handle the 'log' command
 */
void handleLog();

/**
 * @brief Handle the 'status' command
 */
void handleStatus();

/**
 * @brief Handle the 'verify-tree' command
 * @param argc Argument count
 * @param argv Argument vector
 */
void handleVerifyTree(int argc, char* argv[]);

/**
 * @brief Handle the 'diff-tree' command
 * @param argc Argument count
 * @param argv Argument vector
 */
void handleDiffTree(int argc, char* argv[]);

/**
 * @brief Handle the 'verify-integrity' command
 */
void handleVerifyIntegrity();

/**
 * @brief Handle the 'compare-branches' command
 * @param argc Argument count
 * @param argv Argument vector
 */
void handleCompareBranches(int argc, char* argv[]);

} // namespace minigit

#endif // MINIGIT_COMMANDS_HPP
