/* shttpd (c)1999,2000 Stepan Roh
 * see license.txt for copying
 */

/* resource.h
 * nacitani zdroju
 */

#ifndef SHTTPD_RESOURCE_H
#define SHTTPD_RESOURCE_H

#include "global.h"

#include <sys/stat.h>

#include "request.h"
#include "response.h"

/* najde dany zdroj
 * - vraci plne jmeno souboru nebo NULL pri chybe
 * - nastavuje errno - navic EBADMSG -> uri neni absolutni
 * 							 EISDIR  -> uri je adresar, ale nekonci na '/'
 */
char *find_resource (request_uri_t *uri, char *buf, int maxlen, struct stat *st);

#endif