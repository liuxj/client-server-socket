#
# Authors: Xiangjun Liu <xiangjun.liu@archermind.com>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version
# 3 of the License, or (at your option) any later version.
#

CLIENT_OBJS = client.o
SERVER_OBJS = server.o

all: client server

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(CLIENT_OBJS) $(SERVER_OBJS) server client

