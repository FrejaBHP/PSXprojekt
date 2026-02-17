#include "clist.h"
#include <stdbool.h>

/*
GenericList* CreateGenericList(size_t elementSize) {
	const int startingCapacity = 8;

	size_t initSize = sizeof(GenericList) + (elementSize * startingCapacity);
	GenericList* newList = malloc(initSize);

	if (newList != NULL) {
		newList->capacity = startingCapacity;
		newList->count = 0;
		newList->size = elementSize;
	}

	return newList;
}

GenericList* ReallocateGenericList(GenericList* list) {
	size_t oldSize = sizeof(GenericList) + (list->size * list->capacity);
	size_t newSize = sizeof(GenericList) + (list->size * list->capacity * 2);

	GenericList* movedList = malloc(newSize);

	if (movedList != NULL) {
		memcpy(movedList, list, oldSize);
		movedList->capacity *= 2;

		free(list);

		return movedList;
	}

	return list;
}

void AddItemToGenericList(GenericList** list, void* item) {
	// If list has reached capacity, reallocate and double in size
	if ((*list)->count == (*list)->capacity) {
		*list = ReallocateGenericList(*list);
	}

	// Second check in case the list returned is the same one due to an error with malloc
	if ((*list)->count < (*list)->capacity) {
		const int offset = (*list)->size * (*list)->count;
		memcpy(&(*list)->array[offset], item, (*list)->size);
		(*list)->count++;
	}

	free(item);
}

void RemoveLastItemFromGenericList(GenericList* list) {
	if (list->count != 0) {
		list->count--;
	}
}

void* GetItemFromGenericList(GenericList* list, size_t index) {
	if (list->count <= index) {
		//fprintf(stderr, "Accessed invalid list index or indexed out of bounds");
		//abort();
	}
	
	return &list->array[list->size * index];
}
*/

// ====================================================================

// Default starting capacity is 8
GenericPtrList* CreateGenericPtrList() {
	const size_t startingCapacity = 8;

	size_t initSize = sizeof(GenericPtrList) + (sizeof(u_long) * startingCapacity);
	GenericPtrList* newList = malloc(initSize);

	if (newList != NULL) {
		newList->capacity = startingCapacity;
		newList->count = 0;
	}

	return newList;
}

GenericPtrList* CreateGenericPtrListWithSize(size_t startingSize) {
	size_t initSize = sizeof(GenericPtrList) + (sizeof(u_long) * startingSize);
	GenericPtrList* newList = malloc(initSize);

	if (newList != NULL) {
		newList->capacity = startingSize;
		newList->count = 0;
	}

	return newList;
}

GenericPtrList* ReallocateGenericPtrList(GenericPtrList* list) {
	size_t oldSize = sizeof(GenericPtrList) + (sizeof(u_long) * list->capacity);
	size_t newSize = sizeof(GenericPtrList) + (sizeof(u_long) * list->capacity * 2);

	GenericPtrList* movedList = malloc(newSize);

	if (movedList != NULL) {
		memcpy(movedList, list, oldSize);
		movedList->capacity *= 2;

		free(list);

		return movedList;
	}

	return list;
}

void AddItemToGenericPtrList(GenericPtrList** list, void* item) {
	// If list has reached capacity, reallocate and double in size
	if ((*list)->count == (*list)->capacity) {
		*list = ReallocateGenericPtrList(*list);
	}

	// Second check in case the list returned is the same one due to an error with malloc
	if ((*list)->count < (*list)->capacity) {
        (*list)->array[(*list)->count] = item;
		(*list)->count++;
	}
}

void RemoveLastItemFromGenericPtrList(GenericPtrList* list) {
	if (list->count != 0) {
		list->count--;
	}
}

void* GetItemFromGenericPtrList(GenericPtrList* list, size_t index) {
	if (list->count <= index) {
		//fprintf(stderr, "Accessed invalid list index or indexed out of bounds");
		//abort();
	}
	
	return list->array[index];
}


// ====================================================================

LinkedList* CreateGenericLinkedList() {
	LinkedList* list = malloc(sizeof(LinkedList));
	list->head = NULL;
	list->tail = NULL;

	return list;
}

void AppendItemToLinkedList(LinkedList* list, void* item) {
	LLNode* newNode = malloc(sizeof(LLNode));

	if (list->head == NULL) {
		list->head = newNode;
	}

	newNode->data = item;
	newNode->next = NULL;

	if (list->tail != NULL) {
		list->tail->next = newNode;
	}

	list->tail = newNode;
}

void RemoveItemFromLinkedList(LinkedList* list, void* item) {
	LLNode* prevNode = NULL;
	LLNode* node = list->head;

	bool itemFound = false;

	while (node != NULL) {
		if (node->data == item) {
			itemFound = true;
			break;
		}

		prevNode = node;
		node = node->next;
	}

	if (itemFound) {
		// If node is the first element
		if (list->head == node) {
			// If first element is the ONLY element
			if (node->next == NULL) {
				list->head = NULL;
				list->tail = NULL;
			}
			else {
				list->head = node->next;
			}
		}
		// If node is the last element
		else if (list->tail == node) {
			prevNode->next = NULL;
			list->tail = prevNode;
		}
		// If node has another node after it
		// This case should always be hit if the two above aren't, but still doing an extra cautionary nullptr check
		else if (node->next != NULL) {
			prevNode->next = node->next;
		}

		free(item);
		free(node);
	}
}

size_t GetLinkedListLength(LinkedList* list) {
	LLNode* node = list->head;
	size_t count = 0;

	while (node != NULL) {
		node = node->next;
		count++;
	}

	return count;
}
