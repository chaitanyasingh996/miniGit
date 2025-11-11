# miniGit

A minimal implementation of Git in C++20, built for educational purposes.

This project is a learning tool designed to uncover the core concepts behind Git's internal workings. It follows the excellent tutorial "[Write Yourself a Git!](https://wyag.thb.lt/)" but is implemented in modern C++ with a **clean, modular architecture**.

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

### 🌳 Merkle Tree Features (NEW!)

miniGit leverages **Merkle trees** for efficient operations:

1. **Fast Change Detection** 
   - Instantly detect if working directory has changes
   - O(1) comparison instead of checking every file
   - Uses cryptographic hashing for integrity

2. **Efficient Branch Comparison**
   - Compare two branches in constant time
   - Identify differences without scanning all files
   - Perfect for large repositories

3. **Integrity Verification**
   - Verify entire repository hasn't been corrupted
   - Detect tampering or bit rot
   - Validates all commits and objects recursively

**New Commands:**
```bash
miniGit verify-integrity           # Verify repository integrity
miniGit compare-branches main dev  # Compare branch contents
```

## Project Structure

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
│   └── DATA_STRUCTURES.md  # Data structure reference
├── tests/           # Test suite
│   └── run_tests.sh        # Comprehensive test script (19 tests)
└── build/           # Build artifacts
```

## Building

To build miniGit, you need a C++20 compatible compiler and CMake.

```bash
# Using CMake
mkdir -p build
cd build
cmake ..
make
```

The executable will be at `build/miniGit`.

## Testing

miniGit includes a comprehensive test suite for all Merkle tree features:

```bash
# Run all tests
cd tests
./run_tests.sh
```

**Test Coverage (19 tests):**
- ✅ Basic repository operations (4 tests)
- ✅ Fast change detection (3 tests)
- ✅ Efficient branch comparison (4 tests)
- ✅ Integrity verification (2 tests)
- ✅ Corruption detection (2 tests)
- ✅ Empty repository handling (1 test)
- ✅ Branch comparison edge cases (1 test)
- ✅ Multiple file operations (2 tests)

The test suite is a single shell script (`tests/run_tests.sh`) that comprehensively validates all Merkle tree functionality.

## Usage

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

## Merkle Tree Use Cases

### 1. Quick Status Checks
Instead of scanning all files, compare a single hash:
```bash
# Internally uses Merkle tree root comparison
miniGit status
```

### 2. Branch Comparison
Instantly see if branches have diverged:
```bash
miniGit compare-branches main develop
# Output shows:
#   ✓ IDENTICAL - branches have same content
#   ✗ DIFFERENT - shows which files differ
```

### 3. Corruption Detection
Verify repository hasn't been tampered with:
```bash
miniGit verify-integrity
# Verifies:
#   - All commit hashes valid
#   - All tree hashes valid  
#   - All blob hashes valid
#   - Chain of custody intact
```

## Documentation

- **[docs/DATA_STRUCTURES.md](docs/DATA_STRUCTURES.md)** - Detailed explanation of data structures including Merkle trees