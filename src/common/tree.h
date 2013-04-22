#ifndef XCHAT_TREE_H
#define XCHAT_TREE_H

typedef struct _tree tree;

typedef int (tree_cmp_func) (const void *keya, const void *keyb, void *data);
typedef int (tree_traverse_func) (const void *key, void *data);

tree *tree_new (tree_cmp_func *cmp, void *data);
void tree_destroy (tree *t);
void *tree_find (tree *t, void *key, tree_cmp_func *cmp, void *data, long *pos);
int tree_remove (tree *t, void *key, long *pos);
void tree_remove_at_pos (tree *t, long pos);
void tree_foreach (tree *t, tree_traverse_func *func, void *data);
long tree_insert (tree *t, void *key);
void tree_append (tree* t, void *key);
size_t tree_size (tree *t);

#endif
