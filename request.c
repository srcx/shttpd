/* shttpd (c)1999,2000 Stepan Roh
 * see license.txt for copying
 */

/* request.c
 * obsluha pozadavku
 */

#include "request.h"

#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "logfile.h"
#include "utils.h"
#include "response.h"
#include "config.h"

#define HTTP_TOK_DELIM			 "\n\r\t "
#define HTTP_FIELD_TOK_DELIM	":\n\r\t "

/* parsovani request_uri */
request_uri_t *parse_request_uri (request_uri_t *req_uri, const char *str) {
  char *q = NULL, *p = NULL, *e = NULL;

  req_uri->query = req_uri->params = req_uri->path = NULL;
  if ((q = strchr (str, '?'))) {
    req_uri->query = strdup (q);
    if (!req_uri->query) return NULL;
  }
  e = q;
  if ((p = strchr (str, ';'))) {
    if (!q) {
      req_uri->params = strdup (p);
      if (!req_uri->params) return NULL;
      e = p;
    } else if (p < q) {
      req_uri->params = malloc ((q - p) + 1);
      if (!req_uri->params) return NULL;
      strncpy (req_uri->params, p, (q - p));
      req_uri->params[q - p] = 0;
      e = p;
    }
  }
  if (!e) {
    req_uri->path = strdup (str);
    if (!req_uri->path) return NULL;
  } else {
    req_uri->path = malloc ((e - str) + 1);
    if (!req_uri->path) return NULL;
    strncpy (req_uri->path, p, (e - str));
    req_uri->path[e - str] = 0;
  }
  decode_url (req_uri->path);
 
  return req_uri;
}

/* parsovani pozadavku - nastavi f na prvni datovy byte
 * alokuje novy request_t, paklize req == NULL */
request_t *parse_request (FILE *f, status_t *status, request_t *req) {
  char buf[MAX_HTTP_LINE_LEN+1];

  *status = HTTP_STATUS_OK;
  if (!req) req = malloc (sizeof (request_t));
  if (!req) return NULL;
  req->uri.query = req->uri.params = req->uri.path = req->req_str = req->req_from = NULL;
  /* cteni hlavicky */
  {
    char *x; int i;
    
    if (!fgets (buf, MAX_HTTP_LINE_LEN, f)) { *status = HTTP_STATUS_BAD_REQUEST; goto lend; }

    req->req_str = strdup (buf);
    { char *xx;
      xx = strchr (req->req_str, '\n');
      if (xx) *xx = 0;
      xx = strchr (req->req_str, '\r');
      if (xx) *xx = 0;
    }
    /* metoda */
    if (!(x = strtok (buf, HTTP_TOK_DELIM))) { *status = HTTP_STATUS_BAD_REQUEST; goto lend; }
    i = find_str (method_str, x, 0);
    if (i == -1) { *status = HTTP_STATUS_METHOD_NOT_ALLOWED; goto lend; }
    req->method = (method_t)i;
    /* URI */
    if (!(x = strtok (NULL, HTTP_TOK_DELIM))) { *status = HTTP_STATUS_BAD_REQUEST; goto lend; }
    if (!parse_request_uri (&req->uri, x)) { *status = HTTP_STATUS_BAD_REQUEST; goto lend; }
    /* HTTP verze */
    if (!(x = strtok (NULL, HTTP_TOK_DELIM))) { *status = HTTP_STATUS_BAD_REQUEST; goto lend; }
    i = find_str (http_version_str, x, 0);
    if (i == -1) { *status = HTTP_STATUS_HTTP_VERSION_NOT_SUPPORTED; goto lend; }
    req->version = (http_version_t)i;

  }
  while (fgets (buf, MAX_HTTP_LINE_LEN, f)) {
    char *x;
    
    if (!(x = strtok (buf, HTTP_FIELD_TOK_DELIM))) break; /* prazdna radka - konec hlavicky */
    switch (find_str (header_field_str, x, 1)) {
      case HTTP_CONTENT_LENGTH :
        req->content_length = atoi (strtok (NULL, HTTP_TOK_DELIM));
        break;
    }
  }

  lend:
  return req;
}

/* zruseni struktury s pozadavkem */
void free_request (request_t *req) {
  if (req) {
    if (req->uri.path) free (req->uri.path);
    if (req->uri.params) free (req->uri.params);
    if (req->uri.query) free (req->uri.query);
    if (req->req_str) free (req->req_str);
    if (req->req_from) free (req->req_from);
    free (req);
  }
}

/* obsluha pozadavku na danem socketu */
void handle_request (struct sockaddr_in client, int sock, void *buffer, int buffer_len) {
  request_t *req = NULL; response_t *res = NULL; FILE *fr = NULL, *fw = NULL;

  log_printf (ERROR_LOG, LOG_LEVEL_INFO, "Connection from %s, port %d\n",
    inet_ntoa (client.sin_addr), ntohs (client.sin_port));

  fr = fdopen (sock, "r");
  if (!fr) { log_printf (ERROR_LOG, LOG_LEVEL_ERR, "Can't make stream from socket\n"); goto lend; }
  fw = fdopen (sock, "w");
  if (!fw) { log_printf (ERROR_LOG, LOG_LEVEL_ERR, "Can't make stream from socket\n"); goto lend; }
  res = malloc (sizeof (response_t));
  if (!res) { log_printf (ERROR_LOG, LOG_LEVEL_ERR, "Out of memory\n"); goto lend; }
  req = parse_request (fr, &(res->status), NULL);
  if (!req) { log_printf (ERROR_LOG, LOG_LEVEL_ERR, "Out of memory\n"); goto lend; }
  if (server_config.remote_names) {
    struct hostent *h;
    h = gethostbyaddr ((char *)&client.sin_addr, sizeof (client.sin_addr), AF_INET);
    if (!h) req->req_from = strdup (inet_ntoa (client.sin_addr)); else req->req_from = strdup (h->h_name);
  } else {
    req->req_from = strdup (inet_ntoa (client.sin_addr));
  }
    
  if (make_response (req, fr, res))
    send_response (res, fw, req, buffer, buffer_len);

  lend:
  if (fr) fclose (fr);
  if (fw) fclose (fw);
  free_request (req);
  free_response (res);
  close (sock); 
}
