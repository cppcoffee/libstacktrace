#include <stdlib.h>
#include <stdbool.h>
#include <bfd.h>

#include "symbol_table.h"


static int slurp_symtab(bfd *abfd, asymbol ***out_syms);


int symbol_table_build(char *path, symbol_table_t *table)
{
    bfd      *abfd;
    int       rc = -1;

    bfd_init();

    abfd = bfd_openr(path, NULL);
    if (abfd == NULL) {
        return rc;
    }

    abfd->flags |= BFD_DECOMPRESS;

    if (bfd_check_format(abfd, bfd_archive)) {
        return rc;
    }

    if (!bfd_check_format(abfd, bfd_object)) {
        return rc;
    }

    if (slurp_symtab(abfd, &table->syms) == 0) {
        rc = 0;
        table->abfd = abfd;

    } else {
        bfd_close(abfd);
    }

    return rc;
}


bfd_boolean symbol_table_find(symbol_table_t *table, void *addr, frame_record_t *out)
{
    asection       *sect;
    bfd_vma         pc, sign, vma;
    bfd_size_type   size;
    bfd_boolean     ret = false;
    asymbol       **syms = table->syms;
    bfd            *abfd = table->abfd;

    pc = (bfd_vma)addr;

    if (bfd_get_flavour(abfd) == bfd_target_elf_flavour) {
        sign = (bfd_vma)1 << (bfd_get_arch_size(abfd) - 1);
        pc &= (sign << 1) - 1;

        if (bfd_get_sign_extend_vma(abfd)) {
            pc = (pc ^ sign) - sign;
        }
    }

    for (sect = abfd->sections; sect != NULL; sect = sect->next) {
        if ((bfd_section_flags(sect) & SEC_ALLOC) == 0) {
            continue;
        }

        vma = bfd_section_vma(sect);
        if (pc < vma) {
            continue;
        }

        size = bfd_section_size(sect);
        if (pc >= vma + size) {
            continue;
        }

        ret = bfd_find_nearest_line_discriminator(abfd, sect, syms, pc - vma,
                                                  &out->filename,
                                                  &out->functionname,
                                                  &out->line,
                                                  &out->discriminator);

        if (ret) {
            break;
        }
    }

    return ret;
}


// Read in the symbol table
static int
slurp_symtab(bfd *abfd, asymbol ***out_syms)
{
    asymbol    **res;
    long         storage, symcount;
    bfd_boolean  dynamic = FALSE;

    storage = bfd_get_symtab_upper_bound(abfd);
    if (storage == 0) {
        storage = bfd_get_dynamic_symtab_upper_bound(abfd);
        dynamic = TRUE;
    }

    if (storage < 0) {
        return -1;
    }

    res = (asymbol **) malloc(storage);
    if (dynamic) {
        symcount = bfd_canonicalize_dynamic_symtab(abfd, res);

    } else {
        symcount = bfd_canonicalize_symtab(abfd, res);
    }

    if (symcount < 0) {
        return -1;
    }

    /* If there are no symbols left after canonicalization and
       we have not tried the dynamic symbols then give them a go.  */
    if (symcount == 0
        && !dynamic
        && (storage = bfd_get_dynamic_symtab_upper_bound(abfd)) > 0)
    {
        free(res);
        res = (asymbol **) malloc(storage);
        symcount = bfd_canonicalize_dynamic_symtab(abfd, res);
    }

    /* PR 17512: file: 2a1d3b5b.
       Do not pretend that we have some symbols when we don't.  */
    if (symcount <= 0) {
        free(res);
        return -1;
    }

    *out_syms = res;

    return 0;
}
