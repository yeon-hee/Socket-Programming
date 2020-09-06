EXCT = proxy_cache
CC = gcc
all :
	 $(CC) -o $(EXCT) thread.c -lcrypto -lpthread

