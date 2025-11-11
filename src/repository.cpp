// src/repository.cpp
// Repository initialization and management

#include "repository.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>

using namespace std;
namespace fs = filesystem;

namespace minigit
{

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

    string getRefHash(const string& ref) {
        if (ref.empty()) return "";
        string refPath = ".minigit/" + ref;
        
        if (!fs::exists(refPath)) {
            return "";
        }

        ifstream file(refPath);
        string hash;
        getline(file, hash);

        return hash;
    }

} // namespace minigit
