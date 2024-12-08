/*
 * Copyright (C) 2014, 2015 Firejail Authors
 *
 * This file is part of firejail project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include "firejail.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void check_output(int argc, char **argv) {
	int i;
	char *outfile = NULL;
//	drop_privs(0);

	int found = 0;
	for (i = 1; i < argc; i++) {
		if (strncmp(argv[i], "--", 2) != 0) {
			return;
		}
		if (strcmp(argv[i], "--") == 0) {
			return;
		}
		if (strncmp(argv[i], "--output=", 9) == 0) {
			found = 1;
			invalid_filename(argv[i] + 9);
			outfile = argv[i] + 9;

			// do not accept directories, links, and files with ".."
			if (strstr(outfile, "..") || is_link(outfile) || is_dir(outfile)) {
				fprintf(stderr, "Error: invalid output file. Links, directories and files with \"..\" are not allowed.\n");
				exit(1);
			}
			
			struct stat s;
			if (stat(outfile, &s) == 0) {
				// check permissions
				if (s.st_uid != getuid() || s.st_gid != getgid()) {
					fprintf(stderr, "Error: the output file needs to be owned by the current user.\n");
					exit(1);
				}
				
				// check hard links
				if (s.st_nlink != 1) {
					fprintf(stderr, "Error: no hard links allowed.\n");
					exit(1);
				}
			}

			// drop privileges and try to open the file for writing
			drop_privs(0);
			/* coverity[toctou] */
			FILE *fp = fopen(outfile, "a");
			if (!fp) {
				fprintf(stderr, "Error: cannot open output file %s\n", outfile);
				exit(1);
			}
			fclose(fp);
			break;
		}
	}
	if (!found)
		return;


	// build the new command line
	int len = 0;
	for (i = 0; i < argc; i++) {
		len += strlen(argv[i]) + 1; // + ' '
	}
	len += 50 + strlen(outfile); // tee command
	
	char *cmd = malloc(len + 1); // + '\0'
	if (!cmd)
		errExit("malloc");
	
	char *ptr = cmd;
	for (i = 0; i < argc; i++) {
		if (strncmp(argv[i], "--output=", 9) == 0)
			continue;
		ptr += sprintf(ptr, "%s ", argv[i]);
	}
	sprintf(ptr, "| %s/lib/firejail/ftee %s", PREFIX, outfile);

	// run command
	char *a[4];
	a[0] = "/bin/bash";
	a[1] = "-c";
	a[2] = cmd;
	a[3] = NULL;

	execvp(a[0], a); 

	perror("execvp");
	exit(1);
}
