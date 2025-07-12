#ifndef MKSQUASHFS_ERROR_H
#define MKSQUASHFS_ERROR_H
/*
 * Create a squashfs filesystem.  This is a highly compressed read only
 * filesystem.
 *
 * Copyright (c) 2021, 2024, 2025
 * Phillip Lougher <phillip@squashfs.org.uk>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * mksquashfs_error.h
 */

#include <stdio.h>

#include "error.h"

extern int exit_on_error;
extern void prep_exit();
extern int display_info;
extern FILE *info_file;

#define INFO(s, args...) \
		do {\
			if(display_info) { \
				if(info_file) \
					fprintf(info_file, s, ## args); \
				else \
					progressbar_info(s, ## args); \
			} \
		} while(0)


#define ERROR_START(s, args...) \
		do { \
			disable_progress_bar(); \
			fprintf(stderr, s, ## args); \
		} while(0)

#define ERROR_EXIT(s, args...) \
		do {\
			if (exit_on_error) { \
				fprintf(stderr, "\n"); \
				EXIT_MKSQUASHFS(); \
			} else { \
				fprintf(stderr, s, ## args); \
				enable_progress_bar(); \
			} \
		} while(0)

#define EXIT_MKSQUASHFS() \
		do {\
			prep_exit();\
			exit(1);\
		} while(0)
#endif
