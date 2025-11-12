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

}

#endif // MINIGIT_COMMANDS_HPP