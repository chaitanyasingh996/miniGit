// src/objects.cpp
// Object database management (blobs, trees, commits)

#include "objects.hpp"
#include "index.hpp"
#include "sha1.hpp"
#include <sstream>
#include <iostream>
#include <fstream>
#include <chrono>
#include <filesystem>

using namespace std;
namespace fs = filesystem;

namespace minigit {

string calculateHash(const string& content) {
    SHA1 sha1;
    sha1.update(content);
    return sha1.final();
}

string writeObject(const string& content) {
    string hash = calculateHash(content);

    string objectDir = ".minigit/objects/" + hash.substr(0, 2);
    string objectFile = objectDir + "/" + hash.substr(2);

    try {
        fs::create_directories(objectDir);
        ofstream file(objectFile);
        file << content;
    } catch (const exception& e) {
        cerr << "Error writing object: " << e.what() << endl;
        return "";
    }

    return hash;
}

string readObject(const string& hash) {
    string objectFile = ".minigit/objects/" + hash.substr(0, 2) + "/" + hash.substr(2);
    
    if (!fs::exists(objectFile)) {
        return "";
    }

    ifstream file(objectFile);
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

string hashObject(const string& filepath) {
    ifstream file(filepath, ios::binary);
    stringstream buffer;
    buffer << file.rdbuf();
    string content = buffer.str();

    string blob = "blob " + to_string(content.length()) + '\0' + content;
    return writeObject(blob);
}

void catFile(const string& hash) {
    string raw_content = readObject(hash);
    
    if (raw_content.empty()) {
        cerr << "Error: Object not found: " << hash << endl;
        return;
    }

    // Find the null byte separator
    size_t null_pos = raw_content.find('\0');
    if (null_pos == string::npos) {
        cerr << "Error: Invalid object format." << endl;
        return;
    }

    // Print the content after the null byte
    cout << raw_content.substr(null_pos + 1);
}

string writeTree() {
    auto index = readIndex();

    stringstream tree_content;
    for (const auto& [filepath, entry] : index) {
        tree_content << entry.mode << " " << "blob" << " " << entry.hash << " " << filepath << "\n";
    }

    string tree_str = tree_content.str();

    // Create the tree object with header
    string tree_obj = "tree " + to_string(tree_str.length()) + '\0' + tree_str;

    return writeObject(tree_obj);
}

string writeCommit(const string& tree_hash, const string& parent_hash, const string& message) {
    stringstream commit_content;
    commit_content << "tree " << tree_hash << "\n";
    
    if (!parent_hash.empty()) {
        commit_content << "parent " << parent_hash << "\n";
    }

    auto now = chrono::system_clock::now();
    auto timestamp = chrono::duration_cast<chrono::seconds>(now.time_since_epoch()).count();

    string author_line = "Your Name <you@example.com> " + to_string(timestamp) + " +0000";
    commit_content << "author " << author_line << "\n";
    commit_content << "committer " << author_line << "\n";
    commit_content << "\n" << message << "\n";

    string commit_str = commit_content.str();
    string commit_obj = "commit " + to_string(commit_str.length()) + '\0' + commit_str;

    return writeObject(commit_obj);
}

Commit readCommit(const string& hash) {
    string raw_content = readObject(hash);
    
    if (raw_content.empty()) {
        return {};
    }

    size_t null_pos = raw_content.find('\0');
    if (null_pos == string::npos) {
        return {};
    }

    string content = raw_content.substr(null_pos + 1);
    stringstream content_stream(content);
    string line;

    Commit commit;
    while (getline(content_stream, line) && !line.empty()) {
        stringstream line_stream(line);
        string key;
        line_stream >> key;

        if (key == "tree") {
            line_stream >> commit.tree;
        } else if (key == "parent") {
            line_stream >> commit.parent;
        } else if (key == "author") {
            getline(line_stream, commit.author);
            // Trim leading space
            if (!commit.author.empty() && commit.author[0] == ' ') {
                commit.author = commit.author.substr(1);
            }
        } else if (key == "committer") {
            getline(line_stream, commit.committer);
            // Trim leading space
            if (!commit.committer.empty() && commit.committer[0] == ' ') {
                commit.committer = commit.committer.substr(1);
            }
        }
    }

    getline(content_stream, commit.message, (char) EOF);
    commit.message.erase(0, commit.message.find_first_not_of("\n"));

    return commit;
}

vector<pair<string, string>> readTree(const string& tree_hash) {
    vector<pair<string, string>> files;
    string raw_content = readObject(tree_hash);
    
    if (raw_content.empty()) {
        return files;
    }

    size_t null_pos = raw_content.find('\0');
    if (null_pos == string::npos) {
        return files;
    }

    string content = raw_content.substr(null_pos + 1);
    stringstream content_stream(content);
    string line;

    while (getline(content_stream, line)) {
        stringstream line_stream(line);
        string mode, type, file_hash, filepath;
        line_stream >> mode >> type >> file_hash >> filepath;
        files.push_back({filepath, file_hash});
    }

    return files;
}

string readBlobContent(const string& hash) {
    string raw_content = readObject(hash);
    
    if (raw_content.empty()) {
        return "";
    }

    size_t null_pos = raw_content.find('\0');
    if (null_pos == string::npos) {
        return "";
    }

    return raw_content.substr(null_pos + 1);
}

} // namespace minigit
