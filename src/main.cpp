// src/main.cpp
// Main entry point and command dispatcher

#include "commands.hpp"
#include "branch.hpp"
#include "objects.hpp"
#include <iostream>
#include <string>

using namespace std;
using namespace minigit;

int main(int argc, char* argv[]) {
    
    if (argc < 2) {
        cerr << "Usage: miniGit <command> [args]" << endl;
        cerr << "Commands:" << endl;
        cerr << "  init              - Initialize a new repository" << endl;
        cerr << "  commit -m <msg>   - Create a new commit" << endl;
        cerr << "  status            - Show working tree status" << endl;
        cerr << "  add <file>        - Add file to staging area" << endl;
        cerr << "  log               - Show commit logs" << endl;
        cerr << "  branch [name]     - List or create branches" << endl;
        cerr << "  checkout <ref>    - Checkout a branch or commit" << endl;
        cerr << "  switch <branch>   - Switch to a branch" << endl;
        cerr << "  verify-tree <ref> - Verify tree with Merkle hash" << endl;
        cerr << "  verify-integrity  - Verify repository integrity" << endl;
        cerr << "  compare-branches <b1> <b2> - Compare branch contents" << endl;
        cerr << "  merge <branch>    - Merge a branch into current" << endl;
        cerr << "  diff-tree <r1> <r2> - Compare trees efficiently" << endl;
        cerr << "  hash-object <file> - Compute object ID" << endl;
        cerr << "  cat-file <hash>   - Show object content" << endl;
        cerr << "  stash <cmd>       - Stash changes (save|pop|list)" << endl;
        return 1;
    }

    string command = argv[1];
    if (command == "init") {
        handleInit();
    } else if (command == "hash-object") {
        handleHashObject(argc, argv);
    } else if (command == "cat-file") {
        handleCatFile(argc, argv);
    } else if (command == "add") {
        handleAdd(argc, argv);
    } else if (command == "write-tree") {
        // Debug command
        cout << writeTree() << endl;
    } else if (command == "commit") {
        handleCommit(argc, argv);
    } else if (command == "log") {
        handleLog();
    } else if (command == "checkout") {
        handleCheckout(argc, argv);
    } else if (command == "branch") {
        handleBranch(argc, argv);
    } else if (command == "merge") {
        handleMerge(argc, argv);
    } else if (command == "stash") {
        handleStash(argc, argv);
    } else if (command == "switch") {
        handleSwitch(argc, argv);
    } else if (command == "status") {
        handleStatus();
    } else if (command == "diff-tree") {
        handleDiffTree(argc, argv);
    } else if (command == "verify-integrity") {
        handleVerifyIntegrity();
    } else if (command == "verify-tree") {
        handleVerifyTree(argc, argv);
    } else if (command == "compare-branches") {
        handleCompareBranches(argc, argv);
    } else {
        cerr << "Unknown command: " << command << endl;
        cerr << "Run 'miniGit' without arguments to see available commands." << endl;
        return 1;
    }


    return 0;
}
