#ifndef G_HASH_H
#define G_HASH_H

#include "gtypes.h"

G_BEGIN_DECLS

typedef struct _GHashTable  GHashTable;

typedef gboolean  (*GHRFunc)  (gpointer  key,
                               gpointer  value,
                               gpointer  user_data);

/* Hash tables
 */
GHashTable* g_hash_table_new		   (GHashFunc	    hash_func,
					    GEqualFunc	    key_equal_func);
GHashTable* g_hash_table_new_full      	   (GHashFunc	    hash_func,
					    GEqualFunc	    key_equal_func,
					    GDestroyNotify  key_destroy_func,
					    GDestroyNotify  value_destroy_func);
void	    g_hash_table_destroy	   (GHashTable	   *hash_table);
void	    g_hash_table_insert		   (GHashTable	   *hash_table,
					    gpointer	    key,
					    gpointer	    value);
void        g_hash_table_replace           (GHashTable     *hash_table,
					    gpointer	    key,
					    gpointer	    value);
gboolean    g_hash_table_remove		   (GHashTable	   *hash_table,
					    gconstpointer   key);
void        g_hash_table_remove_all        (GHashTable     *hash_table);
gboolean    g_hash_table_steal             (GHashTable     *hash_table,
					    gconstpointer   key);
void        g_hash_table_steal_all         (GHashTable     *hash_table);
gpointer    g_hash_table_lookup		   (GHashTable	   *hash_table,
					    gconstpointer   key);
gboolean    g_hash_table_lookup_extended   (GHashTable	   *hash_table,
					    gconstpointer   lookup_key,
					    gpointer	   *orig_key,
					    gpointer	   *value);
void	    g_hash_table_foreach	   (GHashTable	   *hash_table,
					    GHFunc	    func,
					    gpointer	    user_data);
gpointer    g_hash_table_find	           (GHashTable	   *hash_table,
					    GHRFunc	    predicate,
					    gpointer	    user_data);
guint	    g_hash_table_foreach_remove	   (GHashTable	   *hash_table,
					    GHRFunc	    func,
					    gpointer	    user_data);
guint	    g_hash_table_foreach_steal	   (GHashTable	   *hash_table,
					    GHRFunc	    func,
					    gpointer	    user_data);
guint	    g_hash_table_size		   (GHashTable	   *hash_table);

/* keeping hash tables alive */
GHashTable* g_hash_table_ref   		   (GHashTable 	   *hash_table);
void        g_hash_table_unref             (GHashTable     *hash_table);

#ifndef G_DISABLE_DEPRECATED

/* The following two functions are deprecated and will be removed in
 * the next major release. They do no good. */
#define g_hash_table_freeze(hash_table) ((void)0)
#define g_hash_table_thaw(hash_table) ((void)0)

#endif /* G_DISABLE_DEPRECATED */

/* Hash Functions
 */
gboolean g_str_equal (gconstpointer  v1,
                      gconstpointer  v2);
guint    g_str_hash  (gconstpointer  v);

gboolean g_int_equal (gconstpointer  v1,
                      gconstpointer  v2);
guint    g_int_hash  (gconstpointer  v);

/* This "hash" function will just return the key's address as an
 * unsigned integer. Useful for hashing on plain addresses or
 * simple integer values.
 * Passing NULL into g_hash_table_new() as GHashFunc has the
 * same effect as passing g_direct_hash().
 */
guint    g_direct_hash  (gconstpointer  v) G_GNUC_CONST;
gboolean g_direct_equal (gconstpointer  v1,
                         gconstpointer  v2) G_GNUC_CONST;

G_END_DECLS

#endif /* __G_HASH_H__ */

