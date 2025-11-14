
# miniGit

**A Data Structures and Algorithms Course Project**

---

## ✨ Key Features

### Core Git Operations
- **Repository initialization** (`init`)
- **Object storage** (blobs, trees, commits)
- **Staging area** and `add` command
- **Committing changes** (`commit`)
- **Commit history** (`log`)
- **Checking out** commits and branches (`checkout`)
- **Working directory status** (`status`)
- **Branch management** (`branch`, `switch`)

### 🌳 Merkle Tree Features
- **Fast Change Detection:** O(1) working directory status using Merkle root comparison
- **Efficient Branch Comparison:** Instantly compare two branches using Merkle roots
- **Integrity Verification:** Detect corruption/tampering by recursively validating all objects

**Special Commands:**
```bash
miniGit verify-integrity           # Verify repository integrity
miniGit compare-branches main dev  # Compare branch contents
```

---

## 📚 Data Structures & Algorithms

- **std::map (Red-Black Tree):** Used for index and commit history (ordered, O(log n) lookup)
- **Merkle Tree:** Used for fast change detection, branch comparison, and integrity verification
- **Linked List:** Commit parent chains for history traversal
- **std::string:** For hash storage and manipulation
- **structs:** For index entries and commit objects (POD types)
- **Content-Addressable Storage:** 2-level directory sharding for object database

See [docs/internals.md](docs/internals.md) for format specs and rationale.

---

## 🏗️ Project Structure

```
miniGit/
├── include/         # Header files (public APIs)
├── src/             # Implementation files
│   ├── main.cpp         # Entry point
│   ├── repository.cpp   # Repo management + Merkle features
│   ├── objects.cpp      # Object database
│   ├── commands.cpp     # Command handlers
│   ├── branch.cpp       # Branch operations
│   ├── index.cpp        # Staging area
│   ├── merkle.cpp       # Merkle tree operations
│   └── utils.cpp        # Utilities
├── docs/            # Documentation
│   ├── internals.md      # Internal format/design
│   └── TEAM_CONTRIBUTIONS.txt # Team commit breakdown
├── tests/           # Test suite
│   └── run_tests.sh        # Comprehensive test script (19 tests)
└── build/           # Build artifacts
```

---

## ⚙️ Building

Requires: C++20 compiler, CMake 3.16+, OpenSSL (for SHA-1)

```bash
# Build instructions
mkdir -p build
cd build
cmake ..
make
```
Executable: `build/miniGit`

---

## 🧪 Testing

miniGit includes a comprehensive test suite for all Merkle tree and core features:

```bash
cd tests
./run_tests.sh
```

**Test Coverage (19 tests):**
- ✅ Basic repository operations
- ✅ Fast change detection
- ✅ Efficient branch comparison
- ✅ Integrity verification
- ✅ Corruption detection
- ✅ Empty repository handling
- ✅ Branch comparison edge cases
- ✅ Multiple file operations

---

## 🚀 Usage

```bash
./build/miniGit <command> [options]
```

### Common Commands

```bash
# Initialize a repository
miniGit init

# Add files to staging
miniGit add <file>

# Commit changes
miniGit commit -m "message"

# View history
miniGit log

# Create and switch branches
miniGit branch feature
miniGit switch feature

# Merkle tree operations
miniGit verify-integrity
miniGit compare-branches main feature
```

---

## 🌳 Merkle Tree Use Cases

1. **Quick Status Checks:** O(1) working directory status
2. **Branch Comparison:** Instantly see if branches have diverged
3. **Corruption Detection:** Verify repository integrity recursively

---

## 👥 Team & Documentation

- **Team Contributions:** See [docs/TEAM_CONTRIBUTIONS.txt](docs/TEAM_CONTRIBUTIONS.txt) for a full breakdown of sequential commits and member roles.
- **Internals:** [docs/internals.md](docs/internals.md) covers index format, object database, Merkle tree, and rationale.
- **SHA-1 Implementation:** See `include/sha1.hpp` for a fully documented cryptographic hash algorithm.

---

## 📖 License

MIT