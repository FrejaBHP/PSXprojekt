#include "clist.h"

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


// ====================================================================

GenericPtrList* CreateGenericPtrList() {
	const int startingCapacity = 8;

	size_t initSize = sizeof(GenericPtrList) + (sizeof(u_long) * startingCapacity);
	GenericPtrList* newList = malloc(initSize);

	if (newList != NULL) {
		newList->capacity = startingCapacity;
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
		//const int offset = (*list)->size * (*list)->count;
		//memcpy(&(*list)->array[offset], item, (*list)->size);
		(*list)->count++;
	}

	//free(item);
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

