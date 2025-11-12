#ifndef MINIGIT_BRANCH_HPP
#define MINIGIT_BRANCH_HPP

namespace minigit {

/**
 * @brief Handle the 'branch' command
 * @param argc Argument count
 * @param argv Argument vector
 */
void handleBranch(int argc, char* argv[]);

/**
 * @brief Handle the 'switch' command
 * @param argc Argument count
 * @param argv Argument vector
 */
void handleSwitch(int argc, char* argv[]);

/**
 * @brief Handle the 'checkout' command
 * @param argc Argument count
 * @param argv Argument vector
 */
void handleCheckout(int argc, char* argv[]);

/**
 * @brief Handle the 'merge' command
 * @param argc Argument count
 * @param argv Argument vector
 */
void handleMerge(int argc, char* argv[]);

/**
 * @brief Handle the 'stash' command
 * @param argc Argument count
 * @param argv Argument vector
 */
void handleStash(int argc, char* argv[]);

} // namespace minigit

#endif // MINIGIT_BRANCH_HPP
