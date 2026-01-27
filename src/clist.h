#ifndef __CLIST_H
#define __CLIST_H

#include <stdlib.h>
#include <memory.h>

typedef struct GenericList {
	size_t count;		// How many valid items are currently in the list
	size_t capacity;	// How many items the list can currently hold. Additions above this will first cause it to reallocate with twice the capacity
	size_t size;		// Expected size of each item in list in bytes
	u_char array[];		// Holds the data. The type makes pointer arithmetic work in increments of 1 byte, but isn't indicative of what it holds. This is important!
} GenericList;

typedef struct GenericPtrList {
	size_t count;		// How many valid items are currently in the list
	size_t capacity;	// How many items the list can currently hold. Additions above this will first cause it to reallocate with twice the capacity
	void* array[];	// Holds the data. The type makes pointer arithmetic work in increments of 1 byte, but isn't indicative of what it holds. This is important!
} GenericPtrList;

GenericList* CreateGenericList(size_t elementSize);
GenericList* ReallocateGenericList(GenericList* list);
void AddItemToGenericList(GenericList** list, void* item);
void RemoveLastItemFromGenericList(GenericList* list);
void* GetItemFromGenericList(GenericList* list, size_t index);

GenericPtrList* CreateGenericPtrList();
GenericPtrList* ReallocateGenericPtrList(GenericPtrList* list);
void AddItemToGenericPtrList(GenericPtrList** list, void* item);
void RemoveLastItemFromGenericPtrList(GenericPtrList* list);
void* GetItemFromGenericPtrList(GenericPtrList* list, size_t index);

#endif
