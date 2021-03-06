/*
 * PostgreSQL C-language bindings example.
 *
 * Copyright 2015 Automation Professionals, LLC <sales@automation-pros.com>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. The name of the author may not be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */
#define _GNU_SOURCE

#include <endian.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <libpq-fe.h>

int keycount = 0;
const char **keys = NULL;
const char **vals = NULL;

int  tblcount = 0;
char **tables = NULL;

/*
 * Separate program arguments into two lists, one holding everything of
 * the form alphanumerickey=anythingvalue, the other holding everything
 * else.  When saving the key/value pairs, modify the argument string
 * to split it into two, saving the key pointer and value pointer
 * separately.
 */
int get_params(int argc, char **argv) {
	keys = calloc(argc, sizeof(char **));
	if (keys == NULL) {
		perror("Unable to allocate for connection parameter keys!");
		exit(1);
	}
	vals = calloc(argc, sizeof(char **));
	if (vals == NULL) {
		perror("Unable to allocate for connection parameter values!");
		exit(1);
	}
	tables = calloc(argc, sizeof(char **));
	if (tables == NULL) {
		perror("Unable to allocate for non-parameter arguments!");
		exit(1);
	}

	for (int i=1; i<argc; i++) {
		int eqidx = strspn(argv[i], "0123456789_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
		if (argv[i][eqidx] == '=') {
			argv[i][eqidx] = 0;
			keys[keycount] = argv[i];
			vals[keycount++] = argv[i]+eqidx+1;
		} else {
			tables[tblcount++] = argv[i];
		}
	}

	return tblcount;
}

struct _pgt {
	Oid oid;
	char *name;
	int16_t bytes;
	Oid elemoid;
	Oid arrayoid;
} *pgtypes = NULL;
int pgtypecount = 0;

int select_table(PGconn *pgc, char *arg) {
	int retv = 0;
	char *query = NULL;
	int qlen = asprintf(&query, "Select * From %s Limit 50;", arg);
	if (qlen == -1) {
		perror("Unable to allocate query string memory!");
		return 1;
	}
	PGresult *pgr = PQexec(pgc, query);
	if (pgr && PQresultStatus(pgr) == PGRES_TUPLES_OK) {
		int cols = PQnfields(pgr);
		char *fname = NULL;
		printf("Table %s has %d columns:\n", arg, cols);
		PQprintOpt opts = {1,1,0,0,0,0,"|","","",&fname};
		PQprint(stdout, pgr, &opts);
	} else {
		if (pgr)
			fprintf(stderr, "Unable to Select from %s : %s\n", arg, PQresultErrorMessage(pgr));
		else
			fprintf(stderr, "Unable to Select from %s : %s\n", arg, PQerrorMessage(pgc));
		retv = 2;
	}
	PQclear(pgr);
	return retv;
}

int main(int argc, char **argv) {
	int retv = 0;
	get_params(argc, argv);

	PGconn *pgc = PQconnectdbParams(keys, vals, 0);
	ConnStatusType pgcs = PQstatus(pgc);
	if (pgcs == CONNECTION_OK) {
		printf("Connected to %s:%s database %s as %s\n",
			   PQhost(pgc),
			   PQport(pgc), 
			   PQdb(pgc),
			   PQuser(pgc));
		for (int i=0; retv==0 && i<tblcount; i++)
			retv = select_table(pgc, tables[i]);
	} else {
		fprintf(stderr, "Connection Failed: %s\n", PQerrorMessage(pgc));
		retv = 2;
	}

	PQfinish(pgc);
	return retv;
}

/*
 * kate: tab-width 4; indent-width 4; tab-indents on; dynamic-word-wrap off; indent-mode cstyle; auto-brackets on; line-numbers on;
 */
