#pragma once

#include <memory.h>
#include <stdbool.h>

#define uint unsigned int

typedef bool (*element_equals)(void* e1, void* e2);

typedef struct _ll_node ll_node;
typedef struct _ll_node {
	ll_node* next;
	void* data;
} ll_node;

typedef struct _linked_list {
	ll_node* root;
	size_t length;
	size_t element_size;
} linked_list;


linked_list* create_list(size_t element_size);
void list_add(linked_list* list, void* element);
void list_add_at(linked_list* list, void* element, uint index);
void* list_get(linked_list* list, uint index);
uint list_index_of(linked_list* list, element_equals compare, void* find);
void list_remove(linked_list* list, uint index);

// If elements are pointers, they are assumed to be already freed
void free_list(linked_list* list);
void _free_list(ll_node* root);

void print_list(linked_list* list);