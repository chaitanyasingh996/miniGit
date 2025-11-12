// src/branch.cpp
// Branch management commands

#include "branch.hpp"
#include "repository.hpp"
#include "objects.hpp"
#include "index.hpp"
#include "utils.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <set>
#include <algorithm>

using namespace std;
namespace fs = filesystem;

namespace minigit {

void handleBranch(int argc, char* argv[]) {
    if (argc < 3) {
        // List branches
        string branchDir = ".minigit/refs/heads";
        
        vector<string> branches;
        if (fs::exists(branchDir)) {
            for (const auto& entry : fs::directory_iterator(branchDir)) {
                if (entry.is_regular_file()) {
                    branches.push_back(entry.path().filename().string());
                }
            }
        }
        
        if (branches.empty()) {
            cerr << "No branches found." << endl;
            return;
        }

        string current_branch = getCurrentBranch();
        
        for (const auto& branch_name : branches) {
            if (branch_name == current_branch) {
                cout << "* " << branch_name << endl;
            } else {
                cout << "  " << branch_name << endl;
            }
        }
        return;
    }

    string branch_name = argv[2];
    
    // Check if branch already exists
    string branchPath = ".minigit/refs/heads/" + branch_name;
    if (fs::exists(branchPath)) {
        cerr << "Error: Branch '" << branch_name << "' already exists." << endl;
        return;
    }

    // Get current commit
    string current_commit = getHeadCommit();
    if (current_commit.empty()) {
        cerr << "Error: No commits yet. Cannot create branch." << endl;
        return;
    }

    // Create new branch pointing to current commit
    ofstream branch_file(branchPath);
    branch_file << current_commit << "\n";

    cout << "Created branch '" << branch_name << "' at " << current_commit.substr(0, 7) << endl;
}

void handleSwitch(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: miniGit switch <branch-name>" << endl;
        return;
    }

    string branch_name = argv[2];
    string branchPath = ".minigit/refs/heads/" + branch_name;

    if (!fs::exists(branchPath)) {
        cerr << "Error: Branch '" << branch_name << "' does not exist." << endl;
        return;
    }

    // Read the commit hash from the branch
    ifstream branch_file(branchPath);
    string branch_commit;
    getline(branch_file, branch_commit);

    if (branch_commit.empty()) {
        cerr << "Error: Branch '" << branch_name << "' is invalid." << endl;
        return;
    }

    // Read the commit to get the tree
    Commit commit = readCommit(branch_commit);
    if (commit.tree.empty()) {
        cerr << "Error: Invalid commit or tree." << endl;
        return;
    }

    // Read the tree
    map<string, string> files;
    readTreeToMap(commit.tree, files);

    // Clear the working directory (of tracked files)
    auto index = readIndex();
    for (const auto& [filepath, entry] : index) {
        if (fs::exists(filepath)) {
            cout << "Removing: " << filepath << endl;
            fs::remove(filepath);
        }
    }

    // Write the files from the tree
    for (const auto& [filepath, hash] : files) {
        string objectFile = ".minigit/objects/" + hash.substr(0, 2) + "/" + hash.substr(2);
        if (!fs::exists(objectFile)) {
            cerr << "Warning: Object not found for " << filepath << endl;
            continue;
        }

        ifstream obj_file(objectFile);
        stringstream buffer;
        buffer << obj_file.rdbuf();
        string raw_content = buffer.str();
        
        size_t null_pos = raw_content.find('\0');
        string content = raw_content.substr(null_pos + 1);

        // Create parent directories if needed
        fs::path file_path(filepath);
        if (file_path.has_parent_path()) {
            fs::create_directories(file_path.parent_path());
        }

        ofstream outFile(filepath, ios::binary | ios::trunc);
        outFile << content;
        outFile.close();

        cout << "Updated: " << filepath << endl;
    }

    // Update HEAD to point to the branch
    string headPath = ".minigit/HEAD";
    ofstream head_file(headPath);
    head_file << "ref: refs/heads/" << branch_name << "\n";

    // Update index to match the new tree
    map<string, IndexEntry> new_index;
    for (const auto& [filepath, hash] : files) {
        new_index[filepath] = {"100644", hash};
    }
    writeIndex(new_index);

    cout << "Switched to branch '" << branch_name << "'" << endl;
}

void handleStash(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: miniGit stash <save|pop|list>" << endl;
        return;
    }

    string subcommand = argv[2];
    fs::path stashDir = ".minigit/stash";

    if (subcommand == "save") {
        // Create stash directory if it doesn't exist
        if (!fs::exists(stashDir)) {
            fs::create_directory(stashDir);
        }

        // Get current index
        auto index = readIndex();
        if (index.empty()) {
            cout << "No changes to stash" << endl;
            return;
        }

        // Create a stash entry
        auto now = chrono::system_clock::now();
        auto timestamp = chrono::duration_cast<chrono::seconds>(now.time_since_epoch()).count();
        string stash_id = to_string(timestamp);
        
        fs::path stashFile = stashDir / stash_id;
        
        // Save current index to stash
        ofstream stash(stashFile);
        for (const auto& [filepath, entry] : index) {
            stash << entry.mode << " " << entry.hash << " " << filepath << endl;
        }
        stash.close();

        // Clear the index
        writeIndex({});
        
        // Remove files from working directory
        for (const auto& [filepath, entry] : index) {
            if (fs::exists(filepath)) {
                fs::remove(filepath);
            }
        }

        cout << "Saved working directory and index state" << endl;
        cout << "Stash ID: " << stash_id << endl;

    } else if (subcommand == "pop") {
        if (!fs::exists(stashDir)) {
            cout << "No stash entries found" << endl;
            return;
        }

        // Find the most recent stash
        string latest_stash;
        for (const auto& entry : fs::directory_iterator(stashDir)) {
            if (entry.is_regular_file()) {
                string filename = entry.path().filename().string();
                if (latest_stash.empty() || filename > latest_stash) {
                    latest_stash = filename;
                }
            }
        }

        if (latest_stash.empty()) {
            cout << "No stash entries found" << endl;
            return;
        }

        fs::path stashFile = stashDir / latest_stash;
        
        // Read stash
        map<string, IndexEntry> stashed_index;
        ifstream stash(stashFile);
        string line;
        while (getline(stash, line)) {
            stringstream ss(line);
            string mode, hash, filepath;
            ss >> mode >> hash >> filepath;
            stashed_index[filepath] = {mode, hash};
        }
        stash.close();

        // Restore files to working directory
        for (const auto& [filepath, entry] : stashed_index) {
            string content = readBlobContent(entry.hash);
            
            // Create parent directories if needed
            fs::path file_path(filepath);
            if (file_path.has_parent_path()) {
                fs::create_directories(file_path.parent_path());
            }
            
            ofstream outFile(filepath);
            outFile << content;
            outFile.close();
        }

        // Restore index
        writeIndex(stashed_index);

        // Remove stash file
        fs::remove(stashFile);

        cout << "Restored stash: " << latest_stash << endl;
        cout << "Dropped stash" << endl;

    } else if (subcommand == "list") {
        if (!fs::exists(stashDir)) {
            cout << "No stash entries found" << endl;
            return;
        }

        vector<string> stashes;
        for (const auto& entry : fs::directory_iterator(stashDir)) {
            if (entry.is_regular_file()) {
                stashes.push_back(entry.path().filename().string());
            }
        }

        if (stashes.empty()) {
            cout << "No stash entries found" << endl;
        } else {
            sort(stashes.rbegin(), stashes.rend()); // Newest first
            cout << "Stash entries:" << endl;
            int index = 0;
            for (const auto& stash : stashes) {
                cout << "stash@{" << index++ << "}: " << stash << endl;
            }
        }
    } else {
        cerr << "Unknown stash subcommand: " << subcommand << endl;
        cerr << "Usage: miniGit stash <save|pop|list>" << endl;
    }
}


void handleMerge(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: miniGit merge <branch-name>" << endl;
        return;
    }

    string branch_to_merge = argv[2];
    string current_branch = getCurrentBranch();

    if (current_branch.empty()) {
        cerr << "Error: Cannot merge while in detached HEAD state" << endl;
        return;
    }

    if (branch_to_merge == current_branch) {
        cerr << "Error: Cannot merge branch into itself" << endl;
        return;
    }

    // Get current commit
    string current_commit = getHeadCommit();
    if (current_commit.empty()) {
        cerr << "Error: No commits on current branch" << endl;
        return;
    }

    // Get branch to merge commit
    fs::path mergeBranchPath = fs::path(".minigit") / "refs" / "heads" / branch_to_merge;
    if (!fs::exists(mergeBranchPath)) {
        cerr << "Error: Branch '" << branch_to_merge << "' does not exist" << endl;
        return;
    }

    ifstream mergeBranchFile(mergeBranchPath);
    string merge_commit;
    getline(mergeBranchFile, merge_commit);
    mergeBranchFile.close();

    // Read both commits
    Commit current = readCommit(current_commit);
    Commit merge = readCommit(merge_commit);

    if (current.tree.empty() || merge.tree.empty()) {
        cerr << "Error: Invalid commits" << endl;
        return;
    }

    // Get file trees
    map<string, string> current_files;
    map<string, string> merge_files;
    readTreeToMap(current.tree, current_files);
    readTreeToMap(merge.tree, merge_files);

    // Track conflicts
    vector<string> conflicts;
    map<string, IndexEntry> merged_index;

    // Get current index
    auto current_index = readIndex();

    // Merge files
    set<string> all_files;
    for (const auto& [path, _] : current_files) all_files.insert(path);
    for (const auto& [path, _] : merge_files) all_files.insert(path);

    for (const auto& filepath : all_files) {
        bool in_current = current_files.count(filepath) > 0;
        bool in_merge = merge_files.count(filepath) > 0;

        if (in_current && in_merge) {
            // File exists in both - check if same
            if (current_files[filepath] == merge_files[filepath]) {
                // Same content - no conflict
                merged_index[filepath] = {"100644", current_files[filepath]};
            } else {
                // Different content - CONFLICT!
                conflicts.push_back(filepath);
                
                // Create conflict markers
                string current_content = readBlobContent(current_files[filepath]);
                string merge_content = readBlobContent(merge_files[filepath]);
                
                stringstream conflict_content;
                conflict_content << "<<<<<<< " << current_branch << "\n";
                conflict_content << current_content;
                if (!current_content.empty() && current_content.back() != '\n') {
                    conflict_content << "\n";
                }
                conflict_content << "=======\n";
                conflict_content << merge_content;
                if (!merge_content.empty() && merge_content.back() != '\n') {
                    conflict_content << "\n";
                }
                conflict_content << ">>>>>>> " << branch_to_merge << "\n";

                // Write conflict file to working directory
                ofstream conflict_file(filepath);
                conflict_file << conflict_content.str();
                conflict_file.close();

                // Hash the conflict version
                string conflict_hash = hashObject(filepath);
                merged_index[filepath] = {"100644", conflict_hash};
            }
        } else if (in_current && !in_merge) {
            // File only in current - keep it
            merged_index[filepath] = {"100644", current_files[filepath]};
            
            // Restore file if not present
            if (!fs::exists(filepath)) {
                string content = readBlobContent(current_files[filepath]);
                ofstream outFile(filepath);
                outFile << content;
            }
        } else if (!in_current && in_merge) {
            // File only in merge - add it
            merged_index[filepath] = {"100644", merge_files[filepath]};
            
            // Write file to working directory
            string content = readBlobContent(merge_files[filepath]);
            
            // Create parent directories if needed
            fs::path file_path(filepath);
            if (file_path.has_parent_path()) {
                fs::create_directories(file_path.parent_path());
            }
            
            ofstream outFile(filepath);
            outFile << content;
        }
    }

    // Update index
    writeIndex(merged_index);

    if (conflicts.empty()) {
        // No conflicts - create merge commit automatically
        string tree_hash = writeTree();
        
        stringstream commit_content;
        commit_content << "tree " << tree_hash << "\n";
        commit_content << "parent " << current_commit << "\n";
        commit_content << "parent " << merge_commit << "\n";

        auto now = chrono::system_clock::now();
        auto timestamp = chrono::duration_cast<chrono::seconds>(now.time_since_epoch()).count();
        string author_line = "Your Name <you@example.com> " + to_string(timestamp) + " +0000";
        
        commit_content << "author " << author_line << "\n";
        commit_content << "committer " << author_line << "\n";
        commit_content << "\nMerge branch '" << branch_to_merge << "' into " << current_branch << "\n";

        string commit_str = commit_content.str();
        string commit_obj = "commit " + to_string(commit_str.length()) + '\0' + commit_str;
        string new_commit_hash = writeObject(commit_obj);

        // Update current branch
        fs::path currentBranchPath = fs::path(".minigit") / "refs" / "heads" / current_branch;
        ofstream branchFile(currentBranchPath, ios::trunc);
        branchFile << new_commit_hash << endl;
        branchFile.close();

        cout << "Merge successful!" << endl;
        cout << "[" << current_branch << " " << new_commit_hash.substr(0, 7) 
                  << "] Merge branch '" << branch_to_merge << "'" << endl;
    } else {
        // Conflicts exist
        cout << "Automatic merge failed; fix conflicts and then commit the result." << endl;
        cout << "\nConflicts in:" << endl;
        for (const auto& file : conflicts) {
            cout << "\t" << file << endl;
        }
        cout << "\nResolve conflicts, then run: miniGit commit -m \"Merge " 
                  << branch_to_merge << "\"" << endl;
    }
}

void handleCheckout(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: miniGit checkout <commit-hash|branch-name>" << endl;
        return;
    }

    string target = argv[2];
    string commit_hash;
    bool is_branch = false;
    
    // Check if target is a branch name
    fs::path branchPath = fs::path(".minigit") / "refs" / "heads" / target;
    if (fs::exists(branchPath)) {
        is_branch = true;
        ifstream branchFile(branchPath);
        getline(branchFile, commit_hash);
    } else {
        // Assume it's a commit hash
        commit_hash = target;
    }

    Commit commit = readCommit(commit_hash);

    if (commit.tree.empty()) {
        cerr << "Error: Invalid commit or tree." << endl;
        return;
    }

    // Read the tree
    map<string, string> files;
    readTreeToMap(commit.tree, files);

    // Clear the working directory (of tracked files)
    auto index = readIndex();
    for (const auto& [filepath, entry] : index) {
        if (fs::exists(filepath)) {
            cout << "Removing: " << filepath << endl;
            fs::remove(filepath);
        }
    }

    // Write the files from the tree
    for (const auto& [filepath, hash] : files) {
        fs::path objectFile = fs::path(".minigit") / "objects" / hash.substr(0, 2) / hash.substr(2);
        ifstream file(objectFile, ios::binary);
        stringstream buffer;
        buffer << file.rdbuf();
        string raw_content = buffer.str();

        size_t null_pos = raw_content.find('\0');
        string content = raw_content.substr(null_pos + 1);

        // Create parent directories if needed
        fs::path file_path(filepath);
        if (file_path.has_parent_path()) {
            fs::create_directories(file_path.parent_path());
        }

        ofstream outFile(filepath);
        outFile << content;
    }

    // Update index to match the tree
    map<string, IndexEntry> new_index;
    for (const auto& [filepath, hash] : files) {
        new_index[filepath] = {"100644", hash};
    }
    writeIndex(new_index);

    // Update HEAD
    fs::path headPath = ".minigit/HEAD";
    ofstream headFile(headPath, ios::trunc);
    if (is_branch) {
        headFile << "ref: refs/heads/" << target << endl;
        cout << "Switched to branch '" << target << "'" << endl;
    } else {
        headFile << commit_hash << endl;
        cout << "HEAD is now at " << commit_hash.substr(0, 7) << " (detached)" << endl;
    }
}

} // namespace minigit
