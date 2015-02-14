/* shttpd (c)1999,2000 Stepan Roh
 * see license.txt for copying
 */

/* http.h
 * definice pro HTTP protokol
 */

#ifndef SHTTPD_HTTP_H
#define SHTTPD_HTTP_H

#include "global.h"

#include <time.h>

/* verze serveru */
#define SHTTPD_VERSION_STR	"shttpd/0.3.1"

#define MAX_HTTP_LINE_LEN	256

/* HTTP metody */
typedef enum {
  HTTP_METH_OPTIONS, HTTP_METH_GET, HTTP_METH_HEAD, HTTP_METH_POST, HTTP_METH_PUT, HTTP_METH_DELETE, HTTP_METH_TRACE,
} method_t;

/* jmena HTTP metod */
extern const char *method_str[];

/* verze HTTP */
typedef enum {
  HTTP_VER_0_9, HTTP_VER_1_0, HTTP_VER_1_1,
} http_version_t;

/* jmena verzi HTTP */
extern const char *http_version_str[];

/* stavove kody */
typedef enum {
  HTTP_STATUS_OK							= 200,
  HTTP_STATUS_MOVED_PERMANENTLY				= 301,
  HTTP_STATUS_BAD_REQUEST					= 400,
  HTTP_STATUS_FORBIDDEN						= 403,
  HTTP_STATUS_NOT_FOUND						= 404,
  HTTP_STATUS_METHOD_NOT_ALLOWED			= 405,
  HTTP_STATUS_HTTP_VERSION_NOT_SUPPORTED	= 505,
} status_t;

/* jmena stavovych kodu */
const char *get_status_str (status_t status);

/* pole hlavicky */
typedef enum {
  HTTP_CONTENT_LENGTH,
} header_field_t;

/* jmena poli hlavicky */
extern const char *header_field_str[];

/* nahradi '%' hex hex */
char *decode_url (char *url);

/* zformatuje cas podle rfc1123 */
char *rfc1123_time (char *buf, int buflen, time_t t);

#endif
