/* BFD back-end for Intel 386 COFF files.
   Copyright 1990, 1991, 1992 Free Software Foundation, Inc.
   Written by Cygnus Support.

This file is part of BFD, the Binary File Descriptor library.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include "bfd.h"
#include "sysdep.h"
#include "libbfd.h"
#include "obstack.h"
#include "coff/i386.h"
#include "coff/internal.h"
#include "libcoff.h"


static reloc_howto_type howto_table[] = 
{
    {0},
    {1},
    {2},
    {3},
    {4},
    {5},
  HOWTO(R_DIR32,	       0,  2, 	32, false, 0, true,true,0,"dir32",	true, 0xffffffff,0xffffffff, false),
    {7},
    {010},
    {011},
    {012},
    {013},
    {014},
    {015},
    {016},
  HOWTO(R_RELBYTE,	       0,  0,  	8,  false, 0, true,  true,0,"8",	true, 0x000000ff,0x000000ff, false),
  HOWTO(R_RELWORD,	       0,  1, 	16, false, 0, true,  true,0,"16",	true, 0x0000ffff,0x0000ffff, false),
  HOWTO(R_RELLONG,	       0,  2, 	32, false, 0, true,  true,0,"32",	true, 0xffffffff,0xffffffff, false),
  HOWTO(R_PCRBYTE,	       0,  0, 	8,  true,  0, false, true,0,"DISP8",    true, 0x000000ff,0x000000ff, false),
  HOWTO(R_PCRWORD,	       0,  1, 	16, true,  0, false, true,0,"DISP16",   true, 0x0000ffff,0x0000ffff, false),
  HOWTO(R_PCRLONG,	       0,  2, 	32, true,  0, false, true,0,"DISP32",   true, 0xffffffff,0xffffffff, false),
};

/* Turn a howto into a reloc  nunmber */

#define SELECT_RELOC(x,howto) { x = howto->type; }
#define BADMAG(x) I386BADMAG(x)
#define I386 1			/* Customize coffcode.h */

#define RTYPE2HOWTO(cache_ptr, dst) \
	    cache_ptr->howto = howto_table + (dst)->r_type;

/* On SCO Unix 3.2.2 the native assembler generates two .data
   sections.  We handle that by renaming the second one to .data2.  It
   does no harm to do this for any 386 COFF target.  */
#define TWO_DATA_SECS

/* For some reason when using i386 COFF the value stored in the .text
   section for a reference to a .bss or absolute symbol is the value
   itself plus any desired offset.  The linker has already clobbered
   the value stored in the bfd symbol, so we have to dig it out of the
   stored COFF symbol information.  Moreover, we have to dig it out of
   the COFF information for this COFF file, since we might have a
   different common value in different COFF files.  What a mess.
   Ian Taylor, Cygnus Support.  */

static long i386comm_value ();

#define CALC_ADDEND(abfd, ptr, reloc, cache_ptr)		\
  if (ptr							\
      && ((ptr->flags & BSF_OLD_COMMON) != 0			\
	  || ptr->section == &bfd_abs_section))			\
    cache_ptr->addend = - i386comm_value (abfd, ptr);		\
  else if (ptr && bfd_asymbol_bfd(ptr) == abfd			\
	   && ptr->section != (asection *) NULL			\
	   && ((ptr->flags & BSF_OLD_COMMON)== 0))		\
    cache_ptr->addend = - (ptr->section->vma + ptr->value);	\
  else								\
    cache_ptr->addend = 0;					\
  if (ptr && howto_table[reloc.r_type].pc_relative)		\
    cache_ptr->addend += asect->vma;

#include "coffcode.h"

static long
i386comm_value (abfd, ptr)
     bfd *abfd;
     asymbol *ptr;
{
  coff_symbol_type *symbol, *symbol_end;  
  CONST char *name;

  /* If this symbol is from the current COFF file, we can just
     use coff_symbol_from.  */
  if (bfd_asymbol_bfd(ptr) == abfd)
    return coff_symbol_from (abfd, ptr)->native->u.syment.n_value;

  /* Otherwise we have to dig through the symbol information for
     BFD we are dealing with.  */
  name = bfd_asymbol_name (ptr);
  if (! coff_slurp_symbol_table (abfd))
    return 0;
  symbol = obj_symbols (abfd);
  symbol_end = symbol + bfd_get_symcount (abfd);
  for (; symbol < symbol_end; symbol++)
    {
      struct internal_syment *p;

      /* The flags for this symbol will often be UNDEFINED rather than
	 either of the COMMON flags, aand I don't feel like figuring
	 out why.  */
      p = &symbol->native->u.syment;
      if (p->n_sclass == C_EXT
	  && p->n_value != 0
	  && symbol->symbol.name[0] == name[0]
	  && strcmp (symbol->symbol.name, name) == 0)
	return p->n_value;
    }

  return 0;
}

bfd_target *i3coff_object_p(a)
bfd *a ;
{ return coff_object_p(a); }

bfd_target i386coff_vec =
{
  "coff-i386",			/* name */
  bfd_target_coff_flavour,
  false,			/* data byte order is little */
  false,			/* header byte order is little */

  (HAS_RELOC | EXEC_P |		/* object flags */
   HAS_LINENO | HAS_DEBUG |
   HAS_SYMS | HAS_LOCALS | DYNAMIC | WP_TEXT),

  (SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_RELOC), /* section flags */
  0,				/* leading underscore */
  '/',				/* ar_pad_char */
  15,				/* ar_max_namelen */

  2,				/* minimum alignment power */
  _do_getl64, _do_putl64,  _do_getl32, _do_putl32, _do_getl16, _do_putl16, /* data */
  _do_getl64, _do_putl64,  _do_getl32, _do_putl32, _do_getl16, _do_putl16, /* hdrs */

/* Note that we allow an object file to be treated as a core file as well. */
    {_bfd_dummy_target, i3coff_object_p, /* bfd_check_format */
       bfd_generic_archive_p, i3coff_object_p},
    {bfd_false, coff_mkobject, _bfd_generic_mkarchive, /* bfd_set_format */
       bfd_false},
    {bfd_false, coff_write_object_contents, /* bfd_write_contents */
       _bfd_write_archive_contents, bfd_false},

  JUMP_TABLE(coff),
  0, 0,
  COFF_SWAP_TABLE,
};
