#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

#define NULL 0

/* If we haven't passed -DDEBUG=1 to gcc, then this will be set to 0: */
#ifndef DEBUG
#define DEBUG 0
#endif

#if DEBUG
#define LOGP(str) \
    do { fprintf(2, "%s:%d:%s(): %s", __FILE__, \
            __LINE__, __func__, str); } while (0)

#define LOG(fmt, ...) \
    do { fprintf(2, "%s:%d:%s(): " fmt, __FILE__, \
            __LINE__, __func__, __VA_ARGS__); } while (0)
#else
#define LOGP(str) do {} while (0)
#define LOG(fmt, ...) do {} while (0)
#endif

// ============================================================================
// Structs

struct mem_block *head = NULL;
struct mem_block *tail = NULL;
struct mem_block *free_head = NULL;

struct __attribute__((__packed__)) mem_block {
  char name[8];
  uint64 size;
  struct mem_block *next_block;
  struct mem_block *prev_block;
};


struct free_list_node {
  struct mem_block *next_free;
  struct mem_block *prev_free;
};

int current_fsm = FIRST_FIT;

/* FSM algorithm selection */
#define FIRST_FIT 0
#define BEST_FIT 1
#define WORST_FIT 2

// ============================================================================
// Size and Free Bit 

void
set_used(struct mem_block *block)
{
  // turn off the least significant bit
  block->size = block->size & (~0x01);
}

void
set_free(struct mem_block *block)
{
  // turn on the least significant bit
  block->size = block->size | 0x01;
}

int
is_free(struct mem_block *block)
{
  // check whether the last bit is set or not
  return block->size & 0x01;
}

uint
get_size(struct mem_block *block)
{
  // ignore the free bit when retrieving the size
  return (uint)(block->size & (~0x01));
}

void
set_size(struct mem_block *block, uint size)
{
  int was_free = is_free(block);
  block->size = size;
  if (was_free) {
    set_free(block);
  }
}

// ============================================================================
// Alignment 

uint
align_size(uint size)
{
  if (size % 16 == 0) {
    return size;
  }
  return size + (16 - (size % 16));
}

uint
align_to_page(uint size)
{
  if (size % 4096 == 0) {
    return size;
  }
  return size + (4096 - (size % 4096));
}

// ============================================================================
// Free List 

struct free_list_node *
get_free_node(struct mem_block *block)
{
  return (struct free_list_node *)((char *)block + sizeof(struct mem_block));
}

void
free_list_add(struct mem_block *block)
{
  struct free_list_node *node = get_free_node(block);
  node->next_free = NULL;
  node->prev_free = NULL;

  if (free_head == NULL) {
    free_head = block;
    return;
  }

  // Find tail of free list
  struct mem_block *current = free_head;
  while (get_free_node(current)->next_free != NULL) {
    current = get_free_node(current)->next_free;
  }

  // Add at end
  get_free_node(current)->next_free = block;
  node->prev_free = current;
}

void
free_list_remove(struct mem_block *block)
{
  struct free_list_node *node = get_free_node(block);

  if (node->prev_free != NULL) {
    get_free_node(node->prev_free)->next_free = node->next_free;
  } else {
    free_head = node->next_free;
  }

  if (node->next_free != NULL) {
    get_free_node(node->next_free)->prev_free = node->prev_free;
  }
}

// ============================================================================
// Block List 

void
block_list_add(struct mem_block *block)
{
  if (head == NULL) {
    head = block;
    tail = block;
    block->next_block = NULL;
    block->prev_block = NULL;
    return;
  }

  // Insert in address order
  struct mem_block *current = head;

  // Check if new head
  if (block < head) {
    block->next_block = head;
    block->prev_block = NULL;
    head->prev_block = block;
    head = block;
    return;
  }

  // Find insertion point
  while (current->next_block != NULL && current->next_block < block) {
    current = current->next_block;
  }

  // Insert after current
  block->next_block = current->next_block;
  block->prev_block = current;

  if (current->next_block != NULL) {
    current->next_block->prev_block = block;
  } else {
    tail = block;
  }

  current->next_block = block;
}

void
block_list_remove(struct mem_block *block)
{
  if (block->prev_block != NULL) {
    block->prev_block->next_block = block->next_block;
  } else {
    head = block->next_block;
  }

  if (block->next_block != NULL) {
    block->next_block->prev_block = block->prev_block;
  } else {
    tail = block->prev_block;
  }
}

// ============================================================================
// Block 

struct mem_block *
split(struct mem_block *block, uint size)
{

  if (block == NULL) {
    return NULL;
  }

  uint block_size = get_size(block);

  uint min_split_size = size + sizeof(struct mem_block) + 16;

  if (block_size < min_split_size) {
    return NULL;
  }

  LOG("Splitting block %p (size=%d, request=%d)\n", block, block_size, size);

  // Calculate remaining size
  uint remaining_size = block_size - size;

  // Update original block size
  set_size(block, size);

  struct mem_block *new_block = (struct mem_block *)((char *)block + size);

  // Initialize new block
  memset(new_block->name, 0, 8);
  set_size(new_block, remaining_size);
  set_free(new_block);

  // Insert into block list (right after current)
  new_block->prev_block = block;
  new_block->next_block = block->next_block;

  if (block->next_block != NULL) {
    block->next_block->prev_block = new_block;
  } else {
    tail = new_block;
  }

  block->next_block = new_block;

  LOG("Split created new block %p (size=%d)\n", new_block, remaining_size);

  return new_block;
}

// ============================================================================
// Block 

void
merge_with_next(struct mem_block *block)
{
  if (block->next_block == NULL || !is_free(block->next_block)) {
    return;
  }

  LOG("Merging %p with next %p\n", block, block->next_block);

  struct mem_block *next = block->next_block;

  // Remove next from free list
  free_list_remove(next);

  // Expand current block
  uint new_size = get_size(block) + get_size(next);
  set_size(block, new_size);

  // Update block list pointers
  block->next_block = next->next_block;
  if (next->next_block != NULL) {
    next->next_block->prev_block = block;
  } else {
    tail = block;
  }
}

void
merge_with_prev(struct mem_block *block)
{
  if (block->prev_block == NULL || !is_free(block->prev_block)) {
    return;
  }

  LOG("Merging %p with prev %p\n", block, block->prev_block);

  struct mem_block *prev = block->prev_block;

  // Remove current from free list
  free_list_remove(block);

  // Expand previous block
  uint new_size = get_size(prev) + get_size(block);
  set_size(prev, new_size);

  // Update block list pointers
  prev->next_block = block->next_block;
  if (block->next_block != NULL) {
    block->next_block->prev_block = prev;
  } else {
    tail = prev;
  }
}

// ============================================================================
// Free Space 

struct mem_block *
find_free_block_first_fit(uint size)
{
  struct mem_block *current = free_head;

  while (current != NULL) {
    if (get_size(current) >= size) {
      return current;
    }
    current = get_free_node(current)->next_free;
  }

  return NULL;
}

struct mem_block *
find_free_block_best_fit(uint size)
{
  struct mem_block *current = free_head;
  struct mem_block *best = NULL;
  uint best_size = 0xFFFFFFFF;

  while (current != NULL) {
    uint current_size = get_size(current);
    if (current_size >= size && current_size < best_size) {
      best = current;
      best_size = current_size;
    }
    current = get_free_node(current)->next_free;
  }

  return best;
}

struct mem_block *
find_free_block_worst_fit(uint size)
{
  struct mem_block *current = free_head;
  struct mem_block *worst = NULL;
  uint worst_size = 0;

  while (current != NULL) {
    uint current_size = get_size(current);
    if (current_size >= size && current_size > worst_size) {
      worst = current;
      worst_size = current_size;
    }
    current = get_free_node(current)->next_free;
  }

  return worst;
}

struct mem_block *
reuse_block(uint size)
{
  struct mem_block *block = NULL;

  if (current_fsm == FIRST_FIT) {
    block = find_free_block_first_fit(size);
  } else if (current_fsm == BEST_FIT) {
    block = find_free_block_best_fit(size);
  } else if (current_fsm == WORST_FIT) {
    block = find_free_block_worst_fit(size);
  }

  return block;
}

// ============================================================================
// Malloc

void *
malloc(uint size)
{
  if (size == 0) {
    return NULL;
  }

  LOG("Allocation request: %d bytes\n", size);
  uint aligned_sz = align_size(size);
  uint total_sz = sizeof(struct mem_block) + aligned_sz;

    // Try to reuse a free block
  struct mem_block *block = reuse_block(total_sz);

  if (block != NULL) {
    LOG("Reusing block at %p\n", block);
    set_used(block);
    free_list_remove(block);

    // Try to split
    struct mem_block *split_block = split(block, total_sz);
    if (split_block != NULL) {
      free_list_add(split_block);
    }

    LOG("Allocation successful: %p\n", (char *)block + sizeof(struct mem_block));
    return (char *)block + sizeof(struct mem_block);
  }

  // 2. If not, then we can request a new block from the OS:
  uint page_sz = align_to_page(total_sz);
  block = (struct mem_block *)sbrk(page_sz);

  if (block == (void *)-1) {
    return NULL;
  }

  LOG("New block from sbrk: %p (size=%d)\n", block, page_sz);

  // Initialize block
  memset(block->name, 0, 8);
  set_size(block, page_sz);
  set_used(block);
  block->next_block = NULL;
  block->prev_block = NULL;

  // Add to block list
  block_list_add(block);

  // Split to create free block from leftover
  struct mem_block *split_block = split(block, total_sz);
  if (split_block != NULL) {
    free_list_add(split_block);
  }

  LOG("Allocation successful: %p\n", (char *)block + sizeof(struct mem_block));
  return (char *)block + sizeof(struct mem_block);
}

void
free(void *ptr)
{
  if (ptr == NULL) {
    return;
  }

  LOG("Free request: %p\n", ptr);
  struct mem_block *block = (struct mem_block *)((char *)ptr - sizeof(struct mem_block));

  set_free(block);
  free_list_add(block);

  LOG("Freed block: %p (size=%d)\n", block, get_size(block));

  // Merge with neighbors
  merge_with_next(block);
  merge_with_prev(block);

  // Release memory if at end and >= 4096
  while (tail != NULL && is_free(tail) && get_size(tail) >= 4096) {
      LOG("Releasing memory: %p (size=%d)\n", tail, get_size(tail));
  
      struct mem_block *to_release = tail;
      int release_size = get_size(to_release);  
      
      free_list_remove(to_release);
      block_list_remove(to_release);
  
      sbrk(-release_size); 
  }
}


void *
calloc(uint nmemb, uint size)
{
  char *mem = malloc(nmemb * size);
  if (mem == NULL) {
    return NULL;
  }
  memset(mem, 0, nmemb * size);
  return mem;
}

// takes a pointer to a previous allocation and resizes it (shrink / grow)
void *
realloc(void *ptr, uint size)
{
  // Edge case: NULL ptr = malloc
  if (ptr == NULL) {
    return malloc(size);
  }

  // Edge case: size 0 = free
  if (size == 0) {
    free(ptr);
    return NULL;
  }

  LOG("Realloc request: %p, size=%d\n", ptr, size);

  struct mem_block *block = (struct mem_block *)((char *)ptr - sizeof(struct mem_block));
  uint old_size = get_size(block);
  uint aligned_sz = align_size(size);
  uint new_total_sz = sizeof(struct mem_block) + aligned_sz;

  // Case 1: Block already has enough space
  if (new_total_sz <= old_size) {
    LOGP("Realloc: shrinking in place\n");

    // Try to split off excess
    struct mem_block *split_block = split(block, new_total_sz);
    if (split_block != NULL) {
      free_list_add(split_block);
      merge_with_next(split_block);
    }

    return ptr;
  }

  // Case 2: Try to expand into next block
  if (block->next_block != NULL && is_free(block->next_block)) {
    uint combined_size = old_size + get_size(block->next_block);
    if (combined_size >= new_total_sz) {
      LOGP("Realloc: expanding into next block\n");

      free_list_remove(block->next_block);
      merge_with_next(block);

      // Split if excess
      struct mem_block *split_block = split(block, new_total_sz);
      if (split_block != NULL) {
        free_list_add(split_block);
      }

      return ptr;
    }
  }

  // Case 3: Can't resize in place, allocate new block
  LOGP("Realloc: allocating new block\n");

  void *new_ptr = malloc(size);
  if (new_ptr == NULL) {
    return NULL;
  }

  // Copy old data
  uint copy_size = old_size - sizeof(struct mem_block);
  if (size < copy_size) {
    copy_size = size;
  }
  memcpy(new_ptr, ptr, copy_size);

  // Free old block
  free(ptr);

  return new_ptr;
}

// ============================================================================
//  Features

static void
print_padded_uint(uint n, int width)
{
  // Count digits
  int digits = 0;
  uint temp = n;
  if (temp == 0) {
    digits = 1;
  } else {
    while (temp > 0) {
      digits++;
      temp /= 10;
    }
  }
  
  // Print the number
  printf("%u", n);
  
  // Print padding spaces
  for (int i = digits; i < width; i++) {
    printf(" ");
  }
}

void
malloc_print(void)
{
  printf("-- Current Memory State --\n");

  struct mem_block *current = head;
  while (current != NULL) {
    uint block_size = get_size(current);
    printf("[BLOCK %p-%p] ", current, (char *)current + block_size);
    print_padded_uint(block_size, 8);
    printf(" [%s]  '%s'\n",
           is_free(current) ? "FREE" : "USED",
           current->name);
    current = current->next_block;
  }

  printf("\n-- Free List --\n");

  if (free_head == NULL) {
    printf("NULL\n");
    return;
  }

  current = free_head;
  while (current != NULL) {
    printf("[%p]", current);
    struct mem_block *next = get_free_node(current)->next_free;
    if (next != NULL) {
      printf(" -> ");
    }
    current = next;
  }
  printf(" -> NULL\n");
}

void
malloc_setfsm(int algorithm)
{
  current_fsm = algorithm;
}

void
malloc_name(void *ptr, char *name)
{
  if (ptr == NULL || name == NULL) {
    return;
  }

  struct mem_block *block = (struct mem_block *)((char *)ptr - sizeof(struct mem_block));

  int i = 0;
  while (i < 7 && name[i] != '\0') {
    block->name[i] = name[i];
    i++;
  }
  block->name[i] = '\0';
}

