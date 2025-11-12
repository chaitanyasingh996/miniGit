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
        cerr << "  add <file>        - Add file to staging area" << endl;
        cerr << "  commit -m <msg>   - Create a new commit" << endl;
        cerr << "  status            - Show working tree status" << endl;
        cerr << "  log               - Show commit logs" << endl;
        cerr << "  branch [name]     - List or create branches" << endl;
        cerr << "  switch <branch>   - Switch to a branch" << endl;
        cerr << "  checkout <ref>    - Checkout a branch or commit" << endl;
        cerr << "  merge <branch>    - Merge a branch into current" << endl;
        cerr << "  stash <cmd>       - Stash changes (save|pop|list)" << endl;
        cerr << "  verify-tree <ref> - Verify tree with Merkle hash" << endl;
        cerr << "  diff-tree <r1> <r2> - Compare trees efficiently" << endl;
        cerr << "  verify-integrity  - Verify repository integrity" << endl;
        cerr << "  compare-branches <b1> <b2> - Compare branch contents" << endl;
        cerr << "  hash-object <file> - Compute object ID" << endl;
        cerr << "  cat-file <hash>   - Show object content" << endl;
        return 1;
    }

    string command = argv[1];

    if (command == "init") {
        minigit::handleInit();
    } else if (command == "hash-object") {
        minigit::handleHashObject(argc, argv);
    } else if (command == "cat-file") {
        minigit::handleCatFile(argc, argv);
    } else if (command == "add") {
        minigit::handleAdd(argc, argv);
    } else if (command == "write-tree") {
        // Debug command
        cout << minigit::writeTree() << endl;
    } else if (command == "commit") {
        minigit::handleCommit(argc, argv);
    } else if (command == "log") {
        minigit::handleLog();
    } else if (command == "checkout") {
        minigit::handleCheckout(argc, argv);
    } else if (command == "switch") {
        minigit::handleSwitch(argc, argv);
    } else if (command == "branch") {
        minigit::handleBranch(argc, argv);
    } else if (command == "merge") {
        minigit::handleMerge(argc, argv);
    } else if (command == "stash") {
        minigit::handleStash(argc, argv);
    } else if (command == "status") {
        minigit::handleStatus();
    } else if (command == "verify-tree") {
        minigit::handleVerifyTree(argc, argv);
    } else if (command == "diff-tree") {
        minigit::handleDiffTree(argc, argv);
    } else if (command == "verify-integrity") {
        minigit::handleVerifyIntegrity();
    } else if (command == "compare-branches") {
        minigit::handleCompareBranches(argc, argv);
    } else {
        cerr << "Unknown command: " << command << endl;
        cerr << "Run 'miniGit' without arguments to see available commands." << endl;
        return 1;
    }

    return 0;
}
