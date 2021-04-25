#ifndef __SYMBOL_TABLE_H__
#define __SYMBOL_TABLE_H__

#include <bfd.h>


struct frame_record_s {
    const char *filename;
    const char *functionname;
    unsigned int line;
    unsigned int discriminator;
};
typedef struct frame_record_s   frame_record_t;


struct symbol_table_s {
    bfd      *abfd;
    asymbol **syms;
};
typedef struct symbol_table_s   symbol_table_t;


/*
 * build program symbal table by binary path.
 *
 * On success, malloc asymbol object, return 0.
 * On error, return -1.
 */
int symbol_table_build(char *path, symbol_table_t *table);

/*
 * find symbal table by stack address, output fill frame_record_t.
 *
 * On success, fill frame_record_t fields return true.
 * On error, return false.
 */
bfd_boolean symbol_table_find(symbol_table_t *table, void *addr, frame_record_t *out);


#endif
