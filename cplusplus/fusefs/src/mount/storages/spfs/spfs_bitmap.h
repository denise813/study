#ifndef SPFS_BITMAP_H
#define SPFS_BITMAP_H

// Get the given bit from the bitmap.
int bitmap_get(void *bm, int i);

// Set the given bit in the bitmap to the given value.
// Value should be 0 or 1.
void bitmap_put(void *bm, int i, int v);

// Pretty-print the bitmap (with the given no. of bits).
void bitmap_print(void *bm, int size);

#endif
