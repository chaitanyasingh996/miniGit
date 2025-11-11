# miniGit Internals Documentation

## Index File Format Specification

### Overview
The miniGit index file (`.minigit/index`) stores the staging area with file metadata in a structured text format. Each line represents one tracked file with its mode, hash, and path.

### Format Specification

#### Line Format
```
MODE HASH PATH
```

#### Components

| Component | Type | Format | Example | Description |
|-----------|------|--------|---------|-------------|
| **MODE** | String | Octal permission | `100644` | File permissions (currently always `100644` for regular files) |
| **HASH** | String | SHA-1 hex | `a1b2c3d4...` | 40-character SHA-1 hash of file blob object |
| **PATH** | String | Relative path | `src/main.cpp` | File path relative to repository root |

#### Example Index File
```
100644 d8e8fca2dc0f896fd7cb4cb0031ba249 README.md
100644 a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6 src/main.cpp
100644 f1e2d3c4b5a69788796859585857565a src/utils.cpp
100644 7a8b9c0d1e2f3a4b5c6d7e8f9a0b1c2d test/test_main.cpp
```

### Design Rationale

#### Why Text-Based Format?
- **Human-readable**: Easy to debug and understand repository state
- **Portable**: Works across platforms without binary compatibility issues
- **Simple parsing**: Line-by-line tokenization with space delimiter
- **Version control friendly**: Can track changes in Git itself if needed
- **Diff-friendly**: Clear what changed between index versions

#### Why This Line Order?
- **Sorted alphabetically**: Deterministic output for reproducible builds
- **Enables binary search**: Future optimization for large repositories
- **Matches Git behavior**: Familiar to users transitioning from Git

#### Index Entry Structure
```cpp
struct IndexEntry {
    string mode;  // File permissions (e.g., "100644")
    string hash;  // SHA-1 blob hash (40 hex characters)
    // path is the map key in std::map<string, IndexEntry>
};
```

### Index File Location
```
Repository Root (.minigit/):
.minigit/
  ├── index                    ← Index file (staging area)
  ├── HEAD                     ← Current branch reference
  ├── objects/                 ← Object database
  │   ├── ab/                  ← Hash prefix directory
  │   │   └── cdef1234...      ← Object file
  │   └── ...
  └── refs/
      ├── heads/              ← Branch references
      │   └── main
      └── tags/               ← Tag references
```

### Operations on Index

#### Reading Index
- Parse `.minigit/index` line by line
- For each line: `mode hash path`
- Store in `std::map<string, IndexEntry>` for sorted ordering

#### Writing Index
- Iterate over `std::map<string, IndexEntry>` (naturally sorted)
- Write each entry as: `mode hash path\n`
- Flush to disk at `.minigit/index`

#### Adding Files
1. Calculate blob hash: `hashObject(filepath)`
2. Add entry: `index[filepath] = {mode, blob_hash}`
3. Mark index as dirty (needs write)

#### Committing
1. Read index into map
2. Create tree object from index entries
3. Create commit object with tree hash
4. Clear index after successful commit

---

## Next Sections (To Be Completed)

### Merkle Tree Operations
- Tree object format and parsing
- Hash calculation and verification
- Branch comparison using Merkle roots

### Commit Object Structure
- Commit metadata (author, timestamp, message)
- Parent chain for history traversal
- Tree reference for snapshot

### Object Database Layout
- Hash-based sharding (2-char prefix)
- Object file naming scheme
- Blob, tree, and commit object formats

---

**Status**: 30% complete - Index format specification documented

## Performance Notes (50% complete)

To reduce the runtime cost of index operations, miniGit uses a small in-memory cache for the index:

- Reads: `readIndex()` populates an in-memory cache on first access and returns the cached map on subsequent calls. This avoids repeatedly parsing `.minigit/index` when multiple operations (status, add, commit) query the index.
- Writes: `writeIndex()` writes the provided `std::map<std::string, IndexEntry>` to `.minigit/index` and updates the cache. This ensures the on-disk index and cache stay consistent.
- Adds: `addToIndex()` operates on the cached index and calls `writeIndex()` to persist changes. Future work could add a lazy-flush mode to batch multiple additions into a single disk write for large operations.

Why this helps:
- Reduces file I/O when many operations read the index repeatedly (status, tree generation, verification).
- Keeps behavior deterministic while improving performance on common sequences of commands.

Next steps to further optimize:
- Implement a configurable lazy-flush mechanism to batch writes.
- Use file locking when working in concurrent environments.
- Consider a binary index format for faster parsing if performance becomes critical.

