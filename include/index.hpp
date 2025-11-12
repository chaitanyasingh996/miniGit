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