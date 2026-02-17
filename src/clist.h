#ifndef __CLIST_H
#define __CLIST_H

#include <stdlib.h>
#include <memory.h>

/*
typedef struct GenericList {
	size_t count;		// How many valid items are currently in the list
	size_t capacity;	// How many items the list can currently hold. Additions above this will first cause it to reallocate with twice the capacity
	size_t size;		// Expected size of each item in list in bytes
	u_char array[];		// Holds the data. The type makes pointer arithmetic work in increments of 1 byte, but isn't indicative of what it holds. This is important!
} GenericList;

GenericList* CreateGenericList(size_t elementSize);
GenericList* ReallocateGenericList(GenericList* list);
void AddItemToGenericList(GenericList** list, void* item);
void RemoveLastItemFromGenericList(GenericList* list);
void* GetItemFromGenericList(GenericList* list, size_t index);
*/

typedef struct GenericPtrList {
	size_t count;		// How many valid items are currently in the list
	size_t capacity;	// How many items the list can currently hold. Additions above this will first cause it to reallocate with twice the capacity
	void* array[];	// Holds the data. The type makes pointer arithmetic work in increments of 1 byte, but isn't indicative of what it holds. This is important!
} GenericPtrList;

GenericPtrList* CreateGenericPtrList();
GenericPtrList* CreateGenericPtrListWithSize(size_t startingSize);
GenericPtrList* ReallocateGenericPtrList(GenericPtrList* list);
void AddItemToGenericPtrList(GenericPtrList** list, void* item);
void RemoveLastItemFromGenericPtrList(GenericPtrList* list);
void* GetItemFromGenericPtrList(GenericPtrList* list, size_t index);


typedef struct LLNode {
	void* data;
	struct LLNode* next;
} LLNode;

typedef struct LinkedList{
	LLNode* head;
	LLNode* tail;
} LinkedList;

LinkedList* CreateGenericLinkedList();
void AppendItemToLinkedList(LinkedList* list, void* item);
void RemoveItemFromLinkedList(LinkedList* list, void* item);
size_t GetLinkedListLength(LinkedList* list);



#endif
