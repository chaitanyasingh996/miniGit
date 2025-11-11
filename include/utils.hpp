#ifndef MINIGIT_UTILS_HPP
#define MINIGIT_UTILS_HPP

#include <string>
#include <vector>
#include <map>

using namespace std;

namespace minigit {

/**
 * @brief Calculate the hash of a file without storing it
 * @param filepath Path to the file
 * @return SHA-1 hash of the file content
 */
string getFileHash(const string& filepath);

/**
 * @brief Get all files in the working directory (excluding ignored files)
 * @return Vector of file paths
 */
vector<string> getWorkingDirectoryFiles();

/**
 * @brief Read a tree object and populate a map of files
 * @param hash SHA-1 hash of the tree
 * @param files Map to populate with filepath -> blob hash
 */
void readTreeToMap(const string& hash, map<string, string>& files);

} // namespace minigit

#endif // MINIGIT_UTILS_HPP
