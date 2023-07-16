#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include "linked_list.h"

linked_list* create_list(size_t element_size) {
	linked_list* li = (linked_list*)malloc(sizeof(linked_list));
	if (li == NULL) {
		printf("list creation fail");
		return NULL;
	}
	li->element_size = element_size;
	li->length = 0;
	li->root = NULL;

	return li;
}

void list_add(linked_list* li, void* element) {
	list_add_at(li, element, li->length);
}

void list_add_at(linked_list* li, void* element, uint index) {
	printf("index: %i\n", index);

	if (li->root == NULL) {
		li->root = malloc(sizeof(ll_node));

		if (li->root == NULL) {
			printf("failed to init node in list at: %i\n", index);
			return;
		}

		li->root->data = malloc(li->element_size);

		if (li->root->data == NULL) {
			printf("failed to init node data in list at: %i\n", index);
			return;
		}

		memcpy(li->root->data, element, li->element_size);
		li->root->next = NULL;
		li->length++;
		return;
	}

	ll_node* prev = li->root;
	uint i = 0;
	//printf("------------------\n");
	while (i++ < index-1) {
		//printf("index: %i, cur: %p, next: %p\n", i-1, prev, prev->next);
		prev = prev->next;
	}

	//the pointer to the node at the targeted index
	ll_node* cur_tmp = prev->next;

	//node to be placed at the index
	ll_node* new_cur = malloc(sizeof(ll_node));

	if (new_cur == NULL) {
		printf("failed to init node in list at: %i\n", index);
		return;
	}

	new_cur->data = malloc(li->element_size);

	if (new_cur->data == NULL) {
		printf("failed to init node data in list at: %i\n", index);
		return;
	}

	memcpy(new_cur->data, element, li->element_size);
	new_cur->next = cur_tmp;
	prev->next = new_cur;

	li->length++;
}

void list_remove(linked_list* li, uint index) {
	if (index == 0) {
		ll_node* nxt = li->root->next;
		free(li->root->data);
		free(li->root);
		li->root = nxt;
		li->length--;
		return;
	}

	ll_node* prev = li->root;
	uint i = 0;
	while (i++ < index - 1) {
		prev = prev->next;
	}

	ll_node* target = prev->next;
	ll_node* nxt = target->next;
	free(target->data);
	free(target);

	prev->next = nxt;
	li->length--;
}

void free_list(linked_list* list){
	_free_list(list->root);
	free(list->root->data);
	free(list->root);
	free(list);
}

void _free_list(ll_node* root) {
	if (root->next == NULL) {
		return;
	}

	_free_list(root->next);

	free(root->next->data);
	free(root->next);
}

void print_list(linked_list* list) {
	ll_node* tmp = list->root;
	printf("[");
	while (tmp) {
		printf("%p, ", tmp->data);
		tmp = tmp->next;
	}
	printf("]\n");
}

void* list_get(linked_list* li, uint index) {
	ll_node* cur = li->root;
	uint i = 0;
	while (i++ < index) {
		cur = cur->next;
	}
	return cur->data;
}

uint list_index_of(linked_list* list, element_equals compare, void* find) {
	ll_node* cur = list->root;
	uint i = 0;
	while (i < list->length) {

		bool res = compare(cur->data, &find);
		if (res)
			return true;

		i++;
		cur = cur->next;
	}
	return -1;
}