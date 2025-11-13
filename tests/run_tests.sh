#!/bin/bash

# miniGit Merkle Tree Features Test Suite
# Tests all three implemented Merkle tree features

# MINIGIT="/home/siddhant/Documents/miniGit/build/miniGit"
MINIGIT="../../build/miniGit.exe "
TEST_DIR="test_repo"

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Test counter
PASSED=0
FAILED=0

# Setup test repository
setup_test() {
    cd "$(dirname "$0")"
    rm -rf "$TEST_DIR"
    mkdir -p "$TEST_DIR"
    cd "$TEST_DIR"
    $MINIGIT init > /dev/null 2>&1
}

# Cleanup test repository
cleanup_test() {
    cd ..
    rm -rf "$TEST_DIR"
}

# Print test header
print_header() {
    echo ""
    echo "=========================================="
    echo "$1"
    echo "=========================================="
}

# Print test result
print_result() {
    if [ $1 -eq 0 ]; then
        echo -e "${GREEN}✓ $2${NC}"
        ((PASSED++))
    else
        echo -e "${RED}✗ $2${NC}"
        ((FAILED++))
    fi
}

# Test 1: Basic Repository Operations
test_basic_operations() {
    print_header "Test 1: Basic Repository Operations"
    
    setup_test
    
    echo "test content" > file.txt
    $MINIGIT add file.txt > /dev/null 2>&1
    print_result $? "Add file to staging"
    
    $MINIGIT commit -m "Initial commit" > /dev/null 2>&1
    print_result $? "Create commit"
    
    [ -d ".minigit" ]
    print_result $? "Repository directory exists"
    
    [ -d ".minigit/objects" ]
    print_result $? "Objects directory exists"
    
    cleanup_test
}

# Test 2: Fast Change Detection
test_change_detection() {
    print_header "Test 2: Fast Change Detection"
    
    setup_test
    
    # Create initial commit
    echo "content 1" > file1.txt
    $MINIGIT add file1.txt > /dev/null 2>&1
    $MINIGIT commit -m "First commit" > /dev/null 2>&1
    print_result $? "Create initial commit"
    
    # Make a change
    echo "content 2" > file2.txt
    $MINIGIT add file2.txt > /dev/null 2>&1
    print_result $? "Add new file to staging"
    
    # Commit the change
    $MINIGIT commit -m "Second commit" > /dev/null 2>&1
    print_result $? "Create second commit"
    
    cleanup_test
}

# Test 3: Efficient Branch Comparison
test_branch_comparison() {
    print_header "Test 3: Efficient Branch Comparison"
    
    setup_test
    
    # Create initial commit
    echo "main content" > main.txt
    $MINIGIT add main.txt > /dev/null 2>&1
    $MINIGIT commit -m "Main commit" > /dev/null 2>&1
    
    # Create feature branch (same content)
    $MINIGIT branch feature > /dev/null 2>&1
    print_result $? "Create feature branch"
    
    # Compare branches - should be identical
    OUTPUT=$($MINIGIT compare-branches main feature 2>&1)
    echo "$OUTPUT" | grep -q "IDENTICAL"
    print_result $? "Detect identical branches"
    
    # Switch to feature and make changes
    $MINIGIT switch feature > /dev/null 2>&1
    echo "feature content" > feature.txt
    $MINIGIT add feature.txt > /dev/null 2>&1
    $MINIGIT commit -m "Feature commit" > /dev/null 2>&1
    
    # Compare branches - should be different
    OUTPUT=$($MINIGIT compare-branches main feature 2>&1)
    echo "$OUTPUT" | grep -q "DIFFERENT"
    print_result $? "Detect different branches"
    
    echo "$OUTPUT" | grep -q "feature.txt"
    print_result $? "Show file differences"
    
    cleanup_test
}

# Test 4: Integrity Verification
test_integrity_verification() {
    print_header "Test 4: Integrity Verification"
    
    setup_test
    
    # Create multiple commits
    echo "file 1" > file1.txt
    $MINIGIT add file1.txt > /dev/null 2>&1
    $MINIGIT commit -m "Commit 1" > /dev/null 2>&1
    
    echo "file 2" > file2.txt
    $MINIGIT add file2.txt > /dev/null 2>&1
    $MINIGIT commit -m "Commit 2" > /dev/null 2>&1
    
    echo "file 3" > file3.txt
    $MINIGIT add file3.txt > /dev/null 2>&1
    $MINIGIT commit -m "Commit 3" > /dev/null 2>&1
    
    # Verify integrity
    OUTPUT=$($MINIGIT verify-integrity 2>&1)
    echo "$OUTPUT" | grep -q "✓ Repository integrity verified!"
    print_result $? "Verify repository integrity"
    
    echo "$OUTPUT" | grep -q "Commits verified: 3"
    print_result $? "Verify all 3 commits checked"
    
    cleanup_test
}

# Test 5: Corruption Detection
test_corruption_detection() {
    print_header "Test 5: Corruption Detection"
    
    setup_test
    
    # Create commit
    echo "original content" > file.txt
    $MINIGIT add file.txt > /dev/null 2>&1
    $MINIGIT commit -m "Commit" > /dev/null 2>&1
    
    # Verify it's valid first
    $MINIGIT verify-integrity > /dev/null 2>&1
    print_result $? "Repository valid before corruption"
    
    # Corrupt an object file
    OBJECT_FILE=$(find .minigit/objects -type f | head -1)
    if [ -n "$OBJECT_FILE" ]; then
        echo "CORRUPTED" > "$OBJECT_FILE"
        
        # Verify should fail now
        $MINIGIT verify-integrity > /dev/null 2>&1
        if [ $? -ne 0 ]; then
            print_result 0 "Corruption detected correctly"
        else
            print_result 1 "Corruption NOT detected (ERROR)"
        fi
    else
        print_result 1 "Could not find object to corrupt"
    fi
    
    cleanup_test
}

# Test 6: Empty Repository
test_empty_repository() {
    print_header "Test 6: Empty Repository Handling"
    
    setup_test
    
    # Verify empty repository
    OUTPUT=$($MINIGIT verify-integrity 2>&1)
    echo "$OUTPUT" | grep -q "No commits to verify"
    print_result $? "Handle empty repository gracefully"
    
    cleanup_test
}

# Test 7: Branch Comparison Edge Cases
test_branch_edge_cases() {
    print_header "Test 7: Branch Comparison Edge Cases"
    
    setup_test
    
    # Create commit
    echo "content" > file.txt
    $MINIGIT add file.txt > /dev/null 2>&1
    $MINIGIT commit -m "Commit" > /dev/null 2>&1
    
    # Compare non-existent branches (should print error but might not return error code)
    OUTPUT=$($MINIGIT compare-branches main nonexistent 2>&1)
    echo "$OUTPUT" | grep -q "not found"
    print_result $? "Handle non-existent branch gracefully"
    
    cleanup_test
}

# Test 8: Multiple File Changes
test_multiple_files() {
    print_header "Test 8: Multiple File Operations"
    
    setup_test
    
    # Create multiple files
    echo "file 1" > file1.txt
    echo "file 2" > file2.txt
    echo "file 3" > file3.txt
    $MINIGIT add file1.txt file2.txt file3.txt > /dev/null 2>&1
    $MINIGIT commit -m "Add 3 files" > /dev/null 2>&1
    print_result $? "Add multiple files in single commit"
    
    # Create branch
    $MINIGIT branch modify > /dev/null 2>&1
    $MINIGIT switch modify > /dev/null 2>&1
    
    # Modify only one file
    echo "modified" > file2.txt
    $MINIGIT add file2.txt > /dev/null 2>&1
    $MINIGIT commit -m "Modify one file" > /dev/null 2>&1
    
    # Compare should show difference
    OUTPUT=$($MINIGIT compare-branches main modify 2>&1)
    echo "$OUTPUT" | grep -q "DIFFERENT"
    print_result $? "Detect single file change among many"
    
    cleanup_test
}

# Run all tests
main() {
    echo ""
    echo "╔════════════════════════════════════════╗"
    echo "║  miniGit Merkle Tree Test Suite       ║"
    echo "╔════════════════════════════════════════╝"
    echo ""
    
    test_basic_operations
    test_change_detection
    test_branch_comparison
    test_integrity_verification
    test_corruption_detection
    test_empty_repository
    test_branch_edge_cases
    test_multiple_files
    
    # Print summary
    echo ""
    echo "=========================================="
    echo "Test Summary"
    echo "=========================================="
    echo -e "${GREEN}Passed: $PASSED${NC}"
    echo -e "${RED}Failed: $FAILED${NC}"
    echo "=========================================="
    
    if [ $FAILED -eq 0 ]; then
        echo -e "${GREEN}All tests passed! ✓${NC}"
        exit 0
    else
        echo -e "${RED}Some tests failed! ✗${NC}"
        exit 1
    fi
}

main
