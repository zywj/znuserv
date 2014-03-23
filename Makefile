CFLAGS=-g -Wall
LDFLAGS=-lpthread -llua5.2 -lm
PFLAGS=-I src
#LDFLAGS = -fPIC -shared

all: serv

serv: src/znuserv.c \
	 src/zs_event.c \
	 src/zs_string.c \
	 src/zs_config.c \
	 src/zs_deamon.c \
	 src/zs_palloc.c \
	 src/zs_context.c \
	 src/zs_socket.c \
	 src/zs_timer.c  \
	 src/zs_process.c \
	 src/zs_cache.c  \
	 src/zs_php.c \
	 src/zs_http.c
	 
	gcc $(CFLAGS) -o $@  $^ $(LDFLAGS) $(PFLAGS)

clean:
	rm serv 
