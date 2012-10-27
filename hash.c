#include "hash.h"
#include "list.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>


//hashfunction: adds each letter of the input together as integers and mods the resulting integer by the size of the hashtable
int hasher(const char *str, int size) 
{
	int sum = 0;
	int i = 0;
	for (;i<strlen(str);i++) //iterate through string, adding up letters
	{
		sum+=str[i];
	}
	int index = sum%size;
	return index;
}

// create a new hashtable; parameter is a size hint
hashtable_t *hashtable_new(int sizehint)
{
	struct hashtable_t *newhash = (struct hashtable_t*)malloc(sizeof(struct hashtable_t));
	if (sizehint<1) // shortcut: if sizehint is smaller than 1, we will not bother making a table since it would be of size 0
	{
		return NULL;
	}
	else 
	{
		// store unique list size and mutex in the hashtable
		newhash->size = sizehint;
		pthread_mutex_t newmutex;
		pthread_mutex_init(&newmutex, NULL);
		newhash->hashlock = newmutex;

		// malloc space for the table full of pointers and their linked lists
		newhash->table = malloc(sizehint*sizeof(list_t*));
		int i = 0;
		for (;i<sizehint;i++)
		{
			list_t *newlist = malloc(sizeof(list_t*));
			list_init(newlist);
			newhash->table[i] = newlist;
		}
	}
        return newhash;
}

// free anything allocated by the hashtable library
void hashtable_free(hashtable_t *hashtable) 
{
	int i = 0;
	pthread_mutex_lock(&hashtable->hashlock);//lock
	for (;i<hashtable->size;i++)
	{
		list_clear(hashtable->table[i]);//list_clear frees all nodes in the given list
	}
	//include this (and comment the free command below) if you don't want to effectively destroy the hashtable -- hashtable = hashtablenew(hashtable->size);
	free(hashtable->table);
	pthread_mutex_unlock(&hashtable->hashlock);//unlock
	free(hashtable);
}

// add a new string to the hashtable
void hashtable_add(hashtable_t *hashtable, const char *s) 
{
	int index = hasher(s, hashtable->size);
	pthread_mutex_lock(&hashtable->hashlock);//lock
	list_add(hashtable->table[index],s);
	pthread_mutex_unlock(&hashtable->hashlock);//unlock
}

// remove a string from the hashtable; if the string
// doesn't exist in the hashtable, do nothing
void hashtable_remove(hashtable_t *hashtable, const char *s) 
{
	//assuming that if there are multiple copies of the string s in the hashtable, then we want to remove every copy
	int index = hasher(s, hashtable->size);
	pthread_mutex_lock(&hashtable->hashlock);//lock
	int q = list_remove(hashtable->table[index],s);
	pthread_mutex_unlock(&hashtable->hashlock);//unlock
	if (q == 0)
	{
		printf("nothing was removed\n");
	}
}

// print the contents of the hashtable
void hashtable_print(hashtable_t *hashtable) 
{
	int i = 0;
	//pthread_mutex_lock(&hashtable->hashlock);//lock
	for (;i<hashtable->size;i++)
	{
		list_print(hashtable->table[i]);
	}
	//pthread_mutex_unlock(&hashtable->hashlock);//unlock
}
