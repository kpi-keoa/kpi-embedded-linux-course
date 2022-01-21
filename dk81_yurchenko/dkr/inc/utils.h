#ifndef _INC_UTILS_H
#define _INC_UTILS_H

#include <stddef.h>

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})
//#define address_struct(addr_node, type, node) addr_node -  &((type *)0)->node;

enum STATE {
	STATE_OK = 0,
	STATE_EOP = -1,		// error open file
	STATE_EFA = -2,		// Failed to allocate
	STATE_EFC = -3,		// not find Chatacter
	STATE_EEM = -4		// not found
};

struct list_t { struct list_t *prev, *next; };

struct database {
	char *name;
	char *suname;
	char *patronymic;
	int salary;
	struct list_t list;
};

#define LIST_T_INIT(name) { &(name), &(name) }

#define list_for_each(head, list) \
	for (head = list->next; head != list; head = head->next)

#define list_for_each_safe(head, t, list) \
	for (head = (list)->next, t = head->next; head != list; \
			head = t, t = head->next)

static inline int list_empty(struct list_t *list)
{
	if (list == list->next && list == list->prev)
		return 1;
	return 0;
}

static inline void __list_add(struct list_t *new,
			    struct list_t *prev,
			    struct list_t *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

static inline void list_add(struct list_t *head, struct list_t *new_node)
{
	__list_add(new_node, head, head->next);
}

static inline void list_add_tail(struct list_t *new_node, struct list_t *head)
{
	if (list_empty(head)) {
		head->next = new_node;
		head->prev = new_node;
		new_node->next = head;
		new_node->prev = head;
	} else
		__list_add(new_node, head->prev, head);
}

static inline void list_del(struct list_t *del)
{
	if (list_empty(del))
		return;

	del->next->prev = del->prev;
	del->prev->next = del->next;
	del->prev = NULL;
	del->next = NULL;
}

char *char_to_character(char *addr_begin, int characters, char **string);
enum STATE cp(char *string1, char **string2, size_t size);
enum STATE line_to_struct(char *line, struct database *new_node);
void free_database(struct database *str);
void print_error(enum STATE state, int verbose);

#endif

