#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "G3d_intern.h"
#include "cachehash.h"

/*---------------------------------------------------------------------------*/

typedef struct {
  
  char *elts;       /* ptr to array of elts */
  int nofElts;      /* size of "elts" */
  int eltSize;      /* size of elt in "elts" */

  int *names;       /* name[i] is the name of elts[i] */

  char *locks;      /* lock[i] == 1 iff elts[i] is locked
                       lock[i] == 0 iff elts[i] is unlocked but active
                       lock[i] == 2 iff elts[i] doesn't contain valid data */
  int autoLock;     /* 1 if auto locking is turned on */
  int nofUnlocked;  /* nof tiles which are unlocked */
  int minUnlocked;  /* min nof elts which have to remain unlocked. min = 1 */

  int *next, *prev; /* prev/next pointers for fifo */
  int first, last;  /* index (into next) of first and last elt in fifo */
                    /* first == -1 iff fifo is empty */

  int (* eltRemoveFun) (); /* callback activated if the contents of an 
			       elt needs to be removed */
  void *eltRemoveFunData;   /* pointer to user data passed along with 
			       eltRemoveFun */
  int (* eltLoadFun) ();   /* callback activated to load contents of an elt */
  void *eltLoadFunData;     /* pointer to user data passed along with 
			       eltLoadFun */

  void *hash;       /* ptr to hashTable used to relate external names to
                       internal indices (elts) */

} G3D_cache;

/*---------------------------------------------------------------------------*/

#define IS_ACTIVE_ELT(elt) (c->locks[elt] != 2)
#define IS_NOT_ACTIVE_ELT(elt) (c->locks[elt] == 2)
#define IS_LOCKED_ELT(elt) (c->locks[elt] == 1)
#define IS_UNLOCKED_ELT(elt) (c->locks[elt] == 0)
#define IS_NOT_IN_QUEUE_ELT(elt) (IS_LOCKED_ELT (elt))
#define IS_IN_QUEUE_ELT(elt) (! IS_NOT_IN_QUEUE_ELT (elt))

#define DEACTIVATE_ELT(elt) ((IS_LOCKED_ELT(elt) ? \
			      (c->nofUnlocked)++ : (NULL)), \
			     c->locks[elt] = 2)
#define LOCK_ELT(elt) ((IS_LOCKED_ELT(elt) ? \
			(NULL) : (c->nofUnlocked)--), \
		       (c->locks[elt] = 1))
#define UNLOCK_ELT(elt) ((IS_LOCKED_ELT(elt) ? \
			  (c->nofUnlocked)++ : (NULL)), \
                         (c->locks[elt] = 0))

#define ONE_UNLOCKED_ELT_ONLY (c->first == c->last)
#define ARE_MIN_UNLOCKED (c->nofUnlocked <= c->minUnlocked)

/*---------------------------------------------------------------------------*/

void
G3d_cache_reset (c)

     G3D_cache *c;

{
  int i;

  for (i = 0; i < c->nofElts; i++) {
    DEACTIVATE_ELT (i);
    c->next[i] = i + 1;
    c->prev[i] = i - 1;
    c->names[i] = -1;
  }

  c->prev[0] = c->next[c->nofElts - 1] = -1;
  c->first = 0;
  c->last = c->nofElts - 1;

  c->autoLock = 0;
  c->nofUnlocked = c->nofElts;
  c->minUnlocked = 1;

  G3d_cache_hash_reset (c->hash);
}

/*---------------------------------------------------------------------------*/

static int
cache_dummy_fun ()

{
  return 1;
}

/*---------------------------------------------------------------------------*/

void
G3d_cache_dispose (c)

     G3D_cache *c;

{
  if (c == NULL) return;

  G3d_cache_hash_dispose (c->hash);

  if (c->elts != NULL) G3d_free (c->elts);
  if (c->names != NULL) G3d_free (c->names);
  if (c->locks != NULL) G3d_free (c->locks);
  if (c->next != NULL) G3d_free (c->next);
  if (c->prev != NULL) G3d_free (c->prev);
  
  G3d_free (c);
}

/*---------------------------------------------------------------------------*/

void *
G3d_cache_new (nofElts, sizeOfElts, nofNames, eltRemoveFun, eltRemoveFunData,
	      eltLoadFun, eltLoadFunData)

     int nofElts, sizeOfElts, nofNames;
     int (* eltRemoveFun) ();
     void *eltRemoveFunData;
     int (* eltLoadFun) ();
     void *eltLoadFunData;

{
  G3D_cache *tmp;

  tmp = G3d_malloc (sizeof (G3D_cache));
  if (tmp == NULL) {
    G3d_error ("G3d_cache_new: error in G3d_malloc");
    return (void *) NULL;
  }

  tmp->hash = NULL;

  tmp->nofElts = nofElts;
  tmp->eltSize = sizeOfElts;
  tmp->elts = G3d_malloc (tmp->eltSize * tmp->nofElts);
  tmp->names = G3d_malloc (sizeof (int) * tmp->nofElts);
  tmp->locks = G3d_malloc (tmp->nofElts);
  tmp->next = G3d_malloc (sizeof (int) * tmp->nofElts);
  tmp->prev = G3d_malloc (sizeof (int) * tmp->nofElts);

  if ((tmp->elts == NULL) || (tmp->names == NULL) || (tmp->locks == NULL) ||
      (tmp->next == NULL) || (tmp->prev == NULL)) {

    G3d_cache_dispose (tmp);
    G3d_error ("G3d_cache_new: error in G3d_malloc");
    return (void *) NULL;
  }

  tmp->eltRemoveFun = eltRemoveFun;
  tmp->eltRemoveFunData = eltRemoveFunData;
  tmp->eltLoadFun = eltLoadFun;
  tmp->eltLoadFunData = eltLoadFunData;  

  tmp->hash = G3d_cache_hash_new (nofNames);
  if (tmp->hash == NULL) {
    G3d_cache_dispose (tmp);
    G3d_error ("G3d_cache_new: error in G3d_cache_hash_new");
    return (void *) NULL;
  }

  G3d_cache_reset (tmp);

  return tmp;
}

/*---------------------------------------------------------------------------*/

void
G3d_cache_set_removeFun (c, eltRemoveFun, eltRemoveFunData)

     G3D_cache *c;
     int (* eltRemoveFun) ();
     void *eltRemoveFunData;

{
  c->eltRemoveFun = eltRemoveFun;
  c->eltRemoveFunData = eltRemoveFunData;
}

/*---------------------------------------------------------------------------*/

void
G3d_cache_set_loadFun (c, eltLoadFun, eltLoadFunData)

     G3D_cache *c;
     int (* eltLoadFun) ();
     void *eltLoadFunData;

{
  c->eltLoadFun = eltLoadFun;
  c->eltLoadFunData = eltLoadFunData;
}

/*---------------------------------------------------------------------------*/

void *
G3d_cache_new_read (nofElts, sizeOfElts, nofNames, 
		   eltLoadFun, eltLoadFunData)

     int nofElts, sizeOfElts, nofNames;
     int (* eltLoadFun) ();
     void *eltLoadFunData;

{
  return G3d_cache_new (nofElts, sizeOfElts, nofNames,
		       cache_dummy_fun, NULL, 
		       eltLoadFun, eltLoadFunData);
}

/*---------------------------------------------------------------------------*/

static void
cache_queue_dequeue (c, index)

     G3D_cache *c;
     int index;

{
  if (IS_NOT_IN_QUEUE_ELT (index))
    G3d_fatalError ("cache_queue_dequeue: index not in queue");

  if (index == c->first) c->first = c->next[index];
  if (index == c->last) c->last = c->prev[index];

  if (c->next[index] != -1) c->prev[c->next[index]] = c->prev[index];
  if (c->prev[index] != -1) c->next[c->prev[index]] = c->next[index];

  c->next[index] = c->prev[index] = -1;
} 

/*---------------------------------------------------------------------------*/

static void
cache_queue_enqueue (c, left, index)

     G3D_cache *c;
     int left, index;

{
  if (IS_IN_QUEUE_ELT (index))
    G3d_fatalError ("cache_queue_enqueue: index already in queue");
  
  if (c->first == -1) {
    if (left != c->last) 
      G3d_fatalError ("cache_queue_enqueue: position out of range");

    c->first = c->last = index;
    return;
  }

  if (IS_NOT_IN_QUEUE_ELT (left))
    G3d_fatalError ("cache_queue_enqueue: position not in queue");


  if (left == -1) {
    c->next[index] = c->first;
    c->prev[c->first] = index;
    c->first = index;

    return;
  }
    
  c->prev[index] = left;

  if (c->next[left] == -1) {
    c->next[left] = index;
    c->last = index;

    return;
  }

  c->prev[c->next[left]] = index;
  c->next[index] = c->next[left];
  c->next[left] = index;
} 

/*---------------------------------------------------------------------------*/

static int
cache_queue_get_top (c)

     G3D_cache *c;

{
  int top;

  top = c->first;
 
  cache_queue_dequeue (c, c->first);
  
  return top;
}

/*---------------------------------------------------------------------------*/

static void
cache_queue_append (c, index)

     G3D_cache *c;
     int index;

{
  cache_queue_enqueue (c, c->last, index);
}

/*---------------------------------------------------------------------------*/

static void
cache_queue_preppend (c, index)

     G3D_cache *c;
     int index;

{
  cache_queue_enqueue (c, -1, index);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

                        /* EXPORTED FUNCTIONS */

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

int
G3d_cache_lock (c, name)

     G3D_cache *c;
     int name;

{
  int index;

  index = G3d_cache_hash_name2index (c->hash, name);
  if (index == -1) {
    G3d_error ("G3d_cache_lock: name not in cache");
    return 0;
  }

  if (IS_LOCKED_ELT (index)) return 1;
  if (ONE_UNLOCKED_ELT_ONLY) return -1;
  if (ARE_MIN_UNLOCKED) return -1;
  
  cache_queue_dequeue (c, index);
  LOCK_ELT (index);

  return 1;
}

/*---------------------------------------------------------------------------*/

void
G3d_cache_lock_intern (c, index)

     G3D_cache *c;
     int index;

{
  if (IS_LOCKED_ELT (index)) return;

  cache_queue_dequeue (c, index);
  LOCK_ELT (index);
}

/*---------------------------------------------------------------------------*/

int
G3d_cache_unlock (c, name)

     G3D_cache *c;
     int name;

{
  int index;

  index = G3d_cache_hash_name2index (c->hash, name);
  if (index == -1) {
    G3d_error ("G3d_cache_unlock: name not in cache");
    return 0;
  }

  if (IS_UNLOCKED_ELT (index)) return 1;

  cache_queue_append (c, index);
  UNLOCK_ELT (index);

  return 1;
}

/*---------------------------------------------------------------------------*/

int
G3d_cache_unlock_all (c)

     G3D_cache *c;

{
  int index;

  for (index = 0; index < c->nofElts; index++)
    if (IS_LOCKED_ELT (index)) 
      if (! G3d_cache_unlock (c, c->names[index])) {
	G3d_error ("G3d_cache_unlock_all: error in G3d_cache_unlock");
	return 0;
      }
  
  return 1;
}

/*---------------------------------------------------------------------------*/

int
G3d_cache_lock_all (c)

     G3D_cache *c;

{
  int index;

  for (index = 0; index < c->nofElts; index++)
    if (IS_UNLOCKED_ELT (index)) G3d_cache_lock_intern (c, index);

  return 1;
}

/*---------------------------------------------------------------------------*/

void
G3d_cache_autolock_on (c)

     G3D_cache *c;

{
  c->autoLock = 1;
}

/*---------------------------------------------------------------------------*/

void
G3d_cache_autolock_off (c)

     G3D_cache *c;

{
  c->autoLock = 0;
}

/*---------------------------------------------------------------------------*/

void
G3d_cache_set_minUnlock (c, nofMinUnLocked)

     G3D_cache *c;
     int nofMinUnLocked;

{
  c->minUnlocked = nofMinUnLocked;
}

/*---------------------------------------------------------------------------*/

static int
cache_remove_elt (c, name, doFlush)

     G3D_cache *c;
     int name, doFlush;

{
  int index;

  index = G3d_cache_hash_name2index (c->hash, name);
  if (index == -1) {
    G3d_error ("G3d_cache_deactivate_elt : name not in cache");
    return 0;
  }

  if (IS_NOT_ACTIVE_ELT (index)) return 1;

  if (IS_IN_QUEUE_ELT (index)) {
    cache_queue_dequeue (c, index);
    LOCK_ELT (index);
  }

  if (doFlush)
    if (! c->eltRemoveFun (name, c->elts + c->eltSize * index, 
			   c->eltRemoveFunData)) {
      G3d_error ("cache_remove_elt: error in c->eltRemoveFun");
      return 0;
    }

  cache_queue_preppend (c, index);
  DEACTIVATE_ELT (index);

  G3d_cache_hash_remove_name (c->hash, name);

  return 1;
}

/*---------------------------------------------------------------------------*/

int
G3d_cache_remove_elt (c, name)

     G3D_cache *c;
     int name;

{
  if (! cache_remove_elt (c, name, 0)) {
    G3d_error ("G3d_cache_remove_elt: error in cache_remove_elt");
    return 0;
  }

  return 1;
}

/*---------------------------------------------------------------------------*/

int
G3d_cache_flush (c, name)

     G3D_cache *c;
     int name;

{
  if (! cache_remove_elt (c, name, 1)) {
    G3d_error ("G3d_cache_flush: error in cache_remove_elt");
    return 0;
  }

  return 1;
}

/*---------------------------------------------------------------------------*/

int
G3d_cache_remove_all (c)

     G3D_cache *c;

{
  int index;

  for (index = 0; index < c->nofElts; index++)
    if (IS_ACTIVE_ELT (index)) 
      if (! G3d_cache_remove_elt (c, c->names[index])) {
	G3d_error ("G3d_cache_remove_all: error in G3d_cache_remove_elt");
	return 0;
      }

  return 1;
}

/*---------------------------------------------------------------------------*/

int
G3d_cache_flush_all (c)

     G3D_cache *c;

{
  int index;

  for (index = 0; index < c->nofElts; index++)
    if (IS_ACTIVE_ELT (index)) 
      if (! G3d_cache_flush (c, c->names[index])) {
	G3d_error ("G3d_cache_flush_all: error in G3d_cache_flush");
	return 0;
      }

  return 1;
}

/*---------------------------------------------------------------------------*/

char *
G3d_cache_elt_ptr (c, name)

     G3D_cache *c;
     int name;

{
  int index, oldName, doUnlock;

  index = G3d_cache_hash_name2index (c->hash, name);

  if (index != -1) {
    if (c->autoLock)
      if (IS_UNLOCKED_ELT (index) && (! ONE_UNLOCKED_ELT_ONLY) &&
	  (! ARE_MIN_UNLOCKED))
	G3d_cache_lock_intern (c, index);

    return c->elts + c->eltSize * index;
  }

  index = c->first;
  if (IS_ACTIVE_ELT (index)) {
    oldName = c->names[index];
    G3d_cache_hash_remove_name (c->hash, oldName);
    if (! c->eltRemoveFun (oldName, c->elts + c->eltSize * index, 
			   c->eltRemoveFunData)) {
      G3d_error ("G3d_cache_elt_ptr: error in c->eltRemoveFun");
      return (char *) NULL;
    }
  } 

  G3d_cache_hash_load_name (c->hash, name, index);

  doUnlock = ((! c->autoLock) || ONE_UNLOCKED_ELT_ONLY || ARE_MIN_UNLOCKED);

  UNLOCK_ELT (index);
  c->names[index] = name;
  G3d_cache_lock_intern (c, index);

  if (doUnlock) 
    if (! G3d_cache_unlock (c, name)) {
      G3d_error ("G3d_cache_elt_ptr: error in G3d_cache_unlock");
      return (char *) NULL;
    }

  if (! c->eltLoadFun (name, c->elts + c->eltSize * index, 
		       c->eltLoadFunData)) {
    G3d_error("G3d_cache_elt_ptr: error in c->eltLoadFun");
    return (char *) NULL;
  }

  return c->elts + c->eltSize * index;
}
  
/*---------------------------------------------------------------------------*/

int
G3d_cache_load (c, name)

     G3D_cache *c;
     int name;

{
  if (G3d_cache_elt_ptr (c, name) == NULL) {
    G3d_error ("G3d_cache_load: error in G3d_cache_elt_ptr");
    return 0;
  }

  return 1;
}

/*---------------------------------------------------------------------------*/

int
G3d_cache_get_elt (c, name, dst)

     G3D_cache *c;
     int name;
     char *dst;

{
  char *elt, *eltStop;

  elt = G3d_cache_elt_ptr (c, name);
  if (elt == NULL) {
    G3d_error ("G3d_cache_get_elt: error in G3d_cache_elt_ptr");
    return 0;
  }

  eltStop = elt + c->eltSize;
  while (elt != eltStop) *dst++ = *elt++;

  return 1;
}

/*---------------------------------------------------------------------------*/

int
G3d_cache_put_elt (c, name, src)

     G3D_cache *c;
     int name;
     char *src;

{
  char *elt, *eltStop;

  elt = G3d_cache_elt_ptr (c, name);
  if (elt == NULL) {
    G3d_error ("G3d_cache_put_elt: error in G3d_cache_elt_ptr");
    return 0;
  }

  eltStop = elt + c->eltSize;
  while (elt != eltStop) *elt++ = *src++;

  return 1;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

                        /* TEST FUNCTIONS */

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

static void
cache_test_print (c)

    G3D_cache *c;

{
  int i, al;
  int *a;

  al = c->autoLock;
  G3d_cache_autolock_off (c);

  printf ("\n--------------------------------\n");
  for (i = 0; i < c->nofElts; i++) {
    printf ("elt %d: ", i);
    if (IS_NOT_ACTIVE_ELT (i)) {
      printf ("na\n");
      continue;
    }

    a = (int *) G3d_cache_elt_ptr (c, c->names[i]);
    /*G3d_cache_get_elt (c, c->names[i], a);*/
    printf ("name %d val %d %s\n", c->names[i], a[17],
	    (IS_LOCKED_ELT (i) ? "locked" :
	     IS_UNLOCKED_ELT (i) ? "unlocked" : ""));
  }
  printf ("\n--------------------------------\n");

  if (al) G3d_cache_autolock_on (c);
}

/*---------------------------------------------------------------------------*/

static void
cache_test_flush_fun (name, eltPtr, data)

     int name;
     int *eltPtr;
     void *data;

{
  printf ("flushing name %d value %d\n", name, eltPtr[17]);
}

/*---------------------------------------------------------------------------*/

typedef struct {
  
  int *value;
  int size;

} cache_test_data_type;

static void
cache_test_load_fun (name, eltPtr, data)

     int name;
     char *eltPtr;
     void *data;

{
  char *eltStop, *src;

  printf ("loading name %d value %d\n", name, 
	  ((cache_test_data_type *) data)->value[17]);

  src = (char *) ((cache_test_data_type *) data)->value;
  eltStop = eltPtr + ((cache_test_data_type *) data)->size;
  while (eltPtr != eltStop) *eltPtr++ = *src++;
}

/*---------------------------------------------------------------------------*/

static cache_test_data_type ctd;

static void
cache_test_add (c, name, val)

     void *c;
     int name, val;

{
  static int firstTime = 1;

  if (firstTime) {
    ctd.value = G3d_malloc (((G3D_cache *) c)->eltSize * sizeof (int));
    firstTime = 0;
  }

  ctd.value[17] = val;
  ctd.size = ((G3D_cache *) c)->eltSize;

  G3d_cache_load (c, name);
}
  
/*---------------------------------------------------------------------------*/

MAIN ()

{
  void *c;

  c = G3d_cache_new (3, 76 * sizeof (int), 100000, 
		    cache_test_flush_fun, NULL,
		    cache_test_load_fun, &ctd);

  G3d_cache_autolock_on (c);
  cache_test_print (c);
  cache_test_add (c, 1111, -11); cache_test_print (c);
  cache_test_add (c, 2222, -22); cache_test_print (c);
  cache_test_add (c, 3333, -33); cache_test_print (c);
  cache_test_add (c, 4444, -44); cache_test_print (c);
  G3d_cache_unlock_all (c); cache_test_print (c);
  G3d_cache_load (c, 2222); cache_test_print (c);
  cache_test_add (c, 5555, -55); cache_test_print (c);
  cache_test_add (c, 6666, -66); cache_test_print (c);
  cache_test_add (c, 7777, -77); cache_test_print (c);
  cache_test_add (c, 8888, -88); cache_test_print (c);
  cache_test_add (c, 9999, -99); cache_test_print (c);
  G3d_cache_flush (c, 9999); cache_test_print (c);
  G3d_cache_flush_all (c); cache_test_print (c);
  cache_test_add (c, 1111, -11); cache_test_print (c);
  cache_test_add (c, 2222, -22); cache_test_print (c);
  cache_test_add (c, 3333, -33); cache_test_print (c);
  G3d_cache_reset (c); cache_test_print (c);
  cache_test_add (c, 1111, -11); cache_test_print (c);
  cache_test_add (c, 2222, -22); cache_test_print (c);
  cache_test_add (c, 3333, -33); cache_test_print (c);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

