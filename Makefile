CC=gcc
FLAGS=-Wall

C_SOURCES=server.c tcp_client.c udp_client.c stress.c
EXES=$(C_SOURCES:.c=)

all: $(EXES)

run_server: server
	./server

run_tcp_client: tcp_client
	./tcp_client

run_udp_client: udp_client
	./udp_client

run_stress: stress
	./stress

%: %.c
	$(CC) $(FLAGS) $< -o $@ 

clean:
	rm $(EXES)
