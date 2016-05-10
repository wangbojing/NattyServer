/*
 *  Author : WangBoJing , email : 1989wangbojing@gmail.com
 * 
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of NALEX Inc. (C) 2016
 * 
 *
 
****       *****
  ***        *
  ***        *                         *               *
  * **       *                         *               *
  * **       *                         *               *
  *  **      *                        **              **
  *  **      *                       ***             ***
  *   **     *       ******       ***********     ***********    *****    *****
  *   **     *     **     **          **              **           **      **
  *    **    *    **       **         **              **           **      *
  *    **    *    **       **         **              **            *      *
  *     **   *    **       **         **              **            **     *
  *     **   *            ***         **              **             *    *
  *      **  *       ***** **         **              **             **   *
  *      **  *     ***     **         **              **             **   *
  *       ** *    **       **         **              **              *  *
  *       ** *   **        **         **              **              ** *
  *        ***   **        **         **              **               * *
  *        ***   **        **         **     *        **     *         **
  *         **   **        **  *      **     *        **     *         **
  *         **    **     ****  *       **   *          **   *          *
*****        *     ******   ***         ****            ****           *
                                                                       *
                                                                      *
                                                                  *****
                                                                  ****


 *
 */




#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <event.h>
#include <signal.h>

#include "NattyWorkQueue.h"

/* Port to listen on. */
#define SERVER_PORT 5555
/* Connection backlog (# of backlogged connections to accept). */
#define CONNECTION_BACKLOG 8
/* Socket read and write timeouts, in seconds. */
#define SOCKET_READ_TIMEOUT_SECONDS 10
#define SOCKET_WRITE_TIMEOUT_SECONDS 10
/* Number of worker threads.  Should match number of CPU cores reported in /proc/cpuinfo. */
#define NUM_THREADS 8

/* Behaves similarly to fprintf(stderr, ...), but adds file, line, and function
 information. */
#define errorOut(...) {\
	fprintf(stderr, "%s:%d: %s():\t", __FILE__, __LINE__, __FUNCTION__);\
	fprintf(stderr, __VA_ARGS__);\
}

/**
 * Struct to carry around connection (client)-specific data.
 */
typedef struct client {
	/* The client's socket. */
	int fd;

	/* The event_base for this client. */
	struct event_base *evbase;

	/* The bufferedevent for this client. */
	struct bufferevent *buf_ev;

	/* The output buffer for this client. */
	struct evbuffer *output_buffer;

	/* Here you can add your own application-specific attributes which
	 * are connection-specific. */
} client_t;

static struct event_base *evbase_accept;
static workqueue_t workqueue;

/* Signal handler function (defined below). */
static void sighandler(int signal);

/**
 * Set a socket to non-blocking mode.
 */
static int setnonblock(int fd) {
	int flags;

	flags = fcntl(fd, F_GETFL);
	if (flags < 0) return flags;
	flags |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0) return -1;
	return 0;
}

static void closeClient(client_t *client) {
	if (client != NULL) {
		if (client->fd >= 0) {
			close(client->fd);
			client->fd = -1;
		}
	}
}

static void closeAndFreeClient(client_t *client) {
	if (client != NULL) {
		closeClient(client);
		if (client->buf_ev != NULL) {
			bufferevent_free(client->buf_ev);
			client->buf_ev = NULL;
		}
		if (client->evbase != NULL) {
			event_base_free(client->evbase);
			client->evbase = NULL;
		}
		if (client->output_buffer != NULL) {
			evbuffer_free(client->output_buffer);
			client->output_buffer = NULL;
		}
		free(client);
	}
}









/**
 * Called by libevent when there is data to read.
 */
void buffered_on_read(struct bufferevent *bev, void *arg) {
	client_t *client = (client_t *)arg;
	char data[4096];
	int nbytes;

	/* Copy the data from the input buffer to the output buffer in 4096-byte chunks.
	 * There is a one-liner to do the whole thing in one shot, but the purpose of this server
	 * is to show actual real-world reading and writing of the input and output buffers,
	 * so we won't take that shortcut here. */
	while ((nbytes = EVBUFFER_LENGTH(bev->input)) > 0) {
		/* Remove a chunk of data from the input buffer, copying it into our local array (data). */
		if (nbytes > 4096) nbytes = 4096;
		evbuffer_remove(bev->input, data, nbytes); 
		/* Add the chunk of data from our local array (data) to the client's output buffer. */
		evbuffer_add(client->output_buffer, data, nbytes);
	}

	/* Send the results to the client.  This actually only queues the results for sending.
	 * Sending will occur asynchronously, handled by libevent. */
	if (bufferevent_write_buffer(bev, client->output_buffer)) {
		errorOut("Error sending data to client on fd %d\n", client->fd);
		closeClient(client);
	}
}

/**
 * Called by libevent when the write buffer reaches 0.  We only
 * provide this because libevent expects it, but we don't use it.
 */
void buffered_on_write(struct bufferevent *bev, void *arg) {
}

/**
 * Called by libevent when there is an error on the underlying socket
 * descriptor.
 */
void buffered_on_error(struct bufferevent *bev, short what, void *arg) {
	closeClient((client_t *)arg);
}

static void server_job_function(struct job *job) {
	client_t *client = (client_t *)job->user_data;

	event_base_dispatch(client->evbase);
	closeAndFreeClient(client);
	free(job);
}

/**
 * This function will be called by libevent when there is a connection
 * ready to be accepted.
 */
void on_accept(int fd, short ev, void *arg) {
	int client_fd;
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	workqueue_t *workqueue = (workqueue_t *)arg;
	client_t *client;
	job_t *job;

	client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_len);
	if (client_fd < 0) {
		warn("accept failed");
		return;
	}

	/* Set the client socket to non-blocking mode. */
	if (setnonblock(client_fd) < 0) {
		warn("failed to set client socket to non-blocking");
		close(client_fd);
		return;
	}

	/* Create a client object. */
	if ((client = malloc(sizeof(*client))) == NULL) {
		warn("failed to allocate memory for client state");
		close(client_fd);
		return;
	}
	memset(client, 0, sizeof(*client));
	client->fd = client_fd;

	/* Add any custom code anywhere from here to the end of this function
	 * to initialize your application-specific attributes in the client struct. */

	if ((client->output_buffer = evbuffer_new()) == NULL) {
		warn("client output buffer allocation failed");
		closeAndFreeClient(client);
		return;
	}

	if ((client->evbase = event_base_new()) == NULL) {
		warn("client event_base creation failed");
		closeAndFreeClient(client);
		return;
	}

	/* Create the buffered event.
	 *
	 * The first argument is the file descriptor that will trigger
	 * the events, in this case the clients socket.
	 *
	 * The second argument is the callback that will be called
	 * when data has been read from the socket and is available to
	 * the application.
	 *
	 * The third argument is a callback to a function that will be
	 * called when the write buffer has reached a low watermark.
	 * That usually means that when the write buffer is 0 length,
	 * this callback will be called.  It must be defined, but you
	 * don't actually have to do anything in this callback.
	 *
	 * The fourth argument is a callback that will be called when
	 * there is a socket error.  This is where you will detect
	 * that the client disconnected or other socket errors.
	 *
	 * The fifth and final argument is to store an argument in
	 * that will be passed to the callbacks.  We store the client
	 * object here.
	 */
	if ((client->buf_ev = bufferevent_new(client_fd, buffered_on_read, buffered_on_write, buffered_on_error, client)) == NULL) {
		warn("client bufferevent creation failed");
		closeAndFreeClient(client);
		return;
	}
	bufferevent_base_set(client->evbase, client->buf_ev);

	bufferevent_settimeout(client->buf_ev, SOCKET_READ_TIMEOUT_SECONDS, SOCKET_WRITE_TIMEOUT_SECONDS);

	/* We have to enable it before our callbacks will be
	 * called. */
	bufferevent_enable(client->buf_ev, EV_READ);

	/* Create a job object and add it to the work queue. */
	if ((job = malloc(sizeof(*job))) == NULL) {
		warn("failed to allocate memory for job state");
		closeAndFreeClient(client);
		return;
	}
	job->job_function = server_job_function;
	job->user_data = client;

	workqueue_add_job(workqueue, job);
}

/**
 * Run the server.  This function blocks, only returning when the server has terminated.
 */
int runServer(void) {
	int listenfd;
	struct sockaddr_in listen_addr;
	struct event ev_accept;
	int reuseaddr_on;

	/* Initialize libevent. */
	event_init();

	/* Set signal handlers */
	sigset_t sigset;
	sigemptyset(&sigset);
	struct sigaction siginfo = {
		.sa_handler = sighandler,
		.sa_mask = sigset,
		.sa_flags = SA_RESTART,
	};
	sigaction(SIGINT, &siginfo, NULL);
	sigaction(SIGTERM, &siginfo, NULL);

	/* Create our listening socket. */
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0) {
		err(1, "listen failed");
	}
	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = INADDR_ANY;
	listen_addr.sin_port = htons(SERVER_PORT);
	if (bind(listenfd, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) < 0) {
		err(1, "bind failed");
	}
	if (listen(listenfd, CONNECTION_BACKLOG) < 0) {
		err(1, "listen failed");
	}
	reuseaddr_on = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_on, sizeof(reuseaddr_on));

	/* Set the socket to non-blocking, this is essential in event
	 * based programming with libevent. */
	if (setnonblock(listenfd) < 0) {
		err(1, "failed to set server socket to non-blocking");
	}

	if ((evbase_accept = event_base_new()) == NULL) {
		perror("Unable to create socket accept event base");
		close(listenfd);
		return 1;
	}

	/* Initialize work queue. */
	if (workqueue_init(&workqueue, NUM_THREADS)) {
		perror("Failed to create work queue");
		close(listenfd);
		workqueue_shutdown(&workqueue);
		return 1;
	}

	/* We now have a listening socket, we create a read event to
	 * be notified when a client connects. */
	event_set(&ev_accept, listenfd, EV_READ|EV_PERSIST, on_accept, (void *)&workqueue);
	event_base_set(evbase_accept, &ev_accept);
	event_add(&ev_accept, NULL);

	printf("Server running.\n");

	/* Start the event loop. */
	event_base_dispatch(evbase_accept);

	event_base_free(evbase_accept);
	evbase_accept = NULL;

	close(listenfd);

	printf("Server shutdown.\n");

	return 0;
}

/**
 * Kill the server.  This function can be called from another thread to kill the
 * server, causing runServer() to return.
 */
void killServer(void) {
	fprintf(stdout, "Stopping socket listener event loop.\n");
	if (event_base_loopexit(evbase_accept, NULL)) {
		perror("Error shutting down server");
	}
	fprintf(stdout, "Stopping workers.\n");
	workqueue_shutdown(&workqueue);
}

static void sighandler(int signal) {
	fprintf(stdout, "Received signal %d: %s.  Shutting down.\n", signal, strsignal(signal));
	killServer();
}

/* Main function for demonstrating the echo server.
 * You can remove this and simply call runServer() from your application. */
int main(int argc, char *argv[]) {
	ntyDisplay();

	return runServer();
}
