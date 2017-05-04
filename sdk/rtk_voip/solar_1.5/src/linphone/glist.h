/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GLib Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/. 
 */

#ifndef __G_LIST_H__
#define __G_LIST_H__


#undef	MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

#undef	MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

typedef struct _GList
{
  void * data;
  struct _GList *next;
  struct _GList *prev;
} GList ;

typedef const void *gconstpointer;

typedef gint            (*GCompareFunc)         (gconstpointer  a,
                                                 gconstpointer  b);
typedef gint            (*GCompareDataFunc)     (gconstpointer  a,
                                                 gconstpointer  b,
						 gpointer       user_data);
typedef void            (*GFunc)                (gpointer       data,
                                                 gpointer       user_data);

#ifdef __cplusplus
extern "C" {
#endif
                                                 
/* Doubly linked lists
 */
/*
  void     g_list_push_allocator (GAllocator       *allocator);
  void     g_list_pop_allocator  (void);
*/
#ifdef MALLOC_DEBUG

GList*   __g_list_alloc          (char *file, int line);
void     __g_list_free           (GList            *list, char *file, int line);
void     __g_list_free_1         (GList            *list, char *file, int line);
GList*   __g_list_append         (GList            *list,
				void *          data, char *file, int line);
GList*   __g_list_prepend        (GList            *list,
				void *          data, char *file, int line);
GList*   __g_list_insert         (GList            *list,
				void *          data,
				gint              position, char *file, int line);
GList*   __g_list_insert_before  (GList            *list,
				GList            *sibling,
				void *          data, char *file, int line);
GList*   __g_list_remove         (GList            *list,
				gconstpointer     data, char *file, int line);
GList*   __g_list_remove_all     (GList            *list,
				gconstpointer     data, char *file, int line);
GList*   __g_list_delete_link    (GList            *list,
				GList            *link_, char *file, int line);
GList*   __g_list_copy           (GList            *list, char *file, int line);

#define g_list_alloc()				__g_list_alloc(__FILE__, __LINE__)
#define g_list_free(l)				__g_list_free(l, __FILE__, __LINE__)
#define g_list_free_1(l)        	__g_list_free_1(l, __FILE__, __LINE__)
#define g_list_append(l,d)			__g_list_append(l,d, __FILE__, __LINE__)
#define g_list_prepend(l,d)			__g_list_prepend(l,d, __FILE__, __LINE__)
#define g_list_insert(l,d,p)		__g_list_insert(l,d,p, __FILE__, __LINE__)
GList*   g_list_insert_sorted  (GList            *list,
				void *          data,
				GCompareFunc      func);
#define g_list_insert_before(l,s,d)	__g_list_insert_before(l,s,d, __FILE__, __LINE__)
GList*   g_list_concat         (GList            *list1,
				GList            *list2);
#define g_list_remove(l,d)			__g_list_remove(l,d, __FILE__, __LINE__)
#define g_list_remove_all(l,d)		__g_list_remove_all(l,d,__FILE__, __LINE__)
GList*   g_list_remove_link    (GList            *list,
				GList            *llink);
#define g_list_delete_link(l,lk)	__g_list_delete_link(l,lk, __FILE__, __LINE__)
GList*   g_list_reverse        (GList            *list);
#define g_list_copy(l)         		__g_list_copy(l, __FILE__, __LINE__)
GList*   g_list_nth            (GList            *list,
				guint             n);
GList*   g_list_nth_prev       (GList            *list,
				guint             n);
GList*   g_list_find           (GList            *list,
				gconstpointer     data);
GList*   g_list_find_custom    (GList            *list,
				gconstpointer     data,
				GCompareFunc      func);
gint     g_list_position       (GList            *list,
				GList            *llink);
gint     g_list_index          (GList            *list,
				gconstpointer     data);
GList*   g_list_last           (GList            *list);
GList*   g_list_first          (GList            *list);
guint    g_list_length         (GList            *list);
void     g_list_foreach        (GList            *list,
				GFunc             func,
				void *          user_data);
GList*   g_list_sort           (GList            *list,
				GCompareFunc      compare_func);
GList*   g_list_sort_with_data (GList            *list,
				GCompareDataFunc  compare_func,
				void *          user_data);
void    *g_list_nth_data       (GList            *list,
				guint             n);

#else // MALLOC_DEBUG

GList*   g_list_alloc          (void);
void     g_list_free           (GList            *list);
void     g_list_free_1         (GList            *list);
GList*   g_list_append         (GList            *list,
				void *          data);
GList*   g_list_prepend        (GList            *list,
				void *          data);
GList*   g_list_insert         (GList            *list,
				void *          data,
				gint              position);
GList*   g_list_insert_sorted  (GList            *list,
				void *          data,
				GCompareFunc      func);
GList*   g_list_insert_before  (GList            *list,
				GList            *sibling,
				void *          data);
GList*   g_list_concat         (GList            *list1,
				GList            *list2);
GList*   g_list_remove         (GList            *list,
				gconstpointer     data);
GList*   g_list_remove_all     (GList            *list,
				gconstpointer     data);
GList*   g_list_remove_link    (GList            *list,
				GList            *llink);
GList*   g_list_delete_link    (GList            *list,
				GList            *link_);
GList*   g_list_reverse        (GList            *list);
GList*   g_list_copy           (GList            *list);
GList*   g_list_nth            (GList            *list,
				guint             n);
GList*   g_list_nth_prev       (GList            *list,
				guint             n);
GList*   g_list_find           (GList            *list,
				gconstpointer     data);
GList*   g_list_find_custom    (GList            *list,
				gconstpointer     data,
				GCompareFunc      func);
gint     g_list_position       (GList            *list,
				GList            *llink);
gint     g_list_index          (GList            *list,
				gconstpointer     data);
GList*   g_list_last           (GList            *list);
GList*   g_list_first          (GList            *list);
guint    g_list_length         (GList            *list);
void     g_list_foreach        (GList            *list,
				GFunc             func,
				void *          user_data);
GList*   g_list_sort           (GList            *list,
				GCompareFunc      compare_func);
GList*   g_list_sort_with_data (GList            *list,
				GCompareDataFunc  compare_func,
				void *          user_data);
void    *g_list_nth_data       (GList            *list,
				guint             n);
#endif

#ifdef __cplusplus
}
#endif

#define g_list_previous(list)	((list) ? (((GList *)(list))->prev) : NULL)
#define g_list_next(list)	((list) ? (((GList *)(list))->next) : NULL)


typedef GList GSList;
#define g_slist_prepend g_list_prepend
#define g_slist_free g_list_free
#define g_slist_free_1  g_list_free_1
#define g_slist_reverse g_list_reverse
#define g_slist_next g_list_next
#define g_slist_foreach g_list_foreach
#define g_slist_length g_list_length

/* to be continued...*/

#endif /* __G_LIST_H__ */
