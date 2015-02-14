/* shttpd (c)1999,2000 Stepan Roh
 * see license.txt for copying
 */

/* http.c
 * definice pro HTTP protokol
 */

#include "http.h"

#include <stdlib.h>
#include "utils.h"

const char *method_str[] = {
  "OPTIONS", "GET", "HEAD", "POST", "PUT", "DELETE", "TRACE", NULL
};

const char *http_version_str[] = {
  "HTTP/0.9", "HTTP/1.0", "HTTP/1.1", NULL
};

static sarray_item_t status_array[] = {
  { HTTP_STATUS_OK, "OK" },
  { HTTP_STATUS_BAD_REQUEST, "Bad Request" },
  { HTTP_STATUS_FORBIDDEN, "Forbidden" },
  { HTTP_STATUS_NOT_FOUND, "Not Found" },
  { HTTP_STATUS_METHOD_NOT_ALLOWED, "Method Not Allowed" },
  { HTTP_STATUS_HTTP_VERSION_NOT_SUPPORTED, "HTTP Version not supported" },
  { HTTP_STATUS_MOVED_PERMANENTLY, "Moved Permanently" },
  { -1, NULL }
};

/* jmena stavovych kodu */
const char *get_status_str (status_t status) {
  void *p = get_sarray_item (status_array, status);
  if (p) return (const char *)p;
  return "";
}

/* jmena poli hlavicky */
const char *header_field_str[] = {
  "Content-Length", NULL
};

/* nahradi '%' hex hex */
char *decode_url (char *url) {
  char *x = url, *p = url; char v = 0, n = 0;

  while (*url) {
    if (n > 1) {
      n = 0;
      v += hex2int (*url);
      *x = v; x++;
    } else if (n > 0) {
      n = 2;
      v = hex2int (*url) * 16;
    } else if (*url == '%') {
      n = 1;
    } else {
      *x = *url;
      x++;
    }
    url++;
  }

  return p;
}

/* zformatuje cas podle rfc1123 */
char *rfc1123_time (char *buf, int buflen, time_t t) {
  struct tm *gmt;

  gmt = gmtime (&t);
  strftime (buf, buflen, "%a, %d %b %Y %H:%M:%S GMT", gmt);
  return buf;
}
