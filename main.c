/* shttpd (c)1999,2000 Stepan Roh
 * see license.txt for copying
 */

/* main.c
 * hlavni soubor
 */

#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include "config.h"
#include "request.h"
#include "logfile.h"
#include "thread_mgr.h"

#include <arpa/inet.h>

static int main_socket;

#ifdef PTHREADS
#include <pthread.h>

static pthread_t main_thread;
#endif

/* ukonceni */
void main_exit (int retval) {

  log_printf (ERROR_LOG, LOG_LEVEL_SYS, "shttpd shutdown\n");

  done_threads ();

  if (close (main_socket) < 0) {
    perror ("Error closing main socket");
    exit (EXIT_FAILURE);
  }

  done_config ();

  exit (retval);
}

/* osetreni signalu */
void signal_handler (int signum) {
#ifdef PTHREADS
  if (pthread_self () != main_thread) return;
#endif
  main_exit (EXIT_SUCCESS);
}

/* vytvoreni noveho serveroveho socketu */
int make_socket (int port) {
  int sock; struct sockaddr_in server; int buf = 1;
  
  sock = socket (PF_INET, SOCK_STREAM, 0);
  if (sock < 0) return -1;
  setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, (char *) &buf, sizeof (int));
  server.sin_family = AF_INET;
  server.sin_port = htons (port);
  server.sin_addr.s_addr = htonl (INADDR_ANY);
  if (bind (sock, (struct sockaddr *) &server, sizeof (server)) < 0) return -1;

  return sock;
}

int main (int argc, char *argv[]) {
  char *cfg_file = "shttpd.cfg", *prog_loc = argv[0];

  argv++;
  while (*argv) {
    if ((!strcmp (*argv, "-?")) || (!strcmp (*argv, "-h")) || (!strcmp (*argv, "--help"))) {
      fprintf (stderr, SHTTPD_VERSION_STR " (c)1999,2000 Stepan Roh\nsee license.txt for copying\n");
      fprintf (stderr, "Compiled-in features: ");
#ifdef PTHREADS
      fprintf (stderr, "THREADS ");
#endif
      fprintf (stderr, "\n\nusage: %s [-f config_file]\n", prog_loc);
      exit (EXIT_SUCCESS);
    } else if (!strcmp (*argv, "-f") && *(argv+1)) {
      cfg_file = *(++argv);
    }
    argv++;
  }

  { int ret = load_config (cfg_file);
    if (ret == -1) {
      perror ("Error loading configuration or MIME types file");
      exit (EXIT_FAILURE);
    } else if (ret == -2) {
      fprintf (stderr, "Syntax error in configuration file\n");
      exit (EXIT_FAILURE);
    }
  }

  /* hlavni socket */
  if ((main_socket = make_socket (server_config.port)) < 0) {
    perror ("Error making main socket");
    exit (EXIT_FAILURE);
  }
  if (listen (main_socket, server_config.max_threads) < 0) {
    perror ("Error listening on main socket");
    exit (EXIT_FAILURE);
  }

  if (init_threads () < 0) {
#ifdef PTHREADS
    perror ("Error activating threads");
#else
    perror ("Error initialising buffers");
#endif
    exit (EXIT_FAILURE);
  }

#ifdef PTHREADS
  main_thread = pthread_self ();
#endif

  /* registrace signal handleru */
  signal (SIGINT, signal_handler);
  signal (SIGHUP, signal_handler);
  signal (SIGTERM, signal_handler);
  signal (SIGPIPE, SIG_IGN);

  log_printf (ERROR_LOG, LOG_LEVEL_SYS, "shttpd started properly\n");
  
  /* hlavni smycka */
  while (1) {
    socklen_t sock, size; struct sockaddr_in client;

    size = sizeof (client);
    sock = accept (main_socket, (struct sockaddr *) &client, &size);
    if (sock < 0) {
      perror ("Error accepting on main socket");
      main_exit (EXIT_FAILURE);
    }
    handle_request_thread (client, sock);	/* uzavre socket, je-li to nutne */
  }

  main_exit (EXIT_SUCCESS);

  return 0; /* sem nikdy nedorazi */
}