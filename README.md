# TCP/UDP Epoll server

TCP/UDP server that handles connections in a single thread

## Dependencies

- gcc
- make
- linux libraries

## Run server

```bash
make run_server
```

## Test server

### Run tcp client

```bash
make run_tcp_client
```

### Run udp client

```bash
make run_udp_client
```

### Run stress (multiple tcp clients at once)

```bash
make run_stress
```

