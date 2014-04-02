
#include <zs_core.h>

inline void 
zs_rb_left_rotate(zs_rb_node_t **root, zs_rb_node_t *sentinel,  zs_rb_node_t *x)
{
    zs_rb_node_t  *y;    

    y = x->right;

    x->right = y->left;
    if (y->left != sentinel) {
        y->left->p = x;
    }

    y->p = x->p; 
    if (x == *root) {
        *root = y; 

    } else {
        if (x == x->p->left) {
            x->p->left = y; 

        } else {
            x->p->right = y;
        }
    }

    y->left = x;
    x->p = y;
}

inline void 
zs_rb_right_rotate(zs_rb_node_t **root, zs_rb_node_t *sentinel,  zs_rb_node_t *y)
{
    zs_rb_node_t  *x;    

    x = y->left;
    y->left = x->right;

    if (x->right != sentinel) {
        x->right->p = y;
    }

    x->p = y->p; 

    if (y == *root) {
        *root = x; 

    } else {
        if (y->p->left == y) {
            y->p->left = x; 

        } else {
            y->p->right = x; 
        }
    }

    x->right = y;
    y->p = x;
}

void 
zs_zs_rb_insert_fixup(zs_rb_node_t **root, zs_rb_node_t *sentinel, zs_rb_node_t *z)
{
    zs_rb_node_t *y;

    while (z != *root && z->p->color == RED) {

        if (z->p == z->p->p->left) {
            y = z->p->p->right;

            if (y->color == RED) {
                y->color = BLACK; 
                z->p->color = BLACK;
                z->p->p->color = RED;
                z = z->p->p;

            } else {
                if (z == z->p->right) {
                    z = z->p; 
                    zs_rb_left_rotate(root, sentinel, z);
                }

                z->p->color = BLACK;
                z->p->p->color = RED;
                zs_rb_right_rotate(root, sentinel, z->p->p);
            }
        
        } else {
            y = z->p->p->left;

            if (y->color == RED) {
                y->color = BLACK; 
                z->p->color = BLACK;
                z->p->p->color = RED;
                z = z->p->p;

            } else {
                if (z == z->p->left) {
                    z = z->p; 
                    zs_rb_right_rotate(root, sentinel, z);
                }

                z->p->color = BLACK;
                z->p->p->color = RED;
                zs_rb_left_rotate(root, sentinel, z->p->p);
            }
        }
    }

    (*root)->color = BLACK;
}

void 
zs_rb_insert(zs_rb_node_t **root, zs_rb_node_t *sentinel, zs_rb_node_t *z)
{
    zs_rb_node_t *x, *y;

    y = sentinel;
    x = *root;

    while (x != sentinel) {
        y = x; 
        
        if (z->key->data < x->key->data) {
            x = x->left; 

        } else {
            x = x->right;
        }
    }

    z->p = y;

    if (y == sentinel) {
        *root = z; 

    } else {
        if (z->key->data < y->key->data) {
            y->left = z;
          
        } else {
            y->right = z; 
        }
    }
    
    z->left = sentinel;
    z->right = sentinel;
    z->color = RED;

    zs_zs_rb_insert_fixup(root, sentinel, z);
}

void 
zs_rb_transplant(zs_rb_node_t **root, zs_rb_node_t *sentinel, zs_rb_node_t *u, zs_rb_node_t *v)
{
    if (u->p == sentinel) {
        *root = v; 

    } else if (u == u->p->left) {
        u->p->left = v; 

    } else {
        u->p->right = v; 
    }

    v->p = u->p;
}

zs_rb_node_t *
zs_rbtree_min(zs_rb_node_t *node, zs_rb_node_t *sentinel)
{
    while (node->left != sentinel) {
        node = node->left; 
    }

    return node;
}

void 
zs_zs_rb_delete_fixup(zs_rb_node_t **root, zs_rb_node_t *sentinel, zs_rb_node_t *x)
{
    zs_rb_node_t *w;

    while (x != *root && x->color == BLACK) {
        if (x == x->p->left) {
            w = x->p->right; 

            if (w->color == RED) {
                w->color = BLACK; 
                x->p->color = RED;
                zs_rb_left_rotate(root, sentinel, x->p);
                w = x->p->right;
            }

            if (w->left->color == BLACK && w->right->color == BLACK) {
                w->color = RED; 
                x = x->p;

            } else {
                if (w->right->color == BLACK) {
                    w->left->color = BLACK; 
                    w->color = RED;
                    zs_rb_right_rotate(root, sentinel, w);
                    w = x->p->right;
                }

                w->color = x->p->color;
                x->p->color = BLACK;
                w->right->color = BLACK;
                zs_rb_left_rotate(root, sentinel, x->p);
                x = *root;
            }

        } else {
            w = x->p->left; 

            if (w->color == RED) {
                w->color = BLACK; 
                x->p->color = RED;
                zs_rb_right_rotate(root, sentinel, x->p);
                w = x->p->left;
            }

            if (w->left->color == BLACK && w->right->color == BLACK) {
                w->color = RED; 
                x = x->p;

            } else {
                if (w->left->color == BLACK) {
                    w->right->color = BLACK; 
                    w->color = RED;
                    zs_rb_left_rotate(root, sentinel, w);

                    w = x->p->left;
                }

                w->color = x->p->color;
                x->p->color = BLACK;
                w->left->color = BLACK;
                zs_rb_right_rotate(root, sentinel, x->p);
                x = *root;
            }
        }
    }

    x->color = BLACK;
}

void 
zs_rb_delete(zs_rb_node_t **root, zs_rb_node_t *sentinel, zs_rb_node_t *z)
{
    enum {RED, BLACK} y_old_color;
    zs_rb_node_t *y;
    zs_rb_node_t *x;

    y = z;
    y_old_color = y->color;

    if (z->left == sentinel) {
        x = z->right; 
        zs_rb_transplant(root, sentinel, z, z->right);

    } else if (z->right == sentinel) {
        x = z->left; 
        zs_rb_transplant(root, sentinel, z, z->left);

    } else {
        y = zs_rbtree_min(z->right, sentinel); 
        y_old_color = y->color;
        x = y->right;

        if (y->p == z) {
            x->p = y; 

        } else {
            zs_rb_transplant(root, sentinel, y, y->right); 
            y->right = z->right;
            y->right->p = y;
        }

        zs_rb_transplant(root, sentinel, z, y);
        y->left = z->left;
        y->left->p = y;
        y->color = z->color;
    }

    if (y_old_color == BLACK) {
        zs_zs_rb_delete_fixup(root, sentinel, x); 
    }
}

void
zs_rbtree_traverse(zs_rb_node_t *node, zs_rb_node_t *sentinel)
{
    if (node != sentinel) {
        zs_rbtree_traverse(node->left, sentinel);
        printf("%ld\n", node->key->data);
        zs_rbtree_traverse(node->right, sentinel);
    }
}

zs_rb_node_t *
zs_rbtree_search(zs_rb_node_t *root, zs_rb_node_t *sentinel, zs_rb_node_t *z)
{
    zs_rb_node_t *x;

    x = root;

    while (x != sentinel) {
        if (z->key->data < x->key->data) {
            x = x->left; 

        } else if (z->key->data > x->key->data) {
            x = x->right;

        } else {
            return x;
        }
    }
    
    return NULL;
}

zs_rbtree_t *
zs_create_rbtree(zs_timer_t *t)
{
    zs_rbtree_t *rbt;

    rbt = zs_palloc(t->pool, sizeof(zs_rbtree_t));
    rbt->root = rbt->sentinel = zs_palloc(t->pool, sizeof(zs_rb_node_t));
    rbt->root->left = rbt->sentinel;
    rbt->root->right = rbt->sentinel;
    rbt->root->p = rbt->sentinel;

    return rbt;
}

#if 0
int
main()
{
    zs_rbtree_t *rbt;  
    zs_rb_node_t *root;
    struct timeval t1, t2;

    rbt = malloc(sizeof(zs_rbtree_t));
    rbt->sentinel = malloc(sizeof(zs_rb_node_t));
    rbt->root = rbt->sentinel;
    rbt->root->left = rbt->sentinel;
    rbt->root->right = rbt->sentinel;
    rbt->root->p = rbt->sentinel;

    int i;

    for (i = 1; i <= 5; i++) {
        zs_rb_node_t *node = malloc(sizeof(zs_rb_node_t));
        node->key = i;
        node->left = rbt->sentinel;
        node->right = rbt->sentinel;
        zs_rb_insert(&rbt->root, rbt->sentinel, node); 
    }

    for (i = 100000; i >= 1; i--) {
        zs_rb_node_t *m;
        root = rbt->root;
        m = zs_rbtree_min(&root, rbt->sentinel);
        zs_rb_delete(&rbt->root, rbt->sentinel, m);
    }

    int v = 3;
    zs_rb_node_t *node = malloc(sizeof(zs_rb_node_t));
    node->key = v;

    zs_rbtree_search(rbt->root, rbt->sentinel, node);

    zs_rbtree_traverse(rbt->root, rbt->sentinel);
    
    return 0;
}
#endif

