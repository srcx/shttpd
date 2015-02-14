/* shttpd (c)1999,2000 Stepan Roh
 * see license.txt for copying
 */

/* response.c
 * obsluha pozadavku - generovani odpovedi
 */

#include "response.h"

#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include "logfile.h"
#include "resource.h"
#include "config.h"
#include "mime.h"

/* posle danou odpoved */
void send_response (response_t *res, FILE *f, request_t *req, void *buffer, int buffer_len) {
  char buf[64]; int data_sent = 0;

  /* Response header */
  fprintf (f, "%s %d %s\r\n", http_version_str[res->version], res->status, get_status_str (res->status));
  /* zbytek hlavicky */
  fprintf (f, "Date: %s\r\n", rfc1123_time (buf, 64, time (NULL)));
  fprintf (f, "Server: " SHTTPD_VERSION_STR "\r\n");
  fprintf (f, "Connection: close\r\n");	/* persistentni spojeni nejsou podporovana */
  { http_header_t *header = res->headers;
    while (header) {
      fprintf (f, "%s: %s\r\n", header->name, header->content);
      header = header->next;
    }
  }
  switch (res->data_type) {
    case DATA_MEM :
      fprintf (f, "Content-Length: %d\r\n\r\n", res->data_len);
      data_sent = fwrite (res->data.adr, 1, res->data_len, f);
      break;
    case DATA_FILE : {
      FILE *df; int i;

      df = fopen (res->data.name, "r");
      if (!df) {
        log_printf (ERROR_LOG, LOG_LEVEL_ERR, "Error opening file %s\n", res->data.name);
        data_sent += fprintf (f, "\r\n");
        break;
      }
      if (res->data_len == -1) {
        fseek (df, 0L, SEEK_END);
        res->data_len = ftell (df);
        fseek (df, 0L, SEEK_SET);
      }
      fprintf (f, "Content-Length: %d\r\n\r\n", res->data_len);
      data_sent = 0;
      while ((i = fread (buffer, 1, buffer_len, df))) {
        i = fwrite (buffer, 1, i, f);
        if (!i) break;
        data_sent += i;
      }
      fclose (df);
      break; }
    case DATA_STREAM : {
      int i;
      fprintf (f, "Content-Length: %d\r\n\r\n", res->data_len);
      data_sent = 0;
      while ((i = fread (buffer, 1, buffer_len, res->data.file))) {
        i = fwrite (buffer, 1, i, f);
        if (!i) break;
        data_sent += i;
      }
      break; }
    default :
      if (res->status == HTTP_STATUS_OK) {
        fprintf (f, "Content-Length: 0\r\n\r\n");
        data_sent = 0;
      } else {
        char *msg;
        if (server_config.status_page && (msg = get_sarray_item (server_config.status_page, res->status))) {
          fprintf (f, "Content-Type: text/html\r\n\r\n");
          data_sent = fprintf (f, msg, req->uri.path);
        } else
          data_sent = fprintf (f, "\r\n");
      }
      break;
  }
  
  /* access_log - [datum] odkud "pozadavek" status */
  if ((req->method == HTTP_METH_HEAD) || (req->method == HTTP_METH_GET)) {
    log_printf (ACCESS_LOG, LOG_LEVEL_IGN, "%s \"%s\" %d %d\n",
      req->req_from, req->req_str, res->status, data_sent);
  }

  fflush (f);
}

/* zkonstruuje odpoved na zaklade pozadavku
 * alokuje novy response_t, paklize res == NULL */
response_t *make_response (request_t *req, FILE *f, response_t *res) {
  char full_name[MAX_HTTP_LINE_LEN+1]; struct stat st;

  if (!res) {
    res = malloc (sizeof (response_t));
    if (!res) return NULL;
    res->status = HTTP_STATUS_OK;
  }

  res->version = HTTP_VER_1_0;
  res->headers = NULL;
  res->data_len = 0;
  res->data_type = DATA_EMPTY;
  res->free_data_name = 0;

  if (res->status == HTTP_STATUS_OK) {
    switch (req->method) {
      case HTTP_METH_TRACE :		/* vrati pozadavek zpatky */
        add_header (res, "Content-Type", 0, "message/html", 0);
        res->data_type = DATA_STREAM;
        res->data_len = req->content_length;
        res->data.file = f;
        break;
      case HTTP_METH_OPTIONS :
        if (!strcmp (req->uri.path, "*")) break;
      case HTTP_METH_GET :
      case HTTP_METH_HEAD :
        if (!find_resource (&(req->uri), full_name, MAX_HTTP_LINE_LEN, &st)) {
          switch (errno) {
            case ENOENT : res->status = HTTP_STATUS_NOT_FOUND; break;
            case EACCES : res->status = HTTP_STATUS_FORBIDDEN; break;
            case EBADMSG : res->status = HTTP_STATUS_BAD_REQUEST; break;
            case EISDIR : { int l; char *moved_loc;
              res->status = HTTP_STATUS_MOVED_PERMANENTLY;
              l = strlen (req->uri.path);
              moved_loc = malloc (l + 2);
              if (!moved_loc) {
                log_printf (ERROR_LOG, LOG_LEVEL_ERR, "Out of memory\n");
                return NULL;
              }
              strcpy (moved_loc, req->uri.path);
              moved_loc[l] = '/';
              moved_loc[l+1] = 0;
              free (req->uri.path);
              req->uri.path = moved_loc;
              break; }
          }
        }
        break;
      case HTTP_METH_POST :		/* pouze staticke stranky */
      case HTTP_METH_PUT :		/* zadne ukladani na server */
      case HTTP_METH_DELETE :
        res->status = HTTP_STATUS_METHOD_NOT_ALLOWED;
        break;
      default :
        break;
    } 
  }
  if (res->status != HTTP_STATUS_OK) {	/* chybove hlasky */
    switch (res->status) {
      case HTTP_STATUS_METHOD_NOT_ALLOWED :
        add_header (res, "Allow", 0, "GET HEAD OPTIONS", 0);
        break;
      case HTTP_STATUS_MOVED_PERMANENTLY :
        add_header (res, "Location", 0, req->uri.path, 0);
      default :
        break;
    }
  } else {
    if (req->method == HTTP_METH_OPTIONS)
      add_header (res, "Allow", 0, "GET HEAD OPTIONS", 0);
    else {
      /* ??? Expires */
      if ((req->method == HTTP_METH_GET) || (req->method == HTTP_METH_HEAD)) {
        /* urceni media typu */
        add_header (res, "Content-Type", 0, (char *)find_mime (full_name), 0);
        if (req->method == HTTP_METH_GET) {
          /* pripojeni souboru */
          res->data_type = DATA_FILE;
          res->data_len = -1;
          if (!(res->data.name = strdup (full_name))) {
            log_printf (ERROR_LOG, LOG_LEVEL_ERR, "Out of memory\n");
            return NULL;
          }
          res->free_data_name = 1;
        }
      }
    }
  }

  return res;
}

/* zruseni struktury s odpovedi */
void free_response (response_t *res) {
  if (res) {
    if (res->free_data_name) free (res->data.name);
    while (res->headers) {
      http_header_t *h;
      if (res->headers->free_name) free (res->headers->name);
      if (res->headers->free_content) free (res->headers->content);
      h = res->headers->next;
      free (res->headers);
      res->headers = h;
    }
    free (res);
  }
}

/* pridani nove hlavicky - bez kontroly existence */
void add_header (response_t *res, char *name, int free_name, char *content, int free_content) {
  http_header_t *nh;

  nh = malloc (sizeof (http_header_t));
  if (!nh) return;
  nh->name = name;
  nh->free_name = free_name;
  nh->content = content;
  nh->free_content = free_content;
  nh->next = res->headers;
  res->headers = nh;
}
