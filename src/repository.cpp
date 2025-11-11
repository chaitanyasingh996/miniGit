// src/repository.cpp
// Repository initialization and management

#include "repository.hpp"
#include "index.hpp"
#include "objects.hpp"
#include "utils.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <map>
#include <vector>

using namespace std;
namespace fs = filesystem;

namespace minigit
{

    // ============================================================================
    // INDEX FILE FORMAT SPECIFICATION
    // ============================================================================
    //
    // The miniGit index file (.minigit/index) stores the staging area with the
    // following line-based format:
    //
    // Format: MODE HASH PATH
    // Example: 100644 a1b2c3d4e5f6... src/main.cpp
    //
    // Components:
    //   MODE  - File permissions in octal (e.g., "100644" for regular files)
    //   HASH  - 40-character SHA-1 hash of the blob object
    //   PATH  - File path relative to repository root (space-separated)
    //
    // Design Rationale:
    //   - Text-based for human readability and easy debugging
    //   - Line-ordered alphabetically (via std::map<string, IndexEntry>)
    //   - Each line represents one tracked file in staging area
    //   - Deterministic ordering enables reproducible tree generation
    //   - Simple space-delimited format for easy parsing
    //
    // Example Index File:
    //   100644 d8e8fca2dc0f896fd7cb4cb0031ba249 README.md
    //   100644 a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6 src/main.cpp
    //   100644 f1e2d3c4b5a69788796859585857565a src/utils.cpp
    //
    // ============================================================================

    void initRepository()
    {
        string repoPath = ".minigit";

        if (fs::exists(repoPath))
        {
            cerr << "Reinitialization of an existing miniGit repository" << endl;
            return;
        }

        try
        {
            fs::create_directories(repoPath);
            fs::create_directories(repoPath + "/objects");
            fs::create_directories(repoPath + "/refs");
            fs::create_directories(repoPath + "/refs/heads");
            fs::create_directories(repoPath + "/refs/tags");

            ofstream head_file(repoPath + "/HEAD");
            head_file << "ref: refs/heads/main\n";

            cout << "Initialized empty miniGit repository in .minigit" << endl;
        }
        catch (const exception &e)
        {
            cerr << "Error creating repository: " << e.what() << endl;
        }
    }

    string getMinigitDir()
    {
        return ".minigit";
    }

    bool isRepository()
    {
        return fs::exists(".minigit");
    }

    string getRefHash(const string &ref)
    {
        if (ref.empty())
            return "";
        string refPath = ".minigit/" + ref;

        if (!fs::exists(refPath))
        {
            return "";
        }

        ifstream file(refPath);
        string hash;
        getline(file, hash);

        return hash;
    }

    // ============================================================================
    // STATUS COMPARISON OPERATIONS
    // ============================================================================

    bool repositoryHasChanges()
    {
        // Get the tree hash from HEAD
        string head_tree = getHeadTreeHash();
        if (head_tree.empty())
        {
            // No commits yet, check if index has anything
            auto index = readIndex();
            return !index.empty();
        }

        // Get current index
        auto index = readIndex();

        // Read the tree from HEAD
        map<string, string> head_files;
        readTreeToMap(head_tree, head_files);

        // Compare index with HEAD tree
        if (index.size() != head_files.size())
        {
            return true;
        }

        for (const auto &[filepath, entry] : index)
        {
            auto it = head_files.find(filepath);
            if (it == head_files.end() || it->second != entry.hash)
            {
                return true;
            }
        }

        return false;
    }

    string getHeadTreeHash()
    {
        string commit_hash = getHeadCommit();
        if (commit_hash.empty())
        {
            return "";
        }

        Commit commit = readCommit(commit_hash);
        return commit.tree;
    }

    string getHeadCommit()
    {
        string headPath = ".minigit/HEAD";

        if (!fs::exists(headPath))
        {
            return "";
        }

        ifstream file(headPath);
        string head_content;
        getline(file, head_content);

        string ref_prefix = "ref: ";
        if (head_content.rfind(ref_prefix, 0) == 0)
        {
            return getRefHash(head_content.substr(ref_prefix.length()));
        }
        else
        {
            return head_content;
        }
    }

    vector<string> getUntrackedFiles()
    {
        vector<string> untracked;
        auto workdir_files = getWorkingDirectoryFiles();
        auto index = readIndex();

        for (const auto &file : workdir_files)
        {
            if (index.find(file) == index.end())
            {
                untracked.push_back(file);
            }
        }

        return untracked;
    }

} // namespace minigit
