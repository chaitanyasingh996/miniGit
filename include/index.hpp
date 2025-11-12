#ifndef MINIGIT_INDEX_HPP
#define MINIGIT_INDEX_HPP

#include "types.hpp"
#include <map>
#include <string>

using namespace std;

namespace minigit {

/**
 * @brief Read the index file
 * @return Map of filepath to IndexEntry
 */
map<string, IndexEntry> readIndex();

/**
 * @brief Write the index file
 * @param index Map of filepath to IndexEntry
 */
void writeIndex(const map<string, IndexEntry>& index);

/**
 * @brief Add a file to the index
 * @param filepath Path to the file to add
 */
void addToIndex(const string& filepath);

/**
 * @brief Clear the index
 */
void clearIndex();

} // namespace minigit

#endif // MINIGIT_INDEX_HPP
