/* copyout.c - create a cpio archive
   Copyright (C) 1990, 1991, 1992 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "filetypes.h"
#include "system.h"
#include "cpiohdr.h"
#include "dstring.h"
#include "extern.h"
#include "rmt.h"

static unsigned long read_for_checksum ();
static void clear_rest_of_block ();
static void pad_output ();

/* Write out header FILE_HDR, including the file name, to file
   descriptor OUT_DES.  */

void
write_out_header (file_hdr, out_des)
     struct new_cpio_header *file_hdr;
     int out_des;
{
  if (archive_format == arf_newascii || archive_format == arf_crcascii)
    {
      char ascii_header[112];
      char *magic_string;

      if (archive_format == arf_crcascii)
	magic_string = "070702";
      else
	magic_string = "070701";
      sprintf (ascii_header,
	       "%6s%08lx%08lx%08lx%08lx%08lx%08lx%08lx%08lx%08lx%08lx%08lx%08lx%08lx",
	       magic_string,
	       file_hdr->c_ino, file_hdr->c_mode, file_hdr->c_uid,
	       file_hdr->c_gid, file_hdr->c_nlink, file_hdr->c_mtime,
	     file_hdr->c_filesize, file_hdr->c_dev_maj, file_hdr->c_dev_min,
	   file_hdr->c_rdev_maj, file_hdr->c_rdev_min, file_hdr->c_namesize,
	       file_hdr->c_chksum);
      copy_buf_out (ascii_header, out_des, 110L);

      /* Write file name to output.  */
      copy_buf_out (file_hdr->c_name, out_des, (long) file_hdr->c_namesize);
      pad_output (out_des, file_hdr->c_namesize + 110);
    }
  else if (archive_format == arf_oldascii)
    {
      char ascii_header[78];
#ifndef __MSDOS__
      dev_t dev;
      dev_t rdev;

      dev = makedev (file_hdr->c_dev_maj, file_hdr->c_dev_min);
      rdev = makedev (file_hdr->c_rdev_maj, file_hdr->c_rdev_min);
#else
      int dev = 0, rdev = 0;
#endif

      if ((file_hdr->c_ino >> 16) != 0)
	error (0, 0, "%s: truncating inode number", file_hdr->c_name);

      sprintf (ascii_header,
	       "%06lo%06lo%06lo%06lo%06lo%06lo%06lo%06lo%011lo%06lo%011lo",
	       file_hdr->c_magic & 0xFFFF, dev & 0xFFFF,
	       file_hdr->c_ino & 0xFFFF, file_hdr->c_mode & 0xFFFF,
	       file_hdr->c_uid & 0xFFFF, file_hdr->c_gid & 0xFFFF,
	       file_hdr->c_nlink & 0xFFFF, rdev & 0xFFFF,
	       file_hdr->c_mtime, file_hdr->c_namesize & 0xFFFF,
	       file_hdr->c_filesize);
      copy_buf_out (ascii_header, out_des, 76L);

      /* Write file name to output.  */
      copy_buf_out (file_hdr->c_name, out_des, (long) file_hdr->c_namesize);
    }
  else if (archive_format == arf_tar || archive_format == arf_ustar)
    {
      write_out_tar_header (file_hdr, out_des);
    }
  else
    {
      struct old_cpio_header short_hdr;

      short_hdr.c_magic = 070707;
      short_hdr.c_dev = makedev (file_hdr->c_dev_maj, file_hdr->c_dev_min);

      if ((file_hdr->c_ino >> 16) != 0)
	error (0, 0, "%s: truncating inode number", file_hdr->c_name);

      short_hdr.c_ino = file_hdr->c_ino & 0xFFFF;
      short_hdr.c_mode = file_hdr->c_mode & 0xFFFF;
      short_hdr.c_uid = file_hdr->c_uid & 0xFFFF;
      short_hdr.c_gid = file_hdr->c_gid & 0xFFFF;
      short_hdr.c_nlink = file_hdr->c_nlink & 0xFFFF;
      short_hdr.c_rdev = makedev (file_hdr->c_rdev_maj, file_hdr->c_rdev_min);
      short_hdr.c_mtimes[0] = file_hdr->c_mtime >> 16;
      short_hdr.c_mtimes[1] = file_hdr->c_mtime & 0xFFFF;

      short_hdr.c_namesize = file_hdr->c_namesize & 0xFFFF;

      short_hdr.c_filesizes[0] = file_hdr->c_filesize >> 16;
      short_hdr.c_filesizes[1] = file_hdr->c_filesize & 0xFFFF;

      /* Output the file header.  */
      copy_buf_out ((char *) &short_hdr, out_des, 26L);

      /* Write file name to output.  */
      copy_buf_out (file_hdr->c_name, out_des, (long) file_hdr->c_namesize);

      pad_output (out_des, file_hdr->c_namesize + 26);
    }
}

/* Read a list of file names from the standard input
   and write a cpio collection on the standard output.
   The format of the header depends on the compatibility (-c) flag.  */

void
process_copy_out ()
{
  int res;			/* Result of functions.  */
  dynamic_string input_name;	/* Name of file read from stdin.  */
  struct utimbuf times;		/* For resetting file times after copy.  */
  struct stat file_stat;	/* Stat record for file.  */
  struct new_cpio_header file_hdr; /* Output header information.  */
  int in_file_des;		/* Source file descriptor.  */
  int out_file_des;		/* Output file descriptor.  */
  char *p;

  /* Initialize the copy out.  */
  ds_init (&input_name, 128);
  /* Initialize this in case it has members we don't know to set.  */
  bzero (&times, sizeof (struct utimbuf));
  file_hdr.c_magic = 070707;

#ifdef __MSDOS__
  setmode (archive_des, O_BINARY);
#endif
  /* Check whether the output file might be a tape.  */
  out_file_des = archive_des;
  if (_isrmt (out_file_des))
    {
      output_is_special = 1;
      output_is_seekable = 0;
    }
  else
    {
      if (fstat (out_file_des, &file_stat))
	error (1, errno, "standard output is closed");
      output_is_special =
#ifdef S_ISBLK
	S_ISBLK (file_stat.st_mode) ||
#endif
	S_ISCHR (file_stat.st_mode);
      output_is_seekable = S_ISREG (file_stat.st_mode);
    }

  if (append_flag)
    {
      process_copy_in ();
      prepare_append (out_file_des);
    }

  /* Copy files with names read from stdin.  */
  while (ds_fgetstr (stdin, &input_name, name_end) != NULL)
    {
      /* Check for blank line.  */
      if (input_name.ds_string[0] == 0)
	{
	  error (0, 0, "blank line ignored");
	  continue;
	}

      /* Process next file.  */
      if ((*xstat) (input_name.ds_string, &file_stat) < 0)
	error (0, errno, "%s", input_name.ds_string);
      else
	{
	  /* Set values in output header.  */
	  file_hdr.c_dev_maj = major (file_stat.st_dev);
	  file_hdr.c_dev_min = minor (file_stat.st_dev);
	  file_hdr.c_ino = file_stat.st_ino;
	  /* For POSIX systems that don't define the S_IF macros,
	     we can't assume that S_ISfoo means the standard Unix
	     S_IFfoo bit(s) are set.  So do it manually, with a
	     different name.  Bleah.  */
	  file_hdr.c_mode = (file_stat.st_mode & 07777);
	  if (S_ISREG (file_stat.st_mode))
	    file_hdr.c_mode |= CP_IFREG;
	  else if (S_ISDIR (file_stat.st_mode))
	    file_hdr.c_mode |= CP_IFDIR;
#ifdef S_ISBLK
	  else if (S_ISBLK (file_stat.st_mode))
	    file_hdr.c_mode |= CP_IFBLK;
#endif
#ifdef S_ISCHR
	  else if (S_ISCHR (file_stat.st_mode))
	    file_hdr.c_mode |= CP_IFCHR;
#endif
#ifdef S_ISFIFO
	  else if (S_ISFIFO (file_stat.st_mode))
	    file_hdr.c_mode |= CP_IFIFO;
#endif
#ifdef S_ISLNK
	  else if (S_ISLNK (file_stat.st_mode))
	    file_hdr.c_mode |= CP_IFLNK;
#endif
#ifdef S_ISSOCK
	  else if (S_ISSOCK (file_stat.st_mode))
	    file_hdr.c_mode |= CP_IFSOCK;
#endif
#ifdef S_ISNWK
	  else if (S_ISNWK (file_stat.st_mode))
	    file_hdr.c_mode |= CP_IFNWK;
#endif
	  file_hdr.c_uid = file_stat.st_uid;
	  file_hdr.c_gid = file_stat.st_gid;
	  file_hdr.c_nlink = file_stat.st_nlink;
	  file_hdr.c_rdev_maj = major (file_stat.st_rdev);
	  file_hdr.c_rdev_min = minor (file_stat.st_rdev);
	  file_hdr.c_mtime = file_stat.st_mtime;
	  file_hdr.c_filesize = file_stat.st_size;
	  file_hdr.c_chksum = 0;
	  file_hdr.c_tar_linkname = NULL;

	  /* Strip leading `./' from the filename.  */
	  p = input_name.ds_string;
	  while (*p == '.' && *(p + 1) == '/')
	    {
	      ++p;
	      while (*p == '/')
		++p;
	    }
	  file_hdr.c_name = p;
	  file_hdr.c_namesize = strlen (p) + 1;
	  if ((archive_format == arf_tar || archive_format == arf_ustar)
	      && is_tar_filename_too_long (file_hdr.c_name))
	    {
	      error (0, 0, "%s: file name too long", file_hdr.c_name);
	      continue;
	    }

	  /* Copy the named file to the output.  */
	  switch (file_hdr.c_mode & CP_IFMT)
	    {
	    case CP_IFREG:
#ifndef __MSDOS__
	      if (archive_format == arf_tar || archive_format == arf_ustar)
		{
		  char *otherfile;
		  if ((otherfile = find_inode_file (file_hdr.c_ino,
						    file_hdr.c_dev_maj,
						    file_hdr.c_dev_min)))
		    {
		      file_hdr.c_tar_linkname = otherfile;
		      write_out_header (&file_hdr, out_file_des);
		      break;
		    }
		}
#endif
	      in_file_des = open (input_name.ds_string,
				  O_RDONLY | O_BINARY, 0);
	      if (in_file_des < 0)
		{
		  error (0, errno, "%s", input_name.ds_string);
		  continue;
		}

	      if (archive_format == arf_crcascii)
		file_hdr.c_chksum = read_for_checksum (in_file_des,
						       file_hdr.c_filesize,
						       input_name.ds_string);

	      write_out_header (&file_hdr, out_file_des);
	      copy_files (in_file_des, out_file_des, file_hdr.c_filesize);

#ifndef __MSDOS__
	      if (archive_format == arf_tar || archive_format == arf_ustar)
		add_inode (file_hdr.c_ino, file_hdr.c_name, file_hdr.c_dev_maj,
			   file_hdr.c_dev_min);
#endif

	      pad_output (out_file_des, file_hdr.c_filesize);

	      if (close (in_file_des) < 0)
		error (0, errno, "%s", input_name.ds_string);
	      if (reset_time_flag)
		{
		  times.actime = file_stat.st_atime;
		  times.modtime = file_stat.st_mtime;
		  if (utime (file_hdr.c_name, &times) < 0)
		    error (0, errno, "%s", file_hdr.c_name);
		}
	      break;

	    case CP_IFDIR:
	      file_hdr.c_filesize = 0;
	      write_out_header (&file_hdr, out_file_des);
	      break;

#ifndef __MSDOS__
	    case CP_IFCHR:
	    case CP_IFBLK:
#ifdef CP_IFSOCK
	    case CP_IFSOCK:
#endif
#ifdef CP_IFIFO
	    case CP_IFIFO:
#endif
	      if (archive_format == arf_tar)
		{
		  error (0, 0, "%s not dumped: not a regular file",
			 file_hdr.c_name);
		  continue;
		}
	      file_hdr.c_filesize = 0;
	      write_out_header (&file_hdr, out_file_des);
	      break;
#endif

#ifdef CP_IFLNK
	    case CP_IFLNK:
	      {
		char *link_name = (char *) xmalloc (file_stat.st_size + 1);

		if (readlink (input_name.ds_string, link_name,
			      file_stat.st_size) < 0)
		  {
		    error (0, errno, "%s", input_name.ds_string);
		    free (link_name);
		    continue;
		  }
		if (archive_format == arf_tar || archive_format == arf_ustar)
		  {
		    if (file_stat.st_size + 1 > 100)
		      {
			error (0, 0, "%s: symbolic link too long",
			       file_hdr.c_name);
		      }
		    else
		      {
			link_name[file_stat.st_size] = '\0';
			file_hdr.c_tar_linkname = link_name;
			write_out_header (&file_hdr, out_file_des);
		      }
		  }
		else
		  {
		    write_out_header (&file_hdr, out_file_des);
		    copy_buf_out (link_name, out_file_des, file_stat.st_size);
		    pad_output (out_file_des, file_hdr.c_filesize);
		  }
		free (link_name);
	      }
	      break;
#endif

	    default:
	      error (0, 0, "%s: unknown file type", input_name.ds_string);
	    }

	  if (verbose_flag)
	    fprintf (stderr, "%s\n", input_name.ds_string);
	  if (dot_flag)
	    fputc ('.', stderr);
	}
    }

  /* The collection is complete; append the trailer.  */
  file_hdr.c_ino = 0;
  file_hdr.c_mode = 0;
  file_hdr.c_uid = 0;
  file_hdr.c_gid = 0;
  file_hdr.c_nlink = 1;		/* Must be 1 for SVR4 crc format.  */
  file_hdr.c_dev_maj = 0;
  file_hdr.c_dev_min = 0;
  file_hdr.c_rdev_maj = 0;
  file_hdr.c_rdev_min = 0;
  file_hdr.c_mtime = 0;
  file_hdr.c_chksum = 0;

  file_hdr.c_filesize = 0;
  file_hdr.c_namesize = 11;
  file_hdr.c_name = "TRAILER!!!";
  if (archive_format != arf_tar && archive_format != arf_ustar)
    write_out_header (&file_hdr, out_file_des);
  else
    {
      copy_buf_out (zeros_512, out_file_des, 512);
      copy_buf_out (zeros_512, out_file_des, 512);
    }

  /* Fill up the output block.  */
  clear_rest_of_block (out_file_des);
  empty_output_buffer (out_file_des);
  if (dot_flag)
    fputc ('\n', stderr);
  res = (output_bytes + io_block_size - 1) / io_block_size;
  if (res == 1)
    fprintf (stderr, "1 block\n");
  else
    fprintf (stderr, "%d blocks\n", res);
}

/* Read FILE_SIZE bytes of FILE_NAME from IN_FILE_DES and
   compute and return a checksum for them.  */

static unsigned long
read_for_checksum (in_file_des, file_size, file_name)
     int in_file_des;
     int file_size;
     char *file_name;
{
  unsigned long crc;
  char buf[BUFSIZ];
  int bytes_left;
  int bytes_read;
  int i;

  crc = 0;

  for (bytes_left = file_size; bytes_left > 0; bytes_left -= bytes_read)
    {
      bytes_read = read (in_file_des, buf, BUFSIZ);
      if (bytes_read < 0)
	error (1, errno, "cannot read checksum for %s", file_name);
      if (bytes_read == 0)
	break;
      for (i = 0; i < bytes_read; ++i)
	crc += buf[i] & 0xff;
    }
  if (lseek (in_file_des, 0L, SEEK_SET))
    error (1, errno, "cannot read checksum for %s", file_name);

  return crc;
}

/* Write out NULs to fill out the rest of the current block on
   OUT_FILE_DES.  */

static void
clear_rest_of_block (out_file_des)
     int out_file_des;
{
  while (output_size < io_block_size)
    {
      if ((io_block_size - output_size) > 512)
	copy_buf_out (zeros_512, out_file_des, 512);
      else
	copy_buf_out (zeros_512, out_file_des, io_block_size - output_size);
    }
}

/* Write NULs on OUT_FILE_DES to move from OFFSET (the current location)
   to the end of the header.  */

static void
pad_output (out_file_des, offset)
     int out_file_des;
     int offset;
{
  int pad;

  if (archive_format == arf_newascii || archive_format == arf_crcascii)
    pad = (4 - (offset % 4)) % 4;
  else if (archive_format == arf_tar || archive_format == arf_ustar)
    pad = (512 - (offset % 512)) % 512;
  else if (archive_format != arf_oldascii)
    pad = (2 - (offset % 2)) % 2;
  else
    pad = 0;

  if (pad != 0)
    copy_buf_out (zeros_512, out_file_des, pad);
}
