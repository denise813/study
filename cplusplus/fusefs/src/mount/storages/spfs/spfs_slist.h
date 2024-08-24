#ifndef SPFS_SLIST_H
#define SPFS_SLIST_H

typedef struct slist {
  char *data;
  int refs;
  struct slist *next;
} slist_t;

// Cons a string to a string list.
slist_t *s_cons(const char *text, slist_t *rest);

// Free the given string list.
void s_free(slist_t *xs);

// Split the given on the given delimiter into a list of strings.
slist_t *s_explode(const char *text, char delim);

#endif
