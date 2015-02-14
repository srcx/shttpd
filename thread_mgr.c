/* shttpd (c)1999,2000 Stepan Roh
 * see license.txt for copying
 */

/* thread_mgr.c
 * obsluha vlaken
 */

#include "thread_mgr.h"

#include "request.h"

#define BUFFER_LEN	65535

#ifdef PTHREADS

#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "config.h"
#include "logfile.h"

typedef struct {
  pthread_t th;					/* vlakno */
  int free;						/* volny zaznam ? */
  int temp;						/* docasne vlakno ? */
  void *buffer;			/* I/O buffer vlakna */
  int buffer_len;		/* delka bufferu */
} thread_item_t;

static thread_item_t *threads = NULL;

/* cekajici pozadavek */
static struct sockaddr_in req_client;
static int req_sock;

/* cekani na pozadavek */
static sem_t waiting_for_request;

/* cekani na prevzeti pozadavku */
static sem_t waiting_for_handle;

/* vlakno */
static void *do_thread (void *arg) {
  thread_item_t *t = (thread_item_t *)arg;
  struct sockaddr_in client; int sock;

  log_printf (ERROR_LOG, LOG_LEVEL_INFO, "Thread number %d has started\n", t - threads);
  do {
    sem_wait (&waiting_for_request);	/* ceka na pozadavek */
    client = req_client; sock = req_sock;
    log_printf (ERROR_LOG, LOG_LEVEL_INFO, "Thread number %d is handling request\n", t - threads);
    sem_post (&waiting_for_handle);
    handle_request (client, sock, t->buffer, t->buffer_len);
    log_printf (ERROR_LOG, LOG_LEVEL_INFO, "Thread number %d closed connection\n", t - threads);
  } while (!t->temp);
  if (t->buffer) free (t->buffer);
  log_printf (ERROR_LOG, LOG_LEVEL_INFO, "Thread number %d is terminating\n", t - threads);
  
  return NULL;
}

/* inicializuje vlakna */
int init_threads (void) {
  int i;
  if (pthread_mutex_init (&log_mutex, NULL)) return -1;
  if (sem_init (&waiting_for_request, 0, 0)) return -1;
  if (sem_init (&waiting_for_handle, 0, 0)) return -1;
  threads = malloc (sizeof (thread_item_t) * server_config.max_threads);
  for (i = 0; i < server_config.max_threads; i++) {
    threads[i].free = 1;
  }
  /* vytvoreni zakladni mnoziny vlaken */
  for (i = 0; i < server_config.min_threads; i++) {
    if (pthread_create (&threads[i].th, NULL, do_thread, &threads[i])) return -1;
    threads[i].free = threads[i].temp = 0;
    threads[i].buffer_len = BUFFER_LEN;
    if (!(threads[i].buffer = malloc (threads[i].buffer_len))) return -1;
  }
  return 0;
}

/* obslouzi jeden pozadavek */
void handle_request_thread (struct sockaddr_in client, int sock) {
  int val, i;
  req_client = client; req_sock = sock;
  sem_post (&waiting_for_request);
  sem_getvalue (&waiting_for_request, &val);
  if (val > 0) {	/* vsichni maji praci */
    for (i = server_config.min_threads; i < server_config.max_threads; i++) {
      if (threads[i].free) {
        threads[i].buffer_len = BUFFER_LEN;
        if (!(threads[i].buffer = malloc (threads[i].buffer_len))) break;
        threads[i].temp = 1;
        if (pthread_create (&threads[i].th, NULL, do_thread, &threads[i])) break;
        threads[i].free = 0;
        break;
      }
    }
  }
  sem_wait (&waiting_for_handle);
}

/* ukonci vlakna */
void done_threads (void) {
  int i;

  pthread_mutex_destroy (&log_mutex);
  for (i = 0; i < server_config.max_threads; i++) {
    if (!threads[i].free) {
      pthread_cancel (threads[i].th);
      if (threads[i].buffer) free (threads[i].buffer);
    }
  }
  sem_destroy (&waiting_for_request);
  sem_destroy (&waiting_for_handle);
  free (threads);
}

#else

#include <stdlib.h>

static void *buffer = NULL;
static int buffer_len = 0;

/* inicializuje vlakna */
int init_threads (void) {

  buffer_len = BUFFER_LEN;
  if (!(buffer = malloc (buffer_len))) return -1;
  
  return 0;
}

/* obslouzi jeden pozadavek */
void handle_request_thread (struct sockaddr_in client, int sock) {
  handle_request (client, sock, buffer, buffer_len);
}

/* ukonci vlakna */
void done_threads (void) {
  if (buffer) free (buffer);
}

#endif

