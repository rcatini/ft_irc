# ft_irc

## Specification
The project is a IRC server, as specified in RFC 1459 and RFC 2812.
A modern version of the IRC protocol is also defined at [IRCv3](https://ircv3.net/irc/).

## Compilation
The project can be compiled with the following command:

```
make
```

If a specific compiler is desired, it can be specified with the `CXX` variable:

```
make CXX=clang++
```

## Usage
A server can be started passing as parameters the port and the password, as in this example:

```
./ircserv 6667 password
```

For now, the server is not compatible with IRC clients.

It can be tested with netcat with the following options:

- `-N` to shutdown the network after EOF on input
- `-C` to send CRLF as line-ending

Example:
```
nc -NC localhost 6667
```

Multiple clients can be connected to the server, and they can be used to test the server (any message is broadcast to all clients).