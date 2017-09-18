#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>

#define MAXEVENTS 64

char INDEX_DIR[512];
char PORT[8];
char* RESPONSE_HEADER = "HTTP/1.0 200 OK\nContent-Type: text/html\n\n";

int load_configuration(char* filename) {
    FILE* fp;

    fp = fopen(filename, "r"); // read mode

    if( fp == NULL ) {
      perror("Error while opening the file.\n");
      exit(EXIT_FAILURE);
    }
    fscanf(fp, "%s", PORT);
    fscanf(fp, "%s", INDEX_DIR);

    fclose(fp);
    return 0;
}

int make_socket_non_blocking (int sockfd) {
/*  Set the unblocking flag to the socket file descriptor
*/
    int flags, s;

    flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        perror ("fcntl");
        return -1;
    }

    flags |= O_NONBLOCK; //Add non-blocking flag
    s = fcntl(sockfd, F_SETFL, flags);
    if (s == -1) {
        perror ("fcntl");
        return -1;
    }

    return 0;
}

int create_and_bind (char *port) {
/*  Create a new socket and bind it to a certain port.
*/
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s, sockfd;

    memset (&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */
    hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */
    hints.ai_flags = AI_PASSIVE;     /* All interfaces */

    s = getaddrinfo (NULL, port, &hints, &result);
    if (s != 0) {
        fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (s));
        return -1;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sockfd = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sockfd == -1)
            continue;

        s = bind(sockfd, rp->ai_addr, rp->ai_addrlen);
        if (s == 0) {
            /* We managed to bind successfully! */
            break;
        }

        close (sockfd);
    }

    if (rp == NULL) {
        fprintf (stderr, "Could not bind\n");
        return -1;
    }

    freeaddrinfo (result);

    return sockfd;
}

void handle_new_connection(int sockfd, int epollfd) {
/* We have a notification on the listening socket, which
   means one or more incoming connections. */
    struct epoll_event event;
    int s;

    while (1) {
        struct sockaddr in_addr;
        socklen_t in_len;
        int infd;
        char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

        in_len = sizeof in_addr;
        infd = accept (sockfd, &in_addr, &in_len);
        if (infd == -1) {
          if ((errno == EAGAIN) ||
              (errno == EWOULDBLOCK)) {
              /* We have processed all incoming
                 connections. */
              break;
            } else {
              perror ("accept");
              break;
            }
        }

        s = getnameinfo (&in_addr, in_len,
                       hbuf, sizeof hbuf,
                       sbuf, sizeof sbuf,
                       NI_NUMERICHOST | NI_NUMERICSERV);
        if (s == 0) {
            printf("Accepted connection on descriptor %d "
                   "(host=%s, port=%s)\n", infd, hbuf, sbuf);
        }

        /* Make the incoming socket non-blocking and add it to the
         list of fds to monitor. */
        s = make_socket_non_blocking (infd);
        if (s == -1)
            abort ();

        event.data.fd = infd;
        event.events = EPOLLIN | EPOLLET;
        s = epoll_ctl (epollfd, EPOLL_CTL_ADD, infd, &event);
        if (s == -1) {
            perror ("epoll_ctl");
            abort ();
        }
    }
}

char* find_request_path(char* buff) {
/*  Get HTTP Request path from a HTTP Request file.
    TODO: handle if path contains webserver name.
*/
    char* path_begin = strchr(buff, '/');
    char* path_end = strchr(path_begin, ' ');
    *path_end = 0;

    return path_begin;
}

int main (int argc, char *argv[]) {
    int sockfd, s;
    int epollfd;
    struct epoll_event event;
    struct epoll_event *events;

    load_configuration("resources/server.conf");

    // sockfd = create_and_bind (PORT);
    sockfd = create_and_bind (argv[1]);
    if (sockfd == -1)
        abort ();

    s = make_socket_non_blocking (sockfd);
    if (s == -1)
        abort ();

    s = listen (sockfd, SOMAXCONN);
    if (s == -1) {
        perror ("listen");
        abort ();
    }

    // Create epoll instance
    epollfd = epoll_create1 (0);
    if (epollfd == -1) {
        perror ("epoll_create");
        abort ();
    }

    event.data.fd = sockfd;
    event.events = EPOLLIN | EPOLLET;

    //Add event on filedescriptor sockfd on epollfd epollinstance
    s = epoll_ctl (epollfd, EPOLL_CTL_ADD, sockfd, &event);
    if (s == -1) {
        perror ("epoll_ctl");
        abort ();
    }

    /* Buffer where events are returned */
    events = (struct epoll_event*) calloc (MAXEVENTS, sizeof event);

  /* The event loop */
    while (1){
        int n, i;
        printf("epoll waiting\n");
        n = epoll_wait (epollfd, events, MAXEVENTS, -1);
        for (i = 0; i < n; i++) {
            if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) ||
                (!(events[i].events & EPOLLIN))) {
                  /* An error has occured on this fd, or the socket is not
                     ready for reading (why were we notified then?) */
                fprintf (stderr, "epoll error\n");
                close (events[i].data.fd);
                continue;
            } else if (sockfd == events[i].data.fd) {
                 handle_new_connection(sockfd, epollfd);
            } else {
                /* We have data on the fd waiting to be read. Read and
                 display it. We must read whatever data is available
                 completely, as we are running in edge-triggered mode
                 and won't get a notification again for the same
                 data. */
                int done = 0;
                while (1) {
                    ssize_t count;
                    char buf[512];
                    memset(buf, 0, 512);
                    printf("incoming data\n");
                    count = read (events[i].data.fd, buf, sizeof buf);
                    if (count == -1) {
                      /* If errno == EAGAIN, that means we have read all
                         data. So go back to the main loop. */
                        if (errno != EAGAIN) {
                            perror ("read");
                            done = 1;
                        }
                        break;
                    } else if (count == 0) {
                      /* End of file. The remote has closed the
                         connection. */
                        done = 1;
                        break;
                    }

                    /* Write the buffer to standard output */
                    printf("'%s'\n", buf);
                    char sendbuff[768];
                    strcpy(sendbuff, RESPONSE_HEADER);
                    strcat(sendbuff, buf);
                    send(events[i].data.fd, buf, count, 0);
                    // char* path = find_request_path(buf);
                    // printf("'%s'\n", path);
                    close(events[i].data.fd);
                    break;
                    if (s == -1) {
                       perror ("write");
                       abort ();
                    }
                }

                if (done) {
                      printf ("Closed connection on descriptor %d\n",
                              events[i].data.fd);

                      /* Closing the descriptor will make epoll remove it
                         from the set of descriptors which are monitored. */
                      close (events[i].data.fd);
                }
            }
        }
    }

    free (events);
    close (sockfd);

    return EXIT_SUCCESS;
}
