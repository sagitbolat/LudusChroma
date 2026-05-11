#pragma once
#include <stdint.h>
#include "skyintrinsics.h"
#include "sky_structs.h"

// SECTION: System-dependant page memory allocation for arena
#ifdef __linux__
#include <sys/mman.h>
#endif
#ifdef _WIN32
// TODO: Include virutual alloc instead of stdlib when changing code from malloc to VirtualAlloc.
#include <string.h>
#include <stdlib.h>
#endif


// SECTION: Arena Allocator


// Returns address of allocation.
void* ArenaAllocateAsset(ArenaAllocator* arena, size_t allocation_size) {
    // NOTE: increment address offset pointer.
    arena->offset_pointer += allocation_size;
    // NOTE: Returns pointer to allocated memory.
    return (void*)((uint8_t*)(arena->memory) + arena->offset_pointer - allocation_size);
}

void InitArena(ArenaAllocator* arena, uint64_t alloc_size) {
#ifdef __linux__
    arena->memory = mmap(
        nullptr, alloc_size, 
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANON | MAP_NORESERVE,
        -1, 0
    );
#endif
#ifdef _WIN32
    // TODO: Change to call VirtualAlloc() instead of malloc?
    arena->memory = malloc(alloc_size);
#endif
    arena->total_size = alloc_size;
    arena->offset_pointer = 0;
}

// NOTE: Sets all allocated values in the arena to value.
void MemsetArena(ArenaAllocator* arena, int value) {
#ifdef __linux__
    memset(arena->memory, value, arena->offset_pointer);
#endif
#ifdef _WIN32
    // TODO: Change to whatever the win32 equivalent is of memset.
    memset(arena->memory, value, arena->offset_pointer);
#endif
    arena->offset_pointer = 0;
}

// NOTE: Returns 0 if successful, -1 otherwise
int FreeArena(ArenaAllocator* arena) {
    int ret = 0;
#ifdef __linux__
    ret = munmap(arena->memory, arena->total_size); 
#endif
#ifdef _WIN32
    // TODO: Change to call VirtualAlloc() instead of malloc and free?
    free(arena->memory);
#endif
    return ret;
}




// SECTION: Pool allocator
struct PoolAllocator {

};

struct FreeHeader {

};

struct PoolListNode {
    FreeHeader data;
    PoolListNode* next;
};






// SECTION: Free-List allocator (general purpose)


struct FreeListHeader {
    size_t block_size;
    size_t padding;
};
struct FreeListNode {
    FreeListNode* next;
    size_t block_size;
};
struct FreeList {
    void* data;
    size_t size;
    size_t used;
    
    FreeListNode* head;
};

// NOTE: Returns 0 if successful, -1 otherwise.
int FreeListFreeAll(FreeList* free_list) {
    int ret = 0; // NOTE: Return value.
#ifdef __linux__
    ret = munmap(free_list->data, free_list->size); 
#endif
#ifdef _WIN32
    // TODO: Change to call VirtualAlloc() instead of malloc and free?
    free(free_list->data);
#endif
    free_list->size = 0;
    free_list->used = 0;
    return ret;
}

// NOTE: Init
void FreeListInit(FreeList* free_list, size_t size) {
    // NOTE: Reserve the memory
    free_list->size = size;

    if (free_list->data != nullptr) {
        FreeListFreeAll(free_list);
        free_list->data = nullptr;
    }
#ifdef __linux__
    free_list->data = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON | MAP_NORESERVE, -1, 0);
#endif
#ifdef _WIN32
    // TODO: Change to call VirtualAlloc() instead of malloc?
    free_list->data = malloc(size);
#endif
   // NOTE: set all the state for the freelist
   FreeListNode* first_node = (FreeListNode*)free_list->data;
   first_node->block_size = size;
   first_node->next = NULL;
   free_list->head = first_node;
}

// NOTE: alignment must be a power of two.
size_t PadWithHeader(uintptr_t ptr, uintptr_t alignment, size_t header_size) {

    // NOTE: This is equivalent to (ptr % alignment), but faster for powers of two
    uintptr_t mod = ptr & (alignment - 1); // NOTE: The current alignment of the ptr.
    uintptr_t padding = 0; // NOTE: The padding is 0 if mod is 0.
    if (mod != 0) {
        padding = alignment - mod; // NOTE: If mod is not 0, then padding is the complement of the alignment.
    }

    uintptr_t needed_space = (uintptr_t)header_size; // NOTE: Conversion for the comparison in next step

    if (padding < needed_space) {
        needed_space -= padding;
        if ((needed_space & (alignment - 1)) != 0) {
            padding += alignment * (1 + (needed_space/alignment));
        } else {
            padding += alignment * (needed_space/alignment);
        }
    }
    return (size_t)padding;
}

// NOTE: Returns first node that fits the specified size. 
FreeListNode* FreeListFindFirst(FreeList* free_list, size_t size, size_t alignment, size_t* p_padding, FreeListNode** p_prev_node) {
    FreeListNode* node = free_list->head;
    FreeListNode* prev_node = NULL;
    size_t padding = 0;

    while(node != NULL) {
        padding = PadWithHeader((uintptr_t)node, (uintptr_t)alignment, sizeof(FreeListHeader));
        size_t required_space = size + padding;
        if (node->block_size >= required_space) {
            break;
        }
        prev_node = node;
        node = node->next;
    }
    if (p_padding) {
        *p_padding = padding;
    }
    if (p_prev_node) {
        *p_prev_node = prev_node; 
    }
    return node;
}

// NOTE: Free List insertion and removal
void FreeListInsert(FreeListNode** phead, FreeListNode* prev_node, FreeListNode* new_node) {
    if (prev_node == NULL) {
        if (*phead != NULL) {
            new_node->next = *phead;
        } else {
            *phead = new_node;
        }
    } else {
        if (prev_node->next == NULL) {
            prev_node->next = new_node;
            new_node->next = NULL;
        } else {
            new_node->next = prev_node->next;
            prev_node->next = new_node;
        }
    }
}
void FreeListRemove(FreeListNode** phead, FreeListNode* prev_node, FreeListNode* del_node) {
    if (prev_node == NULL) {
        *phead = del_node->next;
    } else {
        prev_node->next = del_node->next;
    }
}

// NOTE: Allocates on the freelist allocator. 
void* FreeListAlloc(FreeList* free_list, size_t size, size_t alignment) {
    size_t padding = 0;
    FreeListNode* prev_node = NULL;
    FreeListNode* node = NULL;
    size_t alignment_padding, required_space, remaining;
    FreeListHeader* header_ptr;

    if (size < sizeof(FreeListNode)) {
        size = sizeof(FreeListNode);
    }
    if (alignment < 8) {
        alignment = 8;
    }

    node = FreeListFindFirst(free_list, size, alignment, &padding, &prev_node);
    
    if (node == NULL) {
        // NOTE: No free Memory. Maybe expand the arena here?
        return NULL;
    }

    alignment_padding = padding - sizeof(FreeListHeader);
    required_space = size + padding;
    remaining = node->block_size - required_space;

    if (remaining > 0 ) {
        FreeListNode* new_node = (FreeListNode*)((char*)node + required_space);
        new_node->block_size = remaining;
        FreeListInsert(&free_list->head, prev_node, new_node);
    }
    FreeListRemove(&free_list->head, prev_node, node);

    header_ptr = (FreeListHeader*)((char*)node + alignment_padding);
    header_ptr->block_size = required_space; 
    header_ptr->padding = alignment_padding;

    free_list->used += required_space;

    return (void*)((char*)header_ptr + sizeof(FreeListHeader));
}

// NOTE: Freeing and Coalescence
void FreeListCoalescence(FreeList* free_list, FreeListNode* prev_node, FreeListNode* free_node) {
    if (free_node->next != NULL && (void*)((char*)free_node + free_node->block_size) == free_node->next) {
        free_node->block_size += free_node->next->block_size;
        FreeListRemove(&free_list->head, free_node, free_node->next);
    }

    if (prev_node->next != NULL && (void*)((char*)prev_node + prev_node->block_size) == free_node) {
        prev_node->block_size += free_node->next->block_size;
        FreeListRemove(&free_list->head, prev_node, free_node);
    }
}

void FreeListFree(FreeList* free_list, void* ptr) {
    FreeListHeader* header;
    FreeListNode* free_node;
    FreeListNode* node;
    FreeListNode* prev_node = NULL;
    
    if (ptr == NULL) {
        return;
    }

    header = (FreeListHeader*)((char*)ptr - sizeof(FreeListHeader));
    free_node = (FreeListNode*)header;
    free_node->block_size = header->block_size + header->padding;
    free_node->next = NULL;
    
    node = free_list->head;
    while (node != NULL) {
        if (ptr < node) {
            FreeListInsert(&free_list->head, prev_node, free_node);
            break;
        }
        prev_node = node;
        node = node->next;
    }
    
    free_list->used = free_node->block_size;
    FreeListCoalescence(free_list, prev_node, free_node);
}



// SECTION: General Alloc and General Free:
namespace skymem { 
    bool freelist_init = false;
    size_t alignment = 64;
    FreeList free_list;

    void init(size_t init_size, size_t a) {
        FreeListInit(&free_list, init_size);
        alignment = a;
        freelist_init = true;
    }
    void* alloc(size_t size) {
        return FreeListAlloc(&free_list, size, alignment); 
    }
    void free(void* ptr) {
        FreeListFree(&free_list, ptr);
    }
    /*void* realloc(void* ptr, size_t old_size, size_t new_size) {
        void* newptr;

        int msize;
        msize

    }*/

}
