/*
 * Squashfs
 *
 * Copyright (c) 2024, 2025
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
 * print_pager.c
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "error.h"
#include "print_pager.h"
#include "alloc.h"

extern long long read_bytes(int, void *, long long);

static char *pager_command = NULL;
static char *pager_name = NULL;
static int pager_from_env_var = FALSE;

static char *get_base(char *pathname)
{
	char *cur = pathname, *sow = NULL, *eow = NULL;

	while(*cur != '\0') {
		if(*cur == '/')
			cur ++;
		else if(strcmp(cur, ".") == 0)
			cur ++;
		else if(strcmp(cur, "..") == 0)
			cur += 2;
		else if(strncmp(cur, "./", 2) == 0)
			cur +=2;
		else if(strncmp(cur, "../", 3) == 0)
			cur += 3;
		else {
			sow = cur;

			do {
				cur ++;
			} while(*cur != '/' && *cur != '\0');

			eow = cur;
		}
	}

	if(sow == NULL || eow != cur)
		return NULL;
	else
		return sow;
}


int check_and_set_pager(char *pager)
{
	int i, length = strlen(pager);
	char *base;

	/* Check string :-
	 * 1. Isn't empty,
	 * 2. Doesn't contain spaces, tabs, pipes, command separators or file
	 *    redirects.
	 *
	 * Note: this isn't an exhaustive check of what can't be in the
	 *	 pager name, as the execlp() will do this.  It is more
	 *	 intended to check for common shell metacharacters and
	 *	 warn users this isn't supported in a friendlier way.
	 */
	if(length == 0) {
		ERROR("PAGER environment variable is empty!\n");
		return FALSE;
	}

	base = get_base(pager);
	if(base == NULL) {
		ERROR("PAGER doesn't have a name in it or has trailing '/', '.' or '..' characters!\n");
		return FALSE;
	}

	for(i = 0; i < length; i ++) {
		if(pager[i] == ' ' || pager[i] == '\t') {
			ERROR("PAGER cannot have spaces or tabs!\n");
			goto failed;
		} else if(pager[i] == '|' || pager[i] == ';') {
			ERROR("PAGER cannot have pipes or command separators!\n");
			goto failed;
		} else if(pager[i] == '<' || pager[i] == '>' || pager[i] == '&') {
			ERROR("PAGER cannot have file redirections!\n");
			goto failed;
		}
	}

	pager_command = pager;
	pager_name = base;
	pager_from_env_var = TRUE;
	return TRUE;

failed:
	ERROR("If you want to do this, please use a wrapper script!\n");
	return FALSE;
}


static int determine_pager(char *name, char *path1, char *path2)
{
	int bytes, status, res, pipefd[2];
	pid_t child;
	char buffer[1024];

	res = pipe(pipefd);
	if(res == -1)
		BAD_ERROR("Error determining pager, pipe failed\n");

	child = fork();
	if(child == -1)
		BAD_ERROR("Error determining pager, fork failed\n");

	if(child == 0) { /* child */
		close(pipefd[0]);
		close(STDOUT_FILENO);
		res = dup(pipefd[1]);
		if(res == -1)
			exit(EXIT_FAILURE);

		execlp(path1, name, "--version", (char *) NULL);
		if(path2)
			execl(path2, name, "--version", (char *) NULL);
		close(pipefd[1]);
		exit(EXIT_FAILURE);
	}

	/* parent */
	close(pipefd[1]);

	bytes = read_bytes(pipefd[0], buffer, 1024);

	if(bytes == -1)
		BAD_ERROR("Error determining pager, read failed\n");

	if(res == 1024)
		BAD_ERROR("Pager (%s) returned unexpectedly large amount of data for --version\n", pager_command);

	while(1) {
		res = waitpid(child, &status, 0);
		if(res != -1)
			break;
		else if(errno != EINTR)
			BAD_ERROR("Error determining pager, waitpid failed\n");
	}

	close(pipefd[0]);

	if(!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
		/* Pager didn't understand --version?  Return unknown pager */
		return UNKNOWN_PAGER;
	}

	if(strncmp(buffer, "less", strlen("less")) == 0)
		return LESS_PAGER;
	else if(strncmp(buffer, "more", strlen("more")) == 0 ||
				strncmp(buffer, "pager", strlen("pager")) == 0)
		return MORE_PAGER;
	else
		return UNKNOWN_PAGER;
}


void wait_to_die(pid_t process)
{
	int res, status;

	while(1) {
		res = waitpid(process, &status, 0);
		if(res != -1)
			break;
		else if(errno != EINTR) {
			ERROR("Error executing pager, waitpid failed\n");
			return;
		}
	}

	if(!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		ERROR("Pager failed to run or failed with an error status\n");
}


static void run_cmd(char *name, char *path1, char*path2, int no_arg)
{
	int pager = determine_pager(name, path1, path2);

	if(pager == LESS_PAGER) {
		execlp(path1, name, "--quit-if-one-screen", (char *) NULL);
		if(path2)
			execl(path2, name, "--quit-if-one-screen", (char *) NULL);
	} else if(pager == MORE_PAGER) {
		execlp(path1, name, "--exit-on-eof", (char *) NULL);
		if(path2)
			execl(path2, name, "--exit-on-eof", (char *) NULL);
	} else if(no_arg) {
		execlp(path1, name, (char *) NULL);
		if(path2)
			execl(path2, name, (char *) NULL);
	}
}


void simple_cat()
{
	int c;

	while((c = getchar()) != EOF)
		putchar(c);
}


FILE *exec_pager(pid_t *process)
{
	FILE *file;
	int res, pipefd[2];
	pid_t child;

	res = pipe(pipefd);
	if(res == -1)
		BAD_ERROR("Error executing pager, pipe failed\n");

	child = fork();
	if(child == -1)
		BAD_ERROR("Error executing pager, fork failed\n");

	if(child == 0) { /* child */
		close(pipefd[1]);
		close(STDIN_FILENO);
		res = dup(pipefd[0]);
		if(res == -1)
			exit(EXIT_FAILURE);

		if(pager_from_env_var)
			run_cmd(pager_name, pager_command, NULL, TRUE);
		else
			run_cmd("pager", "pager", "/usr/bin/pager", TRUE);

		run_cmd("less", "less", "/usr/bin/less", FALSE);
		run_cmd("more", "more", "/usr/bin/more", FALSE);
		execlp("less", "less",  (char *) NULL);
		execl("/usr/bin/less", "less", (char *) NULL);
		execlp("more", "more",  (char *) NULL);
		execl("/usr/bin/more", "more", (char *) NULL);
		execlp("cat", "cat", (char *) NULL);
		execl("/usr/bin/cat", "cat", (char *) NULL);
		simple_cat();

		close(pipefd[0]);
		exit(0);
	}

	/* parent */
	close(pipefd[0]);

	file = fdopen(pipefd[1], "w");
	if(file == NULL)
		BAD_ERROR("Error executing pager, fdopen failed\n");

	*process = child;
	return file;
}


int get_column_width()
{
	struct winsize winsize;

	if(ioctl(1, TIOCGWINSZ, &winsize) == -1) {
		if(isatty(STDOUT_FILENO))
			ERROR("TIOCGWINSZ ioctl failed, defaulting to 80 "
				"columns\n");
		return 80;
	} else
		return winsize.ws_col;
}


void autowrap_print(FILE *stream, char *text, int maxl)
{
	char *cur = text;
	int tab_out = 0, length;

	while(*cur != '\0') {
		char *sol = cur, *lw = NULL, *eow = NULL;
		int wrapped = FALSE;

		for(length = 0; length < tab_out; length += 8)
			fputc('\t', stream);

		while(length <= maxl && *cur != '\n' && *cur != '\0') {
			if(*cur == '\t')
				tab_out = length = (length + 8) & ~7;
			else
				length ++;

			if(*cur == '\t' || *cur == ' ')
				eow = lw;
			else
				lw = cur;

			if(length <= maxl)
				cur ++;
		}

		if(*cur == '\n')
			cur ++;
		else if(*cur != '\0') {
			if(eow)
				cur = eow + 1;
			else if(cur - sol == 0)
				cur ++;

			if(tab_out >= maxl)
				tab_out = 0;

			wrapped = TRUE;
		}

		while(sol < cur)
			fputc(*sol ++, stream);

		if(wrapped) {
			fputc('\n', stream);

			while(*cur == ' ')
				cur ++;
		}
	}
}


void autowrap_printf(FILE *stream, int maxl, char *fmt, ...)
{
	va_list ap;
	char *text;

	va_start(ap, fmt);
	VASPRINTF(&text, fmt, ap);
	va_end(ap);

	autowrap_print(stream, text, maxl);
	free(text);
}
