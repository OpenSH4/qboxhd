/**
 * @file  slist.c
 * @brief Implementation of a Single Linked List
 *
 * Copyright (c) 2007 Duolabs S.r.l.
 *
 * Changelog:
 * Date    Author      Comments
 * ------------------------------------------------------------------------
 * 191007  paguilar    Original
 */

/*****************************
 * INCLUDES
 *****************************/

#include "duoutils.h"

/*****************************
 * FUNCTION IMPLEMENTATION
 *****************************/

/**
 * @brief Creates an empty list
 */
SList slist_new() {
    return (SList)NULL;
}

/**
 * @brief Adds a new node to the end of the SList
 * @param node The list
 * @param data The new node to be added
 * @return The list with the new node
 */
SList slist_append(SList node, void *data) {
    SList	ptr = NULL,
		aux = NULL;

    if ((ptr = (SList)xmalloc(sizeof(struct SList_st), 1)) == NULL) {
	return NULL;
    }

    ptr->data = data;
    ptr->next = NULL;

    aux = node;
    if (!aux)
	return ptr;

    while (aux->next) {
	aux = aux->next;
    }
    aux->next = ptr;

    return node;
}

/**
 * @brief Inserts a new node into the slist at the given position
 * @param node The list
 * @param data The new node to be added
 * @param position The position where the new node will be inserted
 * @return The list with the new node
 */
SList slist_insert(SList node, void *data, int position) {
    int		counter = 0;
    SList       ptr = NULL,
                aux = NULL;

    if (position < 0 || slist_count(node) < position) {
	node = slist_append(node, data);
	return node;
    }

    aux = node;
    if (position > 0)
        counter++;

    while (aux && counter != position) {
	aux = aux->next;
	counter++;
    }

    if ((ptr = (SList)xmalloc(sizeof(struct SList_st), 1)) == NULL) {
        return NULL;
    }

    ptr->data = data;

    if (!position) {
	ptr->next = aux;
	node = ptr;
    }
    else {
        ptr->next = aux->next;
        aux->next = ptr;
    }

    return node;
}

/**
 * @brief Removes the element from a slist at the given position 
 *        If the position is negative or greater than the size of the slist, 
 *        then the slist is unchanged
 * @param node The list
 * @param position The element at this position will be removed
 * @return The list with the removed node
 */
SList slist_remove(SList node, int position) {
    int		counter = 0;
    SList	ptr = NULL;

    if (position < 0 || slist_count(node) < position) {
	return node;
    }

    ptr = node;
    if (position > 0)
        counter++;

    while (ptr && counter != position) {
        ptr = ptr->next;
        counter++;
    }

    if (!position) {
	node = node->next;
	xfree(ptr);
    }
    else {
	ptr--;
	ptr->next = ptr->next->next;
	xfree(ptr->next);
    }

    return node;
}

/**
 * @brief Count the number of nodes in the slist
 * @param node The list
 * @return The number of nodes
 */
int slist_count(SList node) {
    int		counter = 0;
    SList	ptr = NULL;

    ptr = node;
    while (ptr) {
	counter++;
	ptr = ptr->next;
    }

    return counter;
}

/**
 * @brief Gets the position of the given element in the 
 *        slist starting from 0
 * @param node The list
 * @param dest The node 
 * @return The position of the node dest
 */
int slist_get_position(SList node, SList dest) {
    int		counter = 0;
    SList	ptr = NULL;

    ptr = node;
    while (ptr) {
	if (ptr == dest)
	    return counter;
	else {
	    ptr = ptr->next;
	    counter++;
	}
    }

    return -1;
}

/**
 * @brief Tells if a slist is empty
 * @param node The list
 * @return 1 if the list is empty, 0 otherwise
 */
int slist_empty(SList node) {
    return node == NULL ? 1 : 0;
}

