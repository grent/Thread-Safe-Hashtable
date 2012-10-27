#include "list.h"
#include <stdio.h>
#include <string.h>

/* ************************************** 
 * init the list by making it's head NULL
 * ************************************** */
void list_init(list_t *list) 
{
        list->head = NULL;
}


/* ************************************** 
 * print the contents of the list to file f.
 * ************************************** */
void list_print(list_t *list) 
{
        struct __list_node *tmp = list->head;
        while (tmp) //continue while the traversal node is not NULL
	{
                printf("%s\n", tmp->data);
                tmp = tmp->next;
        }
}


/* ************************************** 
 * add item "val" to the list, in order.
 * ************************************** */
void list_add(list_t *list, char *str) 
{
	struct __list_node *new_node = (struct __list_node *) malloc(sizeof(struct __list_node));
   	if (!new_node) //shortcut: if for some reason you have no space for new_node, abort
	{
                	fprintf(stderr, "No memory while attempting to create a new list node!\n");
                	abort();
	}
	new_node->data = str;
	new_node->next = NULL;
	struct __list_node *tmp = list->head; //tmp will be used to traverse the list
 	int added = 0;

	/* special case: list is currently empty */
	if (list->head == NULL) 
	{
                	list->head = new_node;
	}

	//node belongs before node currently at head//
	else if (strcmp(str,list->head->data) <= 0) 
	{
		new_node->next = tmp;
		list->head = new_node;
	} 

	//node belongs after the head//
	else 
	{
		while (tmp->next) //traverse list by checking that the current node has a next node and then changing to that next node
		{

			//node belongs between current node and next node//
			if (strcmp(str,tmp->data) >= 0 && strcmp(str,tmp->next->data) <= 0)
			{
				new_node->next = tmp->next;
				tmp->next = new_node;
				added = 1;
				break;
			}
			tmp = tmp->next; 
                	}

		//node belongs at the tail//
		if (added == 0) 
		{
			tmp->next = new_node;
           	}
        }
}


/* ************************************** 
 * remove all items equal to "target" from 
 * the list. return the number of items
 * removed.
 * ************************************** */
int list_remove(list_t *list, char *target) 
{
        int removed = 0;

        /* short cut: is the list empty? */
        if (list->head == NULL)
	{
                return removed;
	}

        /* check for removing items at the head */
        struct __list_node *dead = NULL;
        struct __list_node *tmp = list->head;
        while (tmp != NULL && strcmp(tmp->data,target) == 0) 
	{
                dead = tmp;
                tmp = tmp->next;
                free(dead);
                removed += 1;
        }
        list->head = tmp;

        /* if we removed anything or the data at the head is greater than
        the target, we're done (since the list is sorted) */
        if (removed > 0 || strcmp(target,tmp->data) < 0) 
	{
                return removed;
        }

        /* find the target to destroy (if it exists). 
        keep track of previous node using dead.  */
        dead = tmp;
        while (dead != NULL && strcmp(dead->data,target)!= 0)
	{
                tmp = dead;
                dead = dead->next;
        }

        /* if there's something to destroy... */
        if (dead != NULL) 
	{ 

                // dead is node we want to remove, tmp is node previous to dead
                while (dead != NULL && strcmp(dead->data,target) == 0) 
		{
                        tmp->next = dead->next;
                        free(dead);
                        dead = tmp->next;        
                        removed += 1;
                }
        }
        return removed;
}


/* ************************************** 
 * clear out the entire list, freeing all
 * elements.
 * ************************************** */
void list_clear(list_t *list) 
{
        struct __list_node *tmp = list->head;
	// continue while traversal node is not NULL
        while (tmp)
	{
                struct __list_node *tmp2 = tmp->next;
		free(tmp->data);
                free(tmp);
                tmp = tmp2;
        }
	free(list);
}
