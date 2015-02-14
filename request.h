/* shttpd (c)1999,2000 Stepan Roh
 * see license.txt for copying
 */

/* request.h
 * obsluha pozadavku
 */

#ifndef SHTTPD_REQUEST_H
#define SHTTPD_REQUEST_H

#include "global.h"

#include <stdio.h>
#include <netinet/in.h>
#include "http.h"

/* URI v pozadavku */
typedef struct {
  char *path;			/* cesta */
  char *params;			/* za ';' vcetne */
  char *query;			/* za '?' vcetne */
} request_uri_t;

/* pozadavek */
typedef struct {
  char *req_str;			/* prvni radka pozadavku */
  method_t method;			/* metoda */
  request_uri_t uri;		/* URI */
  http_version_t version;	/* HTTP verze */
  int content_length;		/* Content-Length */
  char *req_from;			/* adresa klienta */
} request_t;

/* parsovani request_uri */
request_uri_t *parse_request_uri (request_uri_t *req_uri, const char *str);

/* parsovani pozadavku - nastavi f na prvni datovy byte
 * alokuje novy request_t, paklize req == NULL */
request_t *parse_request (FILE *f, status_t *status, request_t *req);

/* zruseni struktury s pozadavkem */
void free_request (request_t *req);

/* obsluha pozadavku na danem socketu */
void handle_request (struct sockaddr_in client, int sock, void *buffer, int buffer_len);

#endif
