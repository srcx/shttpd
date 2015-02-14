/* shttpd (c)1999,2000 Stepan Roh
 * see license.txt for copying
 */

/* thread_mgr.h
 * obsluha vlaken
 */

#ifndef SHTTPD_THREAD_MGR
#define SHTTPD_THREAD_MGR

#include "global.h"

#include <stdio.h>
#include <netinet/in.h>

/* inicializuje vlakna */
int init_threads (void);

/* obslouzi jeden pozadavek */
void handle_request_thread (struct sockaddr_in client, int sock);

/* ukonci vlakna */
void done_threads (void);

#endif