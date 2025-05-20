#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define TOTAL_MEMORY (512 * 1024) // 512KB total memory pool
#define MIN_BLOCK_SIZE (4 * 1024) // 4KB
#define MAX_LEVELS 7 // Since 512K / 2^7 = 4K

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// provided structures
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/**
 * Structure representing a node in the buddy allocator tree.
 * 
 * Each node represents a memory block of a specific size. The tree 
 * follows the buddy allocation strategy, where each node may be split
 * into two smaller "buddy" nodes. 
 * 
 * Fields:
 * - is_free: Indicates whether this block is currently available for allocation.
 * - is_split: True if this block has been split into smaller blocks.
 * - left: Pointer to the left child (first half of the split).
 * - right: Pointer to the right child (second half of the split).
 * - parent: Pointer to the parent node, used for coalescing blocks.
 * - size: The total size of memory this node represents.
 */

 typedef struct Node {
    bool is_free;               // True if this block is currently unallocated
    bool is_split;              // True if this block has been split into two smaller blocks
    struct Node* left;          // Pointer to the left child (first half of the split)
    struct Node* right;         // Pointer to the right child (second half of the split)
    struct Node* parent;        // Pointer to the parent node (used for merging)
    size_t size;                // Size of the block in bytes
    size_t mempool_offset;      // Offset in memory_pool representing this block
} Node;

/**
 * Structure representing the buddy memory allocator.
 * 
 * The allocator manages a fixed-size memory pool using a binary 
 * tree structure to efficiently allocate and deallocate memory.
 * 
 * Fields:
 * - root: Pointer to the root node of the buddy tree.
 * - memory_pool: A fixed array representing the entire memory pool.
 *   Memory allocations are returned as pointers within this array.
 */
typedef struct {
    Node* root; // Root of the binary tree used for allocation tracking
    char memory_pool[TOTAL_MEMORY]; // Fixed memory pool for all allocations
} BuddyAllocator;


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// provided utility functions
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/**
 * Prints the details of a given node for debugging purposes.
 * 
 * ## Usage:
 * - Call this function when you need to inspect the properties of a node 
 *   during allocation, deallocation, or tree traversal.
 * - Pass a descriptive `message` to help identify where in the process 
 *   the function is being called.
 * - If the `node` is `NULL`, the function will print an appropriate message 
 *   and return immediately.
 * 
 * ## Example:
 * ```c
 * print_node_details(node, "Checking node before allocation");
 * ```
 * 
 * ## Output:
 * ```
 * Checking node before allocation: Node size=64K, offset=128K, is_split=1, is_free=0
 * ```
 * 
 * ## Notes:
 * - The size and offset are displayed in kilobytes for readability.
 * - `is_split` indicates whether the node has been divided into two smaller blocks.
 * - `is_free` shows whether the node is available for allocation.
 * - This function does not modify any node properties—use it purely for debugging.
 */
void print_node_details(Node* node, const char* message) {
    if (node == NULL) {
        printf("%s: Node is NULL\n", message);
        return;
    }
    printf("%s: Node size=%zuK, offset=%zuK, is_split=%d, is_free=%d\n",
           message, node->size / 1024, node->mempool_offset / 1024, node->is_split, node->is_free);
}

/**
 * Prints the structure of the buddy allocator tree for visualization.
 * 
 * ## Usage:
 * - Call this function to inspect the current state of the memory allocation.
 * - Pass the root node to print the entire tree.
 * - The `depth` parameter controls indentation to visually represent the hierarchy.
 * 
 * ## Example:
 * ```c
 * print_tree(allocator->root, 0);
 * ```
 * 
 * ## Output:
 * ```
 * FS (512K)
 *   FS (256K)
 *     FS (128K)
 *       F (64K)
 *       A (64K)
 * ```
 * 
 * ## Legend:
 * - `"FS"` indicates a split node that contains further subdivisions.
 * - `"F"` represents a free block available for allocation.
 * - `"A"` represents an allocated block.
 * - Indentation increases with depth to reflect the tree structure.
 * 
 * ## Notes:
 * - This function does not modify the tree; it is purely for debugging.
 * - If a node is not split, it will print either `"F"` (free) or `"A"` (allocated).
 * - Recursively prints left and right child nodes to fully display the tree.
 */
void print_tree(Node* node, int depth) {
    if (node == NULL) return;

    for (int i = 0; i < depth; i++) printf("  ");

    if (!node->is_split)
        printf("%s (%zuK)\n", node->is_free ? "F" : "A", node->size / 1024);
    else
        printf("FS (%zuK)\n", node->size / 1024);
        
    print_tree(node->left, depth + 1);
    print_tree(node->right, depth + 1);
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/**
 * Creates a new node in the buddy system.
 * 
 * This function initializes a new node representing a block of memory in the 
 * buddy allocator system. Each node corresponds to a contiguous region of memory 
 * within the overall memory pool. Nodes are structured hierarchically, forming 
 * a binary tree where each non-leaf node is a split block, and each leaf node 
 * represents an allocated or free memory block.
 * 
 * ## Parameters:
 * - `size`: The size of the memory block this node represents. This value must 
 *   always be a power of two and corresponds to the total space managed by this node.
 * - `mempool_offset`: The starting offset of this block within the memory pool. 
 *   This value is used to locate the memory associated with this node.
 * - `parent`: A pointer to the parent node in the buddy tree. If this is the 
 *   root node, the parent is NULL.
 * 
 * ## Behavior:
 * - The function allocates memory for a new `Node` structure.
 * - It marks the node as free (`is_free = true`) because newly created nodes are 
 *   initially unallocated.
 * - It marks the node as not split (`is_split = false`) since splitting occurs 
 *   only when a request requires a smaller block.
 * - It sets `left` and `right` child pointers to `NULL` since this node has not 
 *   been split yet.
 * - It sets the `parent` pointer to the provided parent node.
 * - It assigns the given `size` and `mempool_offset` values to track the memory 
 *   block’s characteristics.
 * - The function returns a pointer to the newly created node.
 * 
 * ## Notes for Students:
 * - This function does not handle memory allocation failures (e.g., `malloc` 
 *   returning NULL). Consider what happens if `malloc` fails in a real-world system.
 * - `mempool_offset` is critical for tracking memory locations. When splitting a 
 *   node, the left child should inherit the same `mempool_offset`, while the right 
 *   child should be offset by `size / 2`. Ensure you understand how offsets change 
 *   when nodes are split.
 * - The returned node is always a single, unallocated block of memory until it is 
 *   either allocated or split into smaller blocks.
 * 
 * @param size The size of the memory block this node represents.
 * @param mempool_offset The offset in the memory pool where this block starts.
 * @param parent A pointer to the parent node, or NULL if this is the root.
 * @return A pointer to the newly created node.
 */
Node* create_node(size_t size, size_t mempool_offset, Node* parent) {
    Node* node = (Node*)malloc(sizeof(Node));
    if (!node) {
        return NULL;
    }

    node->is_free = true;
    node->is_split = false;
    node->left = NULL;
    node->right = NULL;
    node->parent = parent;
    node->size = size;
    node->mempool_offset = mempool_offset;

    return node;
}




/**
 * Creates a new buddy allocator instance.
 * 
 * This function initializes the buddy memory allocator by creating a root node 
 * that represents the entire available memory pool. The buddy allocator is 
 * structured as a binary tree, where each node represents a contiguous block 
 * of memory that can be split into smaller blocks as allocations occur.
 * 
 * ## Parameters:
 * - This function does not take any parameters since it initializes the entire 
 *   memory pool from a predefined `TOTAL_MEMORY` size.
 * 
 * ## Behavior:
 * - Allocates memory for a `BuddyAllocator` structure, which acts as the 
 *   top-level manager for the allocation system.
 * - Calls `create_node` to generate the root node of the buddy tree:
 *   - The root node represents the full memory pool.
 *   - The `mempool_offset` is set to `0` since the root starts at the beginning 
 *     of the memory pool.
 *   - The parent of the root node is `NULL` because it has no parent.
 * - Returns a pointer to the newly created allocator.
 * 
 * ## Notes for Students:
 * - The buddy system starts with one large free block. As allocations occur, 
 *   the system will split blocks to create appropriately sized chunks.
 * - `TOTAL_MEMORY` must be a power of two for the buddy system to function 
 *   correctly, as each split produces two equal-sized child blocks.
 * - The allocator only manages metadata about the memory. The actual memory 
 *   pool would typically be stored separately (e.g., as an array or direct 
 *   system memory allocation).
 * - Consider what happens if `malloc` fails when creating the allocator or the 
 *   root node. In a production system, how should failures be handled?
 * 
 * @return A pointer to the newly created `BuddyAllocator` structure.
 */
BuddyAllocator* create_allocator() {
    BuddyAllocator* allocator = (BuddyAllocator*)malloc(sizeof(BuddyAllocator));
    if (!allocator) {
        return NULL;
    }

    allocator->root = create_node(TOTAL_MEMORY, 0, NULL);
    if (!allocator->root) {
        free(allocator);
        return NULL;
    }

    return allocator;
}


/**
 * Splits a given node into two smaller buddy blocks.
 *
 * ## Purpose:
 * - This function is responsible for dividing a memory block into two equal-sized sub-blocks.
 * - Used when an allocation request requires a smaller block than the current node size.
 *
 * ## Expected Behavior:
 * - The function should only proceed if:
 *   - The node is **not already split**.
 *   - The node size is **greater than** the minimum block size (`MIN_BLOCK_SIZE`).
 * - Creates two child nodes:
 *   - **Left child**: Takes the same memory pool offset as the parent.
 *   - **Right child**: Takes the offset of the left child **plus half the parent size**.
 * - Marks the current node as **split** (`is_split = true`).
 *
 * ## Example:
 * ```
 * Initial Tree:
 *   F (64K)
 * 
 * After `split(node)` on 64K block:
 *   FS (64K)
 *     F (32K)
 *     F (32K)
 * ```
 *
 * ## Notes:
 * - **This function does not allocate memory**; it simply creates structure nodes.
 * - **The actual memory division happens logically** through the buddy system.
 * - The caller must check that `split` is necessary before calling it.
 * - The **offset of the right child** ensures proper tracking of memory segments.
 *
 * ## Potential Issues:
 * - If the node is already split, calling `split` again should do nothing.
 * - A node **must** have a size greater than `MIN_BLOCK_SIZE` to split.
 * - If `create_node` fails (returns NULL), the function should handle it gracefully (students should consider this).
 */
void split(Node* node) {
    if (!node || node->is_split || node->size == MIN_BLOCK_SIZE) {
        return;
    }

    size_t half_size = node->size / 2;
    node->left = create_node(half_size, node->mempool_offset, node);
    node->right = create_node(half_size, node->mempool_offset + half_size, node);

    if (!node->left || !node->right) {
        if (node->left) free(node->left);
        if (node->right) free(node->right);
        node->left = node->right = NULL;
        return;
    }

    node->left->is_free = true;
    node->right->is_free = true;

    node->is_split = true;
    node->is_free = false;
}





/**
 * Recursively allocates memory from the buddy system.
 *
 * ## Purpose:
 * - Searches for a free block of the requested `size` within the buddy system.
 * - If a sufficiently sized block is found, it is allocated.
 * - If the block is too large, it is split into two smaller blocks, and allocation continues recursively.
 *
  * ## Expected Behavior:
 * - The function should stop searching if the node cannot be used for allocation, either because it is 
 *   too small, already allocated, or missing.
 * - If the node is a perfect fit for the request, it should be assigned only if it hasn’t been previously divided.
 * - When the node is larger than necessary, it should be split into two equal parts if it hasn’t been divided already.
 * - The function should attempt to allocate from the left subtree first and only check the right subtree if 
 *   the left cannot fulfill the request.
 *
 * ## Example:
 * ```
 * Initial Tree:
 *   F (64K)
 *
 * Allocating 16K:
 *   FS (64K)
 *     FS (32K)
 *       FS (16K)
 *         A (16K)
 *         F (16K)
 *       F (32K)
 * ```
 *
 * ## Notes:
 * - **Splitting only happens when needed**, keeping fragmentation minimal.
 * - **Left-first allocation** ensures small blocks cluster toward the left side of the tree.
 * - The function naturally **backtracks** if a smaller block isn't available in the left subtree.
 * - **Returning NULL means allocation failed** due to lack of sufficient free space.
 *
 * ## Potential Issues:
 * - If the recursion is not correctly implemented, **memory fragmentation may occur**.
 * - If a node is already split but its children are fully allocated, **allocation should fail**.
 * - Ensure that allocated nodes are correctly marked **not free** (`is_free = false`).
 * - Edge cases:
 *   - **Allocating more than available memory** should fail.
 *   - **Allocating less than `MIN_BLOCK_SIZE`** may require rounding up.
 *   - **Handling multiple allocations in sequence** should still leave space for future allocations.
 */
Node* allocate_recursive(Node* node, size_t size) {
    if (!node || node->size < size) {
        return NULL;
    }

    if (!node->is_free && !node->is_split) {
        return NULL;
    }

    if (node->size == size && !node->is_split) {
        node->is_free = false;
        return node;
    }

    if (!node->is_split && node->size > size) {
        split(node);
    }

    if (node->is_split) {
        Node* result = allocate_recursive(node->left, size);
        if (result) return result;
        return allocate_recursive(node->right, size);
    }

    return NULL;
}









/**
 * Allocate memory from the buddy allocator.
 *
 * This function requests a block of memory from the buddy allocator system. It ensures 
 * the requested size adheres to the minimum block size and does not exceed the total 
 * available memory. The allocation is performed recursively, following the buddy 
 * allocation strategy.
 *
 * ## How It Works:
 * - If the requested `size` is smaller than the minimum block size, it is adjusted to `MIN_BLOCK_SIZE`.
 * - If the requested `size` is larger than the total available memory, the function returns `NULL`.
 * - The allocation process is delegated to `allocate_recursive`, which:
 *   - Traverses the buddy tree to find a suitable free block.
 *   - Splits larger blocks as needed to create the smallest possible fit.
 *   - Allocates the first suitable free block found.
 * - If no block is found, the function returns `NULL`.
 * - If allocation succeeds, the function computes the memory address based on the 
 *   offset within the allocator’s memory pool and returns a pointer to the allocated memory.
 *
 * ## Expected Behavior:
 * - Requests smaller than `MIN_BLOCK_SIZE` are rounded up.
 * - Requests larger than `TOTAL_MEMORY` fail immediately.
 * - If a block is available, a pointer to the allocated memory is returned.
 * - If no suitable block exists, `NULL` is returned.
 *
 * @param allocator A pointer to the `BuddyAllocator` instance managing memory.
 * @param size The size of the requested memory block.
 * @return A pointer to the allocated memory, or `NULL` if allocation fails.
 */
void* allocate(BuddyAllocator* allocator, size_t size) {
    if (!allocator || size == 0 || size > TOTAL_MEMORY) {
        return NULL;
    }

    if (size < MIN_BLOCK_SIZE) {
        size = MIN_BLOCK_SIZE;
    }

    size_t adjusted_size = MIN_BLOCK_SIZE;
    while (adjusted_size < size) {
        adjusted_size *= 2;
    }

    Node* allocated_node = allocate_recursive(allocator->root, adjusted_size);
    if (!allocated_node) {
        return NULL;
    }

    return allocator->memory_pool + allocated_node->mempool_offset;
}


/**
 * Coalesce (merge) buddy blocks to restore larger free memory blocks.
 *
 * This function attempts to merge adjacent free blocks (buddies) back into a single
 * larger block whenever possible. The merging process follows the buddy system rules:
 * two adjacent free blocks of the same size can be combined into their parent block.
 *
 * ## How It Works:
 * - If `node` is `NULL` or does not have a parent, no merging is possible.
 * - The function checks if both the left and right child nodes of the parent:
 *   - Are free.
 *   - Have not been further split.
 * - If both conditions are met, the two child nodes are freed, and the parent is marked
 *   as a free, non-split block.
 * - The function then **recursively** attempts to coalesce at the next level up,
 *   potentially merging larger blocks all the way to the root.
 *
 * ## Expected Behavior:
 * - Ensures that memory fragmentation is minimized by merging free buddies when possible.
 * - Recursively merges as high up in the tree as possible.
 * - Does **not** merge if any of the buddies are still allocated or further subdivided.
 *
 * @param node A pointer to the node being freed, which may trigger coalescing.
 */
void coalesce(Node* node) {
    if (!node || !node->parent) {
        return;
    }

    Node* parent = node->parent;
    Node* left = parent->left;
    Node* right = parent->right;

    if (left && right && left->is_free && right->is_free &&
        !left->is_split && !right->is_split) {

        free(left);
        free(right);
        parent->left = NULL;
        parent->right = NULL;
        parent->is_split = false;
        parent->is_free = true;

        coalesce(parent);
    }
}


/**
 * Recursively frees a memory block and attempts to merge it with its buddy.
 *
 * This function marks a given node as free and then attempts to coalesce it
 * with its buddy to restore larger contiguous free memory blocks.
 *
 * ## How It Works:
 * - If the `node` is `NULL`, do nothing.
 * - Mark the node as **free**.
 * - Attempt to **coalesce** it with its buddy by calling `coalesce(node)`.
 *
 * ## Expected Behavior:
 * - Ensures that memory blocks are properly freed when no longer needed.
 * - Triggers coalescing to prevent memory fragmentation.
 * - Does **not** free a block if it has already been merged into a larger unit.
 *
 * @param node A pointer to the node representing the allocated memory block.
 */
void free_recursive(Node* node) {
    if (!node) {
        return;
    }

    node->is_free = true;
    coalesce(node);
}



/**
 * Recursively finds a node in the buddy system tree based on its memory pool offset.
 *
 * This function searches for the node corresponding to the given `mempool_offset`,
 * ensuring that only leaf nodes (nodes that are not split) are considered valid matches.
 *
 * ## How It Works:
 * - If `node` is NULL, return NULL (base case).
 * - If the node's `mempool_offset` matches the given `mempool_offset` **and**
 *   the node is **not split**, return this node as the result.
 * - Otherwise, determine which subtree to search:
 *   - If the `mempool_offset` falls within the **left child's** range, recurse left.
 *   - Otherwise, recurse right.
 *
 * ## Expected Behavior:
 * - Finds and returns the **first unsplit node** at the given `mempool_offset`.
 * - Ensures that only leaf nodes are returned to prevent incorrect deallocation.
 * - Traverses the tree efficiently, reducing unnecessary comparisons.
 *
 * @param node A pointer to the current node in the buddy system tree.
 * @param mempool_offset The offset within the memory pool to find.
 * @return A pointer to the matching node if found, or NULL if no match exists.
 */

 Node* find_node(Node* node, size_t mempool_offset) {
    if (!node) {
        return NULL;
    }

    if (!node->is_split && node->mempool_offset == mempool_offset) {
        return node;
    }

    if (!node->is_split) {
        return NULL;
    }

    if (mempool_offset < node->right->mempool_offset) {
        return find_node(node->left, mempool_offset);
    } else {
        return find_node(node->right, mempool_offset);
    }
}



/**
 * Converts a memory address back to its corresponding node and frees it.
 *
 * This function determines the memory block associated with a given pointer,
 * verifies its validity, and then frees it recursively while ensuring proper merging.
 *
 * ## How It Works:
 * - Computes the **offset** of `ptr` from the start of `memory_pool`.
 * - If the **offset is out of bounds**, the function returns immediately.
 * - Searches for the corresponding node using `find_node()`.
 * - If the node is found and is **currently allocated**, it is freed using `free_recursive()`.
 *
 * ## Expected Behavior:
 * - Ensures that only valid allocated blocks are freed.
 * - Prevents double frees by checking `is_free` before calling `free_recursive()`.
 * - Preserves memory structure integrity by preventing invalid deallocations.
 *
 * @param allocator A pointer to the buddy allocator structure.
 * @param ptr A pointer to the memory block that needs to be freed.
 */
void deallocate(BuddyAllocator* allocator, void* ptr) {
    if (!allocator || !ptr) {
        return;
    }

    size_t offset = (char*)ptr - allocator->memory_pool;
    if (offset >= TOTAL_MEMORY) {
        return;
    }

    Node* node = find_node(allocator->root, offset);
    if (!node || node->is_free) {
        return;
    }

    free_recursive(node);
}



/**
 * Recursively frees all nodes in the buddy allocator tree.
 *
 * This function traverses the buddy system tree and deallocates all memory
 * associated with it. It ensures that all allocated nodes, including both
 * left and right children, are properly freed.
 *
 * ## How It Works:
 * - If `node` is NULL, return immediately (base case).
 * - Recursively call `destroy_tree()` on the **left** child.
 * - Recursively call `destroy_tree()` on the **right** child.
 * - After both children are freed, **free the current node itself**.
 *
 * ## Expected Behavior:
 * - Deallocates the entire buddy system tree safely.
 * - Prevents memory leaks by ensuring all dynamically allocated nodes are freed.
 * - Handles both leaf nodes and internal nodes appropriately.
 *
 * @param node A pointer to the root node of the tree (or subtree).
 */
void destroy_tree(Node* node) {
    if (!node) {
        return;
    }

    destroy_tree(node->left);
    destroy_tree(node->right);
    free(node);
}


/**
 * Frees all memory associated with the buddy allocator.
 *
 * This function ensures that the entire buddy system tree is properly deallocated 
 * before freeing the allocator itself.
 *
 * ## How It Works:
 * - Calls `destroy_tree(allocator->root)` to recursively free all nodes in the tree.
 * - Frees the `allocator` structure itself to release the remaining memory.
 *
 * ## Expected Behavior:
 * - Properly deallocates all dynamically allocated memory within the allocator.
 * - Ensures no memory leaks by freeing both the tree and the allocator structure.
 * - Should only be called when the allocator is no longer needed.
 *
 * @param allocator A pointer to the `BuddyAllocator` instance to be freed.
 */
void destroy_allocator(BuddyAllocator* allocator) {
    if (!allocator) {
        return;
    }

    destroy_tree(allocator->root);
    free(allocator);
}


// do not remove this line, your main will not be used when this is set
// Note, if you started with the version of the scaffold that did not include
// this then you must either use this scaffold or add this exactly as it appears
// here and below to your own file. 
// 
#ifndef NOMAIN     

// Main function for testing
int main() {
    BuddyAllocator* allocator = create_allocator();

    printf("\nInitial Tree\n");
    print_tree(allocator->root, 0);

    printf("\nAllocating 16KB\n");
    void* block1 = allocate(allocator, 16 * 1024);
    print_tree(allocator->root, 0);

    printf("\nAllocating 8KB\n");
    void* block2 = allocate(allocator, 8 * 1024);
    print_tree(allocator->root, 0);

    printf("\nFreeing 16KB\n");
    deallocate(allocator, block1);
    print_tree(allocator->root, 0);

    printf("\nFreeing 8KB\n");
    deallocate(allocator, block2);
    print_tree(allocator->root, 0);

    destroy_allocator(allocator);
    return 0;
}

// do not remove this line, your main will not be used when this is set
// Note, if you started with the version of the scaffold that did not include
// this then you must either use this scaffold or add this exactly as it appears
// here and below to your own file. 
// 
#endif // NOMAIN     

