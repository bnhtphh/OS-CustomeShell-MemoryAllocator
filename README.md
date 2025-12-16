# FogOS


## Custom Shell (`user/crash.c`)

A custom shell implementation for FogOS with command history, built-in commands, and script execution support.

### Features

- **Command History**: Maintains a history of up to 100 commands with support for:
  - Viewing full history with `history` command
  - Accessing commands by number
  - Prefix-based history search (via helper functions)
- **Built-in Commands**:
  - `exit` - Exits the shell immediately
  - `cd <directory>` - Changes the current working directory (with error handling)
  - `history` - Displays command history with numbered entries
- **Command Execution**: Executes external programs via `fork()` and `exec()`
  - Tracks exit status of executed commands
  - Waits for child processes to complete
- **Script Support**: Can read and execute commands from a file (passed as command-line argument)
  - Reads commands line-by-line from the file
  - Automatically closes file when done
- **Comment Handling**: 
  - Lines starting with `#` are treated as comments and ignored
  - Inline comments (text after `#` on a line) are also stripped
- **Custom Prompt**: Displays exit status and command number in the format `[exit_status]-[cmd_num]─[cwd]$`
  - Only shown in interactive mode (not when reading from file)
  - Shows the exit status of the previous command

### Usage

```bash
# Interactive mode
$ crash

# Execute commands from a file
$ crash script.txt
```

### Implementation Details

- Uses custom tokenization functions (`strspn`, `strcspn`, `next_token`) for parsing commands
- Maintains command history in a circular buffer (max 100 entries)
- Tracks exit status of executed commands and displays in prompt
- Supports both interactive input (via `gets()`) and file-based input (via `read()`)
- Handles empty lines and whitespace trimming
- Custom string utility functions for tokenization without standard library dependencies

## Memory Allocator (`user/umalloc.c`)

A custom memory allocator implementation with multiple allocation strategies, block management, and debugging features.

### Features

- **Multiple Allocation Algorithms**:
  - **First Fit** (FIRST_FIT=0): Allocates from the first free block that fits
  - **Best Fit** (BEST_FIT=1): Allocates from the smallest free block that fits
  - **Worst Fit** (WORST_FIT=2): Allocates from the largest free block
  - Algorithm can be switched at runtime via `malloc_setfsm()`
- **Memory Management**:
  - Block-based allocation with metadata headers (`mem_block` struct)
  - 16-byte alignment for all user allocations
  - Page-aligned (4096 bytes) allocations from OS via `sbrk()`
  - Automatic block splitting when allocating from larger free blocks
    - Splits only if remaining size >= minimum split size (requested size + block header + 16 bytes)
  - Automatic coalescing (merging) of adjacent free blocks on `free()`
  - Memory release back to OS when tail blocks are free and >= 4096 bytes
    - Automatically releases memory at the end of the heap
- **Standard Functions**:
  - `malloc(size)` - Allocate memory (returns NULL on failure or zero size)
  - `free(ptr)` - Free allocated memory (safe to call with NULL)
  - `calloc(nmemb, size)` - Allocate and zero-initialize memory
  - `realloc(ptr, size)` - Resize existing allocation
    - In-place expansion when next block is free and large enough
    - In-place shrinking when new size is smaller
    - Allocates new block and copies data if in-place resize not possible
- **Debug Features**:
  - `malloc_print()` - Print current memory state (all blocks) and free list
  - `malloc_name(ptr, name)` - Name allocations for debugging (max 7 chars, null-terminated)
  - `malloc_setfsm(algorithm)` - Switch allocation algorithm at runtime
  - Optional debug logging when compiled with `-DDEBUG=1`

### Implementation Details

- **Data Structures**:
  - Doubly-linked block list maintained in address order (for coalescing)
  - Separate free list (singly-linked) for efficient allocation searches
  - Free bit stored in the least significant bit of the size field (bit manipulation)
  - Block metadata includes: name (8 bytes), size (with free bit), next/prev pointers
  - Free list nodes stored immediately after block header
- **Allocation Strategy**:
  1. Try to reuse a free block using selected algorithm (First/Best/Worst Fit)
  2. If no suitable free block, request new memory from OS via `sbrk()`
  3. Split blocks when allocating from larger free blocks
  4. Return pointer to user data (after block header)
- **Deallocation Strategy**:
  1. Mark block as free
  2. Add to free list
  3. Merge with adjacent free blocks (both next and previous)
  4. Release memory to OS if tail block is free and >= 4096 bytes
- **Reallocation Strategy**:
  1. Shrink in-place if new size is smaller (split off excess)
  2. Expand in-place if next block is free and combined size is sufficient
  3. Otherwise, allocate new block, copy data, and free old block

### Usage

```c
// Set allocation algorithm
malloc_setfsm(FIRST_FIT);  // or BEST_FIT, WORST_FIT

// Allocate memory
void *ptr = malloc(100);
malloc_name(ptr, "buffer");

// Resize allocation
ptr = realloc(ptr, 200);

// Debug: print memory state
malloc_print();

// Free memory
free(ptr);
```
