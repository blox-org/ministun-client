/*
 * ministun.c
 * Part of the ministun package
 *
 * STUN support code borrowed from Asterisk -- An open source telephony toolkit.
 * Copyright (C) 1999 - 2006, Digium, Inc.
 * Mark Spencer <markster@digium.com>
 * Standalone remake (c) 2009 Vladislav Grishenko <themiron@mail.ru>
 *
 * See http://www.asterisk.org for more information about
 * the Asterisk project. Please do not directly contact
 * any of the maintainers of this project for assistance;
 * the project provides a web site, mailing lists and IRC
 * channels for your use.
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License Version 2. See the LICENSE file
 * at the top of the source tree.
 * 
 * This code provides some support for doing STUN transactions.
 * Eventually it should be moved elsewhere as other protocols
 * than RTP can benefit from it - e.g. SIP.
 * STUN is described in RFC3489 and it is based on the exchange
 * of UDP packets between a client and one or more servers to
 * determine the externally visible address (and port) of the client
 * once it has gone through the NAT boxes that connect it to the
 * outside.
 * The simplest request packet is just the header defined in
 * struct stun_header, and from the response we may just look at
 * one attribute, STUN_MAPPED_ADDRESS, that we find in the response.
 * By doing more transactions with different server addresses we
 * may determine more about the behaviour of the NAT boxes, of
 * course - the details are in the RFC.
 *
 * All STUN packets start with a simple header made of a type,
 * length (excluding the header) and a 16-byte random transaction id.
 * Following the header we may have zero or more attributes, each
 * structured as a type, length and a value (whose format depends
 * on the type, but often contains addresses).
 * Of course all fields are in network format.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#include "ministun.h"


int stunport = STUN_PORT;
int localport = 0 ;
char *stunserver = STUN_SERVER;
static int stuncount = STUN_COUNT;
static int stundebug = 0;

static void usage(char *name)
{
	fprintf(stderr, "Minimalistic STUN client ver.%s\n", VERSION);
	fprintf(stderr, "Usage: %s [-p port] [-l localport] [-c count] [-d] stun_server [local_address]\n", PACKAGE);
}


int main(int argc, char *argv[])
{
	int sock, opt, res;
	struct sockaddr_in server,client,mapped;
	struct hostent *hostinfo;

	while ((opt = getopt(argc, argv, "p:c:t:l:dh")) != -1) {
		switch (opt) {
			case 'l':
				localport = atoi(optarg);
				break;
			case 'p':
				stunport = atoi(optarg);
				break;
			case 'c':
				stuncount = atoi(optarg);
				break;
			case 'd':
				stundebug++;
				break;
			default:
				usage(argv[0]);
				return -1;
			}
	}

	if (optind < argc)
		stunserver = argv[optind++];

	hostinfo = gethostbyname(stunserver);
	if (!hostinfo) {
		fprintf(stderr, "Error resolving host %s\n", stunserver);
		return -1;
	}
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr = *(struct in_addr*) hostinfo->h_addr;
	server.sin_port = htons(stunport);

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if( sock < 0 ) {
		fprintf(stderr, "Error creating socket\n");
		return -1;
	}

	bzero(&client, sizeof(client));
	client.sin_family = AF_INET;
	if(argv[optind]) {
		hostinfo = gethostbyname(argv[optind]);
		client.sin_addr = *(struct in_addr*) hostinfo->h_addr;
	} else {
		client.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	client.sin_port = htons(localport) ;
	if (bind(sock, (struct sockaddr*)&client, sizeof(client)) < 0) {
		fprintf(stderr, "Error bind to socket\n");
		close(sock);
		return -1;
	}
	res = stun_request(sock, &server, NULL, &mapped);

	if (!res && (mapped.sin_addr.s_addr != htonl(INADDR_ANY)))
		printf("%s:%d\n",inet_ntoa(mapped.sin_addr),mapped.sin_port);

	close(sock);
	return res;
}
