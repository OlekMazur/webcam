/*
 * This file is part of webcam.
 *
 * Copyright (c) 2023 Aleksander Mazur
 *
 * webcam is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * webcam is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with webcam. If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "vfo_http.h"
#include "vfo_cgi.h"

/**
 * @addtogroup vfo_http
 * @{
 */

/**************************************/

/**
 * Creates a socket for listening on given TCP port.
 *
 * @param port TCP port number.
 * @return Open socket, or -1 on error.
 */
static int create_server_socket(unsigned short port)
{
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sock >= 0) {
		do {
			struct sockaddr_in addr;
			socklen_t len;

			memset(&addr, 0, sizeof(addr));
			addr.sin_family = AF_INET;
			addr.sin_port = htons(port);

			if (bind(sock, (const struct sockaddr *) &addr, sizeof(addr)))
				break;

			len = sizeof(addr);
			if (getsockname(sock, (struct sockaddr *) &addr, &len))
				break;

			if (listen(sock, 1))
				break;

			return sock;
		} while (0);

		close(sock);
	}

	return -1;
}

/**
 * Reads and parses HTTP query on given input.
 *
 * Expected query is prefixed by "GET /".
 *
 * @param input Input file where HTTP query is expected.
 * @return 0 on success, other value if input doesn't match the expected query.
 */
static int get_query(FILE *input)
{
	static char expected_query_pfx[] = "GET /";
	char buf[128];

	do {
		if (!fgets(buf, sizeof(buf), input))
			break;
		if (strncmp(buf, expected_query_pfx, sizeof(expected_query_pfx) - 1))
			break;
		while (fgets(buf, sizeof(buf), input)) {
			char *p;

			for (p = buf; *p == '\n' || *p == '\r'; p++)
				;
			if (!*p)
				break;
		}
		return 0;
	} while (0);
	return -1;
}

/**************************************/

video_frame_output_t *video_frame_output_http_init(unsigned short port)
{
	int server = create_server_socket(port);
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	int sock;
	FILE *output;

	if (server < 0) {
		perror("socket");
		return NULL;
	}
	sock = accept(server, (struct sockaddr *) &addr, &len);
	close(server);
	if (sock < 0) {
		perror("accept");
		return NULL;
	}
	if (len >= sizeof(addr)) {
		fprintf(stderr, "Connection from %s:%hu\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
	}
	output = fdopen(sock, "r+");
	if (get_query(output)) {
		return NULL;
	}
	return video_frame_output_cgi_init(output);
}

/**
 * @}
 */
