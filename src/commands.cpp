// src/commands.cpp
// Main command handlers

#include "commands.hpp"
#include "repository.hpp"
#include "objects.hpp"
#include "merkle.hpp"
#include "index.hpp"
#include "utils.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <chrono>

using namespace std;
namespace fs = filesystem;

namespace minigit
{

    void handleInit()
    {
        initRepository();
    }

    void handleHashObject(int argc, char *argv[])
    {
        if (argc < 3)
        {
            cerr << "Usage: miniGit hash-object <file>" << endl;
            return;
        }

        string filePath = argv[2];

        // Check if file exists in real filesystem
        if (!fs::exists(filePath))
        {
            cerr << "Error: File not found: " << filePath << endl;
            return;
        }

        cout << getFileHash(filePath) << endl;
    }

    void handleCatFile(int argc, char *argv[])
    {
        if (argc < 3)
        {
            cerr << "Usage: miniGit cat-file <hash>" << endl;
            return;
        }

        string hash = argv[2];
        catFile(hash);
    }

    void handleAdd(int argc, char *argv[])
    {
        if (argc < 3)
        {
            cerr << "Usage: miniGit add <file>" << endl;
            return;
        }

        string filePath = argv[2];

        // Check if file exists in real filesystem
        if (!fs::exists(filePath))
        {
            cerr << "fatal: pathspec '" << filePath << "' did not match any files" << endl;
            return;
        }

        string hash = hashObject(filePath);

        // Add the file to the index
        auto index = readIndex();
        index[filePath] = {"100644", hash};
        writeIndex(index);

        cout << "Added file: \"" << filePath << "\"" << endl;
    }

    void handleCommit(int argc, char *argv[])
    {
        if (argc < 4 || string(argv[2]) != "-m")
        {
            cerr << "Usage: miniGit commit -m <message>" << endl;
            return;
        }

        string message = argv[3];

        // Create a tree object
        string tree_hash = writeTree();
        if (tree_hash.empty())
        {
            cerr << "Error: Failed to write tree." << endl;
            return;
        }

        // Get the parent commit
        string parent_hash = getHeadCommit();

        // Create the commit object
        string commit_hash = writeCommit(tree_hash, parent_hash, message);
        if (commit_hash.empty())
        {
            cerr << "Error: Failed to write commit." << endl;
            return;
        }

        // Update the current branch ref or detached HEAD
        string headPath = ".minigit/HEAD";
        if (!fs::exists(headPath))
        {
            cerr << "Error: HEAD file not found." << endl;
            return;
        }

        ifstream head_file(headPath);
        string head_content;
        getline(head_file, head_content);
        head_file.close();

        string ref_prefix = "ref: ";
        if (head_content.rfind(ref_prefix, 0) == 0)
        {
            // Update branch reference
            string ref_path = head_content.substr(ref_prefix.length());
            string refPath = ".minigit/" + ref_path;
            ofstream ref_file(refPath);
            ref_file << commit_hash << "\n";

            // Extract branch name
            string branch_name = ref_path;
            if (branch_name.rfind("refs/heads/", 0) == 0)
            {
                branch_name = branch_name.substr(11); // strlen("refs/heads/")
            }
            cout << "[" << branch_name << " " << commit_hash.substr(0, 7) << "] " << message << endl;
        }
        else
        {
            // Detached HEAD - update HEAD directly
            ofstream head_file_write(headPath);
            head_file_write << commit_hash << "\n";
            cout << "[detached HEAD " << commit_hash.substr(0, 7) << "] " << message << endl;
        }
    }

    void handleLog()
    {
        string current_hash = getHeadCommit();

        while (!current_hash.empty())
        {
            Commit commit = readCommit(current_hash);

            cout << "commit " << current_hash << endl;

            // Parse author line to extract name/email and timestamp
            string authorInfo = commit.author;
            string timestamp;
            size_t timestampPos = authorInfo.find_last_of('>');
            if (timestampPos != string::npos && timestampPos + 1 < authorInfo.length())
            {
                timestamp = authorInfo.substr(timestampPos + 1);
                authorInfo = authorInfo.substr(0, timestampPos + 1);
                // Trim whitespace from timestamp
                timestamp.erase(0, timestamp.find_first_not_of(" \t"));
            }

            cout << "Author: " << authorInfo << endl;
            cout << "Date:   " << timestamp << endl;
            cout << endl;
            cout << "    " << commit.message << endl;

            current_hash = commit.parent;

            // Add blank line between commits
            if (!current_hash.empty())
            {
                cout << endl;
            }
        }
    }

    void handleStatus()
    {
        auto index = readIndex();

        string head_commit_hash = getHeadCommit();

        // If HEAD is empty there are no commits yet. Avoid calling readCommit
        // with an empty hash and treat the repository as having no tracked
        // files for the purposes of status.
        map<string, string> head_files;
        if (!head_commit_hash.empty())
        {
            Commit head_commit = readCommit(head_commit_hash);
            if (!head_commit.tree.empty())
            {
                readTreeToMap(head_commit.tree, head_files);
            }
        }

        string current_branch = getCurrentBranch();
        if (current_branch.empty())
        {
            if (head_commit_hash.empty())
            {
                cout << "No commits yet" << endl;
            }
            else
            {
                cout << "HEAD detached at " << head_commit_hash.substr(0, 7) << endl;
            }
        }
        else
        {
            cout << "On branch " << current_branch << endl;
        }

        cout << "Changes to be committed:" << endl;

        for (const auto &[filepath, entry] : index)
        {
            if (head_files.find(filepath) == head_files.end())
            {
                cout << "\tnew file:   " << filepath << endl;
            }
            else if (head_files[filepath] != entry.hash)
            {
                cout << "\tmodified:   " << filepath << endl;
            }
        }

        for (const auto &[filepath, hash] : head_files)
        {
            if (index.find(filepath) == index.end())
            {
                cout << "\tdeleted:    " << filepath << endl;
            }
        }

        cout << endl;
        cout << "Changes not staged for commit:" << endl;

        auto working_dir_files = getWorkingDirectoryFiles();
        for (const auto &filepath : working_dir_files)
        {
            if (index.find(filepath) != index.end())
            {
                if (getFileHash(filepath) != index[filepath].hash)
                {
                    cout << "\tmodified:   " << filepath << endl;
                }
            }
        }

        cout << endl;
        cout << "Untracked files:" << endl;

        for (const auto &filepath : working_dir_files)
        {
            if (index.find(filepath) == index.end())
            {
                cout << "\t" << filepath << endl;
            }
        }

        cout << endl;
    }

    void handleVerifyTree(int argc, char *argv[])
    {
        if (argc < 3)
        {
            cerr << "Usage: miniGit verify-tree [--working-dir | <tree-hash>]" << endl;
            cerr << "  --working-dir : Verify current working directory" << endl;
            cerr << "  <tree-hash>   : Verify a specific tree object" << endl;
            return;
        }

        string arg = argv[2];

        if (arg == "--working-dir")
        {
            // Build and display Merkle tree from working directory
            cout << "Building Merkle tree from working directory..." << endl;
            auto tree = MerkleTree::buildFromWorkingDirectory();

            cout << "\nMerkle Tree Structure:" << endl;
            MerkleTree::printTree(tree);

            cout << "\nMerkle Root Hash: " << tree->hash << endl;

            // Verify integrity
            bool valid = MerkleTree::verifyTree(tree->hash, tree);
            cout << "Tree Integrity: " << (valid ? "✓ VALID" : "✗ INVALID") << endl;
        }
        else
        {
            // Verify a tree object
            string tree_hash = arg;
            cout << "Building Merkle tree from tree object: " << tree_hash << endl;

            try
            {
                auto tree = MerkleTree::buildFromTreeObject(tree_hash);

                cout << "\nMerkle Tree Structure:" << endl;
                MerkleTree::printTree(tree);

                cout << "\nMerkle Root Hash: " << tree->hash << endl;

                bool valid = MerkleTree::verifyTree(tree->hash, tree);
                cout << "Tree Integrity: " << (valid ? "✓ VALID" : "✗ INVALID") << endl;
            }
            catch (const exception &e)
            {
                cerr << "Error: " << e.what() << endl;
            }
        }
    }

    void handleDiffTree(int argc, char *argv[])
    {
        if (argc < 4)
        {
            cerr << "Usage: miniGit diff-tree <tree-hash-1> <tree-hash-2>" << endl;
            cerr << "       miniGit diff-tree --working-dir <commit-hash>" << endl;
            cerr << "  Compare two tree objects or working directory with a commit" << endl;
            return;
        }

        string arg1 = argv[2];
        string arg2 = argv[3];

        shared_ptr<MerkleNode> tree1, tree2;

        if (arg1 == "--working-dir")
        {
            cout << "Comparing working directory with commit " << arg2 << endl;
            tree1 = MerkleTree::buildFromWorkingDirectory();

            // Get tree hash from commit
            try
            {
                auto commit = readCommit(arg2);
                tree2 = MerkleTree::buildFromTreeObject(commit.tree);
            }
            catch (...)
            {
                cerr << "Error: Invalid commit hash" << endl;
                return;
            }
        }
        else
        {
            cout << "Comparing tree " << arg1 << " with tree " << arg2 << endl;
            tree1 = MerkleTree::buildFromTreeObject(arg1);
            tree2 = MerkleTree::buildFromTreeObject(arg2);
        }

        // Quick check using Merkle root
        if (tree1->hash == tree2->hash)
        {
            cout << "\n✓ Trees are identical (Merkle roots match)" << endl;
            cout << "Merkle Root: " << tree1->hash << endl;
            return;
        }

        cout << "\n⚠ Trees differ (Merkle roots don't match)" << endl;
        cout << "Tree 1 Root: " << tree1->hash << endl;
        cout << "Tree 2 Root: " << tree2->hash << endl;

        // Find detailed differences
        cout << "\nDetailed differences:" << endl;
        auto changes = MerkleTree::compareTrees(tree1, tree2);

        if (changes.empty())
        {
            cout << "  (No file-level changes detected)" << endl;
        }
        else
        {
            for (const auto &[path, status] : changes)
            {
                if (status == "added")
                {
                    cout << "  A " << path << " (added)" << endl;
                }
                else if (status == "deleted")
                {
                    cout << "  D " << path << " (deleted)" << endl;
                }
                else if (status == "modified")
                {
                    cout << "  M " << path << " (modified)" << endl;
                }
            }
        }

        cout << "\nTotal changes: " << changes.size() << endl;
    }

    void handleVerifyIntegrity()
    {
        if (!verifyRepositoryIntegrity())
        {
            exit(1);
        }
    }

    void handleCompareBranches(int argc, char *argv[])
    {
        if (argc < 4)
        {
            cerr << "Usage: miniGit compare-branches <branch1> <branch2>" << endl;
            return;
        }

        string branch1 = argv[2];
        string branch2 = argv[3];

        string root1 = getBranchMerkleRoot(branch1);
        string root2 = getBranchMerkleRoot(branch2);

        if (root1.empty())
        {
            cerr << "Error: Branch '" << branch1 << "' not found" << endl;
            return;
        }

        if (root2.empty())
        {
            cerr << "Error: Branch '" << branch2 << "' not found" << endl;
            return;
        }

        cout << "Comparing branches using Merkle tree roots:" << endl;
        cout << "  " << branch1 << ": " << root1 << endl;
        cout << "  " << branch2 << ": " << root2 << endl;
        cout << endl;

        if (branchesIdentical(branch1, branch2))
        {
            cout << "✓ Branches are IDENTICAL (same content)" << endl;
        }
        else
        {
            cout << "✗ Branches are DIFFERENT" << endl;
            cout << "\nShowing file differences:" << endl;

            // Get files from both branches
            map<string, string> files1, files2;
            readTreeToMap(root1, files1);
            readTreeToMap(root2, files2);

            // Find differences
            bool has_diff = false;
            for (const auto &[path, hash] : files1)
            {
                auto it = files2.find(path);
                if (it == files2.end())
                {
                    cout << "  - " << path << " (only in " << branch1 << ")" << endl;
                    has_diff = true;
                }
                else if (it->second != hash)
                {
                    cout << "  M " << path << " (modified)" << endl;
                    has_diff = true;
                }
            }

            for (const auto &[path, hash] : files2)
            {
                if (files1.find(path) == files1.end())
                {
                    cout << "  + " << path << " (only in " << branch2 << ")" << endl;
                    has_diff = true;
                }
            }

            if (!has_diff)
            {
                cout << "  (No file differences, but tree structure differs)" << endl;
            }
        }
    }

} // namespace minigit
