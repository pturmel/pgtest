/*
 * PostgreSQL C-language bindings example.
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
const char **tables = NULL;

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

	int i=1;
	for (; i<argc; i++) {
		if (strcmp(argv[i], "--")==0) {
			i++;
			break;
		}
		int eqidx = strspn(argv[i], "0123456789_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
		if (argv[i][eqidx] == '=') {
			argv[i][eqidx] = 0;
			keys[keycount] = argv[i];
			vals[keycount++] = argv[i]+eqidx+1;
		} else {
			tables[tblcount++] = argv[i];
		}
	}

	for (; i<argc; i++)
		tables[tblcount++] = argv[i];

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

int exec_table(PGconn *pgc) {
	int retv = 0;
	if (tblcount<1) {
		fprintf(stderr, "No query string supplied\n");
		return 1;
	}
	PGresult *pgr = PQexecParams(pgc, tables[0], tblcount-1, NULL, tables+1, NULL, NULL, 0);
	if (pgr) {
		printf("Result: %s\n", PQresStatus(PQresultStatus(pgr)));
		if (PQresultStatus(pgr) != PGRES_COMMAND_OK)
			fprintf(stderr, "%s\n", PQresultErrorMessage(pgr));
	} else {
		fprintf(stderr, "Unable to Execute %s : %s\n", tables[0], PQerrorMessage(pgc));
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
		exec_table(pgc);
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
