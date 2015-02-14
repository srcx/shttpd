/* shttpd (c)1999,2000 Stepan Roh
 * see license.txt for copying
 */

/* response.h
 * obsluha pozadavku - generovani odpovedi
 */

#ifndef SHTTPD_RESPONSE_H
#define SHTTPD_RESPONSE_H

#include "global.h"

#include <stdio.h>
#include "http.h"
#include "request.h"

/* typ dat */
typedef enum {
  DATA_EMPTY, DATA_FILE, DATA_STREAM, DATA_MEM,
} data_type_t;

/* dodatecne HTTP hlavicky */
typedef struct http_header_t {
  char *name;					/* jmeno hlavicky */
  char *content;				/* obsah */
  int free_name;				/* zrusit name pri ruseni zaznamu ? */
  int free_content;				/* zrusit content pri ruseni zaznamu ? */
  struct http_header_t *next;	/* dalsi hlavicka v rade */
} http_header_t;

/* odpoved */
typedef struct {
  http_version_t version;		/* verze HTTP */
  status_t status;				/* stavovy kod */
  http_header_t *headers;		/* dodatecne hlavicky */
  data_type_t data_type;		/* typ dat */
  int data_len;					/* delka dat (nebo -1 pro DATA_FILE - spocte si sam) */
  union {
    void *adr;			/* pro DATA_MEM */
    char *name;			/* pro DATA_FILE */
    FILE *file;			/* pro DATA_STREAM */
  } data;						/* data */
  int free_data_name;			/* zrusit data.name ? */
} response_t;

/* posle danou odpoved */
void send_response (response_t *res, FILE *f, request_t *req, void *buffer, int buffer_len);

/* zkonstruuje odpoved na zaklade pozadavku
 * alokuje novy response_t, paklize res == NULL */
response_t *make_response (request_t *req, FILE *f, response_t *res);

/* zruseni struktury s odpovedi */
void free_response (response_t *res);

/* pridani nove hlavicky - bez kontroly existence */
void add_header (response_t *res, char *name, int free_name, char *content, int free_content);

#endif