
#include "glib.h"
#include "gmacros.h"

#define HASH_TABLE_MIN_SIZE 11
#define HASH_TABLE_MAX_SIZE 13845163

typedef struct _GHashNode      GHashNode;

guint
g_direct_hash (gconstpointer v)
{
    return GPOINTER_TO_UINT (v);
}

struct _GHashNode
{
  gpointer   key;
  gpointer   value;
  GHashNode *next;
};

struct _GHashTable
{
  gint             size;
  gint             nnodes;
  GHashNode      **nodes;
  GHashFunc        hash_func;
  GEqualFunc       key_equal_func;
  volatile guint   ref_count;
  GDestroyNotify   key_destroy_func;
  GDestroyNotify   value_destroy_func;
};

#define G_HASH_TABLE_RESIZE(hash_table)				\
   G_STMT_START {						\
     if ((hash_table->size >= 3 * hash_table->nnodes &&	        \
	  hash_table->size > HASH_TABLE_MIN_SIZE) ||		\
	 (3 * hash_table->size <= hash_table->nnodes &&	        \
	  hash_table->size < HASH_TABLE_MAX_SIZE))		\
	   g_hash_table_resize (hash_table);			\
   } G_STMT_END

static void		g_hash_table_resize	  (GHashTable	  *hash_table);
static GHashNode**	g_hash_table_lookup_node  (GHashTable     *hash_table,
                                                   gconstpointer   key);
static GHashNode*	g_hash_node_new		  (gpointer	   key,
                                                   gpointer        value);
static void		g_hash_node_destroy	  (GHashNode	  *hash_node,
                                                   GDestroyNotify  key_destroy_func,
                                                   GDestroyNotify  value_destroy_func);
static void		g_hash_nodes_destroy	  (GHashNode	  *hash_node,
						  GDestroyNotify   key_destroy_func,
						  GDestroyNotify   value_destroy_func);
static guint g_hash_table_foreach_remove_or_steal (GHashTable     *hash_table,
                                                   GHRFunc	   func,
                                                   gpointer	   user_data,
                                                   gboolean        notify);

GHashTable*
g_hash_table_new (GHashFunc    hash_func,
		  GEqualFunc   key_equal_func)
{
  return g_hash_table_new_full (hash_func, key_equal_func, NULL, NULL);
}

GHashTable*
g_hash_table_new_full (GHashFunc       hash_func,
		       GEqualFunc      key_equal_func,
		       GDestroyNotify  key_destroy_func,
		       GDestroyNotify  value_destroy_func)
{
  GHashTable *hash_table;
  
  hash_table = malloc(sizeof(GHashTable));//g_slice_new (GHashTable);
  hash_table->size               = HASH_TABLE_MIN_SIZE;
  hash_table->nnodes             = 0;
  hash_table->hash_func          = hash_func ? hash_func : g_direct_hash;
  hash_table->key_equal_func     = key_equal_func;
  hash_table->ref_count          = 1;
  hash_table->key_destroy_func   = key_destroy_func;
  hash_table->value_destroy_func = value_destroy_func;
  hash_table->nodes              = calloc(hash_table->size, sizeof(GHashNode*));//g_new0 (GHashNode*, hash_table->size);
  
  return hash_table;
}

GHashTable*
g_hash_table_ref (GHashTable *hash_table)
{

  if (hash_table == NULL) return NULL;
  if (hash_table->ref_count < 1) return hash_table;
  g_atomic_int_add (&hash_table->ref_count, 1);
  return hash_table;
}

void
g_hash_table_unref (GHashTable *hash_table)
{
  if (hash_table == NULL) return;
  if (hash_table->ref_count < 1) return;
  //g_return_if_fail (hash_table != NULL);
  //g_return_if_fail (hash_table->ref_count > 0);

  if (g_atomic_int_exchange_and_add (&hash_table->ref_count, -1) - 1 == 0)
    {
      gint i;

      for (i = 0; i < hash_table->size; i++)
        g_hash_nodes_destroy (hash_table->nodes[i], 
                              hash_table->key_destroy_func,
                              hash_table->value_destroy_func);
      g_free (hash_table->nodes);
      free(hash_table);
      //g_slice_free (GHashTable, hash_table);
    }
}

void
g_hash_table_destroy (GHashTable *hash_table)
{
  if (hash_table == NULL) return;
  if (hash_table->ref_count < 1) return;
  g_hash_table_remove_all (hash_table);
  g_hash_table_unref (hash_table);
}

static inline GHashNode**
g_hash_table_lookup_node (GHashTable	*hash_table,
			  gconstpointer	 key)
{
  GHashNode **node;
  
  node = &hash_table->nodes
    [(* hash_table->hash_func) (key) % hash_table->size];
  
  /* Hash table lookup needs to be fast.
   *  We therefore remove the extra conditional of testing
   *  whether to call the key_equal_func or not from
   *  the inner loop.
   */
  if (hash_table->key_equal_func)
    while (*node && !(*hash_table->key_equal_func) ((*node)->key, key))
      node = &(*node)->next;
  else
    while (*node && (*node)->key != key)
      node = &(*node)->next;
  
  return node;
}

gpointer
g_hash_table_lookup (GHashTable	  *hash_table,
		     gconstpointer key)
{
  GHashNode *node;
  if (hash_table == NULL) return NULL;
  node = *g_hash_table_lookup_node (hash_table, key);
  return node ? node->value : NULL;
}

gboolean
g_hash_table_lookup_extended (GHashTable    *hash_table,
			      gconstpointer  lookup_key,
			      gpointer	    *orig_key,
			      gpointer	    *value)
{
  GHashNode *node;
  if (hash_table == NULL) return FALSE;
  node = *g_hash_table_lookup_node (hash_table, lookup_key);
  if (node)
    {
      if (orig_key)
	*orig_key = node->key;
      if (value)
	*value = node->value;
      return TRUE;
    }
  else
    return FALSE;
}

void
g_hash_table_insert (GHashTable *hash_table,
		     gpointer	 key,
		     gpointer	 value)
{
  GHashNode **node;
  if (hash_table == NULL) return;
  if (hash_table->ref_count < 1) return;
  node = g_hash_table_lookup_node (hash_table, key);
  if (*node)
    {
      /* do not reset node->key in this place, keeping
       * the old key is the intended behaviour. 
       * g_hash_table_replace() can be used instead.
       */

      /* free the passed key */
      if (hash_table->key_destroy_func)
	hash_table->key_destroy_func (key);
      
      if (hash_table->value_destroy_func)
	hash_table->value_destroy_func ((*node)->value);

      (*node)->value = value;
    }
  else
    {
      *node = g_hash_node_new (key, value);
      hash_table->nnodes++;
      G_HASH_TABLE_RESIZE (hash_table);
    }
}

void
g_hash_table_replace (GHashTable *hash_table,
		      gpointer	  key,
		      gpointer	  value)
{
  GHashNode **node;
  if (hash_table == NULL) return;
  if (hash_table->ref_count < 1) return;
  node = g_hash_table_lookup_node (hash_table, key);
  if (*node)
    {
      if (hash_table->key_destroy_func)
	hash_table->key_destroy_func ((*node)->key);
      
      if (hash_table->value_destroy_func)
	hash_table->value_destroy_func ((*node)->value);

      (*node)->key   = key;
      (*node)->value = value;
    }
  else
    {
      *node = g_hash_node_new (key, value);
      hash_table->nnodes++;
      G_HASH_TABLE_RESIZE (hash_table);
    }
}

gboolean
g_hash_table_remove (GHashTable	   *hash_table,
		     gconstpointer  key)
{
  GHashNode **node, *dest;
  if (hash_table == NULL) return FALSE;
  node = g_hash_table_lookup_node (hash_table, key);
  if (*node)
    {
      dest = *node;
      (*node) = dest->next;
      g_hash_node_destroy (dest, 
			   hash_table->key_destroy_func,
			   hash_table->value_destroy_func);
      hash_table->nnodes--;
      G_HASH_TABLE_RESIZE (hash_table);
      return TRUE;
    }

  return FALSE;
}

void
g_hash_table_remove_all (GHashTable *hash_table)
{
  guint i;
  if (hash_table == NULL) return;
  for (i = 0; i < hash_table->size; i++)
    {
      g_hash_nodes_destroy (hash_table->nodes[i],
                            hash_table->key_destroy_func,
                            hash_table->value_destroy_func);
      hash_table->nodes[i] = NULL;
    }
  hash_table->nnodes = 0;
  G_HASH_TABLE_RESIZE (hash_table);
}

gboolean
g_hash_table_steal (GHashTable    *hash_table,
                    gconstpointer  key)
{
  GHashNode **node, *dest;
  if (hash_table == NULL) return FALSE;
  node = g_hash_table_lookup_node (hash_table, key);
  if (*node)
    {
      dest = *node;
      (*node) = dest->next;
      g_hash_node_destroy (dest, NULL, NULL);
      hash_table->nnodes--;
      G_HASH_TABLE_RESIZE (hash_table);
      return TRUE;
    }

  return FALSE;
}

/**
 * g_hash_table_steal_all:
 * @hash_table: a #GHashTable.
 *
 * Removes all keys and their associated values from a #GHashTable 
 * without calling the key and value destroy functions.
 *
 * Since: 2.12
 **/
void
g_hash_table_steal_all (GHashTable *hash_table)
{
  guint i;

  if (hash_table == NULL) return;
  ///g_return_if_fail (hash_table != NULL);

  for (i = 0; i < hash_table->size; i++)
    {
      g_hash_nodes_destroy (hash_table->nodes[i], NULL, NULL);
      hash_table->nodes[i] = NULL;
    }

  hash_table->nnodes = 0;

  G_HASH_TABLE_RESIZE (hash_table);
}

/**
 * g_hash_table_foreach_remove:
 * @hash_table: a #GHashTable.
 * @func: the function to call for each key/value pair.
 * @user_data: user data to pass to the function.
 * 
 * Calls the given function for each key/value pair in the #GHashTable.
 * If the function returns %TRUE, then the key/value pair is removed from the
 * #GHashTable. If you supplied key or value destroy functions when creating
 * the #GHashTable, they are used to free the memory allocated for the removed
 * keys and values.
 * 
 * Return value: the number of key/value pairs removed.
 **/
guint
g_hash_table_foreach_remove (GHashTable	*hash_table,
			     GHRFunc	 func,
			     gpointer	 user_data)
{
 if (hash_table == NULL) return 0;
 if (func == NULL) return 0;
  //g_return_val_if_fail (hash_table != NULL, 0);
  //g_return_val_if_fail (func != NULL, 0);
  
  return g_hash_table_foreach_remove_or_steal (hash_table, func, user_data, TRUE);
}

/**
 * g_hash_table_foreach_steal:
 * @hash_table: a #GHashTable.
 * @func: the function to call for each key/value pair.
 * @user_data: user data to pass to the function.
 * 
 * Calls the given function for each key/value pair in the #GHashTable.
 * If the function returns %TRUE, then the key/value pair is removed from the
 * #GHashTable, but no key or value destroy functions are called.
 * 
 * Return value: the number of key/value pairs removed.
 **/
guint
g_hash_table_foreach_steal (GHashTable *hash_table,
                            GHRFunc	func,
                            gpointer	user_data)
{
  if( hash_table == NULL) return 0;
  if (func == NULL) return 0;
  //g_return_val_if_fail (hash_table != NULL, 0);
  //g_return_val_if_fail (func != NULL, 0);
  
  return g_hash_table_foreach_remove_or_steal (hash_table, func, user_data, FALSE);
}

static guint
g_hash_table_foreach_remove_or_steal (GHashTable *hash_table,
                                      GHRFunc	  func,
                                      gpointer	  user_data,
                                      gboolean    notify)
{
  GHashNode *node, *prev;
  gint i;
  guint deleted = 0;
  
  for (i = 0; i < hash_table->size; i++)
    {
    restart:
      
      prev = NULL;
      
      for (node = hash_table->nodes[i]; node; prev = node, node = node->next)
	{
	  if ((* func) (node->key, node->value, user_data))
	    {
	      deleted += 1;
	      
	      hash_table->nnodes -= 1;
	      
	      if (prev)
		{
		  prev->next = node->next;
		  g_hash_node_destroy (node,
				       notify ? hash_table->key_destroy_func : NULL,
				       notify ? hash_table->value_destroy_func : NULL);
		  node = prev;
		}
	      else
		{
		  hash_table->nodes[i] = node->next;
		  g_hash_node_destroy (node,
				       notify ? hash_table->key_destroy_func : NULL,
				       notify ? hash_table->value_destroy_func : NULL);
		  goto restart;
		}
	    }
	}
    }
  
  G_HASH_TABLE_RESIZE (hash_table);
  
  return deleted;
}

/**
 * g_hash_table_foreach:
 * @hash_table: a #GHashTable.
 * @func: the function to call for each key/value pair.
 * @user_data: user data to pass to the function.
 * 
 * Calls the given function for each of the key/value pairs in the
 * #GHashTable.  The function is passed the key and value of each
 * pair, and the given @user_data parameter.  The hash table may not
 * be modified while iterating over it (you can't add/remove
 * items). To remove all items matching a predicate, use
 * g_hash_table_foreach_remove().
 **/
void
g_hash_table_foreach (GHashTable *hash_table,
		      GHFunc	  func,
		      gpointer	  user_data)
{
  GHashNode *node;
  gint i;
  
  if (hash_table == NULL) return NULL;
  if (func == NULL) return NULL;
  //g_return_if_fail (hash_table != NULL);
  //g_return_if_fail (func != NULL);
  
  for (i = 0; i < hash_table->size; i++)
    for (node = hash_table->nodes[i]; node; node = node->next)
      (* func) (node->key, node->value, user_data);
}

/**
 * g_hash_table_find:
 * @hash_table: a #GHashTable.
 * @predicate:  function to test the key/value pairs for a certain property.
 * @user_data:  user data to pass to the function.
 * 
 * Calls the given function for key/value pairs in the #GHashTable until 
 * @predicate returns %TRUE.  The function is passed the key and value of 
 * each pair, and the given @user_data parameter. The hash table may not
 * be modified while iterating over it (you can't add/remove items). 
 *
 * Return value: The value of the first key/value pair is returned, for which 
 * func evaluates to %TRUE. If no pair with the requested property is found, 
 * %NULL is returned.
 *
 * Since: 2.4
 **/
gpointer
g_hash_table_find (GHashTable	   *hash_table,
                   GHRFunc	    predicate,
                   gpointer	    user_data)
{
  GHashNode *node;
  gint i;
  
  if (hash_table == NULL) return NULL;
  if (predicate == NULL) return NULL;
  //g_return_val_if_fail (hash_table != NULL, NULL);
  //g_return_val_if_fail (predicate != NULL, NULL);
  
  for (i = 0; i < hash_table->size; i++)
    for (node = hash_table->nodes[i]; node; node = node->next)
      if (predicate (node->key, node->value, user_data))
        return node->value;       
  return NULL;
}

/**
 * g_hash_table_size:
 * @hash_table: a #GHashTable.
 * 
 * Returns the number of elements contained in the #GHashTable.
 * 
 * Return value: the number of key/value pairs in the #GHashTable.
 **/
guint
g_hash_table_size (GHashTable *hash_table)
{
  if (hash_table == NULL) return 0;
  //g_return_val_if_fail (hash_table != NULL, 0);
  
  return hash_table->nnodes;
}

static void
g_hash_table_resize (GHashTable *hash_table)
{
  GHashNode **new_nodes;
  GHashNode *node;
  GHashNode *next;
  guint hash_val;
  gint new_size;
  gint i;

  new_size = g_spaced_primes_closest (hash_table->nnodes);
  new_size = CLAMP (new_size, HASH_TABLE_MIN_SIZE, HASH_TABLE_MAX_SIZE);
 
  new_nodes = calloc(new_size, sizeof(GHashNode*));//g_new0 (GHashNode*, new_size);
  
  for (i = 0; i < hash_table->size; i++)
    for (node = hash_table->nodes[i]; node; node = next)
      {
	next = node->next;

	hash_val = (* hash_table->hash_func) (node->key) % new_size;

	node->next = new_nodes[hash_val];
	new_nodes[hash_val] = node;
      }
  
  g_free (hash_table->nodes);
  hash_table->nodes = new_nodes;
  hash_table->size = new_size;
}

static GHashNode*
g_hash_node_new (gpointer key,
		 gpointer value)
{
  GHashNode *hash_node = malloc(sizeof(GHashNode));//g_slice_new (GHashNode);
  
  hash_node->key = key;
  hash_node->value = value;
  hash_node->next = NULL;
  
  return hash_node;
}

static void
g_hash_node_destroy (GHashNode      *hash_node,
		     GDestroyNotify  key_destroy_func,
		     GDestroyNotify  value_destroy_func)
{
  if (key_destroy_func)
    key_destroy_func (hash_node->key);
  if (value_destroy_func)
    value_destroy_func (hash_node->value);
  //g_slice_free (GHashNode, hash_node);
  free(hash_node);
}

static void
g_hash_nodes_destroy (GHashNode *hash_node,
		      GFreeFunc  key_destroy_func,
		      GFreeFunc  value_destroy_func)
{
  while (hash_node)
    {
      GHashNode *next = hash_node->next;
      if (key_destroy_func)
	key_destroy_func (hash_node->key);
      if (value_destroy_func)
	value_destroy_func (hash_node->value);
      //g_slice_free (GHashNode, hash_node);
      free(hash_node);
      hash_node = next;
    }
}

static const guint g_primes[] =
{
  11,
  19,
  37,
  73,
  109,
  163,
  251,
  367,
  557,
  823,
  1237,
  1861,
  2777,
  4177,
  6247,
  9371,
  14057,
  21089,
  31627,
  47431,
  71143,
  106721,
  160073,
  240101,
  360163,
  540217,
  810343,
  1215497,
  1823231,
  2734867,
  4102283,
  6153409,
  9230113,
  13845163,
};

static const guint g_nprimes = sizeof (g_primes) / sizeof (g_primes[0]);

guint
g_spaced_primes_closest (guint num)
{
  gint i;

  for (i = 0; i < g_nprimes; i++)
    if (g_primes[i] > num)
      return g_primes[i];

  return g_primes[g_nprimes - 1];
}

gint
g_atomic_int_exchange_and_add (volatile gint *atomic, 
			       gint           val)
{
  gint result;

  __asm__ __volatile__ ("lock; xaddl %0,%1"
                        : "=r" (result), "=m" (*atomic) 
			: "0" (val), "m" (*atomic));
  return result;
}
 
void
g_atomic_int_add (volatile gint *atomic, 
		  gint           val)
{
  __asm__ __volatile__ ("lock; addl %1,%0"
			: "=m" (*atomic) 
			: "ir" (val), "m" (*atomic));
}
