#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

void RedirectToNetwork(const char *userid) {
  struct addrinfo hints;
  struct addrinfo *result;

  const char *hostname = "statistics.abridgegame.org";
  const char *port = "5432";
    /* Obtain address(es) matching host/port */

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
  hints.ai_flags = 0;
  hints.ai_protocol = 0;          /* Any protocol */

  int s = getaddrinfo(hostname, port, &hints, &result);
  if (s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    exit(EXIT_FAILURE);
  }

  /* getaddrinfo() returns a list of address structures.  Try each
     address until we successfully connect(2).  If socket(2) (or
     connect(2)) fails, we (close the socket and) try the next
     address. */

  for (struct addrinfo *rp = result; rp != NULL; rp = rp->ai_next) {
    int sfd = socket(rp->ai_family, rp->ai_socktype,
                     rp->ai_protocol);
    if (sfd == -1)
      continue;

    if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
      freeaddrinfo(result);           /* No longer needed */
      fcntl(sfd, F_SETFL, O_NONBLOCK);
      char *buffer = malloc(strlen(userid)+3);
      strcpy(buffer, userid);
      buffer[strlen(userid)] = '\n';
      buffer[strlen(userid)+1] = 0;
      send(sfd, buffer, strlen(buffer), 0); // send name first
      dup2(sfd, 1); // success!
      free(buffer);
      return;
    }
    close(sfd);
  }

  /* No address succeeded */
  fprintf(stderr, "Could not connect\n");
  exit(EXIT_FAILURE);
}
