#include "linkedlist.h"
#include <stdlib.h>

//  X3 < X1 <-> X2 <-> X3 > X1

typedef struct ListNode
{
    void             *data;
    struct ListNode  *next;
    struct ListNode  *prev;

} ListNode;

typedef struct List
{
    ListNode *head;
    int length;

} List;

List* list_open()
{
    List* list = (List*)malloc(sizeof(ListNode));

    if (list == NULL) return NULL;

    list->head = 0;
    list->length = 0;

    return list;
}

int list_size(List *handle)
{
    if (handle == 0) return 0;
    return handle->length;
}

// 0 1 2 3 4
// length = 5
ListNode* _list_at(List* handle, int index)
{
    if (handle == 0) return 0;

    int listSize = handle->length;
    if (listSize == 0) return 0;
	while (index < 0) index += listSize;

    ListNode *node = handle->head;

    int iter = 0;
    while (index != iter++)
    {
        node = node->next;
    }

    return node;
}


int list_append(List* handle, void *data)
{
    if (handle == NULL) return 0;

    ListNode* newNode = (ListNode*)malloc(sizeof(ListNode));
    if (newNode == NULL) return 0;

    ListNode *lastNode = _list_at(handle, -1);

    if (lastNode == NULL)
    {
        handle->head = newNode;
        lastNode = newNode;
    }

    newNode->data = data;
    newNode->next = handle->head;
    newNode->prev = lastNode;

    lastNode->next = newNode;
    handle->head->prev = newNode;

    handle->length++;

    return 1;
}


void* list_get(List* handle, int index)
{
    ListNode *node = _list_at(handle, index);
    if (node == 0) return 0;

    return node->data;
}

void *list_remove(List* handle, int index)
{
    ListNode *node = _list_at(handle, index);
    if (node == 0) return 0;

    ListNode *prev = node->prev;
    ListNode *next = node->next;

    prev->next = next;
    next->prev = prev;

    if (index == 0)
    {
        handle->head = next;
    }

    void *data = node->data;
    free(node);
    handle->length--;

    return data;
}

// removes node if callback returns a negative value
// returns node data and exits the loop if callback returns a positive value
void* list_loop(List* handle, CallBackFn callbackFn, void *callbackArg)
{
    if (handle == NULL) return 0;

    ListNode *node = handle->head;
    int length = handle->length;
    for(int i = 0; i < length; i++)
    {
        int result = callbackFn(node->data, callbackArg);
        node = node->next;

        if (result < 0)
        {
            list_remove(handle, i--);
            length--;
        }
        else if (result > 0)
        {
            return node->data;
        }
    }

    return 0;
}
