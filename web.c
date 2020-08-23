#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<string.h>
#include<sys/stat.h>
#include<fcntl.h>
#include <sys/sendfile.h>

char webpage[] = 
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<html><head><title>Demo page</title>\r\n"
"<style>body { background-color: #FFFF00 }</style>"
"</head>\r\n<body><center><h1>Hello World!!</h1><br>\r\n"
"<img src= \"doctest.jpg\" ></center></body></html>\r\n";


int main(int argc, char *argv[])
{
	struct sockaddr_in server_addr, client_addr;
	
	/*
	The structure sockaddr is a generic container that just allows the OS to be able to read the first couple of bytes that identify the address family. The address family determines what variant of the sockaddr struct to use that contains elements that make sense for that specific communication type. For IP networking, we use struct sockaddr_in, which is defined in the header netinet/in.h. This structure defines:
	
	struct sockaddr_in 
	{ 
		__uint8_t         sin_len; 
		sa_family_t       sin_family; 
		in_port_t         sin_port; 
		struct in_addr    sin_addr; 
		char              sin_zero[8]; 
	};
	
	sin_family :
		The address family we used when we set up the socket. In our case, it’s AF_INET.
		AF_INET is an address family that is used to designate the type of addresses that your socket can communicate with (in this case, Internet Protocol v4 addresses). When you create a socket, you have to specify its address family, and then you can only use addresses of that type with the socket.
	sin_port:
		The port number (the transport address). You can explicitly assign a transport address (port) or allow the operating system to assign one. If you’re a client and won’t be receiving incoming connections, you’ll usually just let the operating system pick any available port number by specifying port 0. If you’re a server, you’ll generally pick a specific number since clients will need to know a port number to connect to.
	sin_addr:
		The address for this socket. This is just your machine’s IP address. With IP, your machine will have one IP address for each network interface. For example, if your machine has both Wi-Fi and ethernet connections, that machine will have two addresses, one for each interface. Most of the time, we don’t care to specify a specific interface and can let the operating system use whatever it wants. The special address for this is 0.0.0.0, defined by the symbolic constant INADDR_ANY.
		Since the address structure may differ based on the type of transport used, the third parameter specifies the length of that structure. This is simply sizeof(struct sockaddr_in).
	*/
	
	socklen_t sin_len = sizeof(client_addr);
	int fd_server , fd_client;
	char buf[2048];
	int fdimg;
	int on = 1;
	
	fd_server = socket(AF_INET, SOCK_STREAM, 0);
	/*
	
	All the parameters as well as the return value are integers:
	domain, or address family — 
		communication domain in which the socket should be created. Some of address families are AF_INET (IP), AF_INET6 (IPv6), AF_UNIX (local channel, similar to pipes), AF_ISO (ISO protocols), and AF_NS (Xerox Network Systems protocols).
	
	type —
		type of service. This is selected according to the properties required by the application: SOCK_STREAM (virtual circuit service), SOCK_DGRAM (datagram service), SOCK_RAW (direct IP service). Check with your address family to see whether a particular service is available.
	
	protocol —
		indicate a specific protocol to use in supporting the sockets operation. This is useful in cases where some families may have more than one protocol to support a given type of service. The return value is a file descriptor (a small integer). The analogy of creating a socket is that of requesting a telephone line from the phone company.
		
	***For TCP/IP sockets, we want to specify the IP address family (AF_INET) and virtual circuit service (SOCK_STREAM). Since there’s only one form of virtual circuit service, there are no variations of the protocol, so the last argument, protocol, is zero. 
	
		
	*/
	if(fd_server < 0)
	{
		perror("socket");
		exit(1);
	}
	
	setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
	
	/*
	When retrieving a socket option, or setting it, you specify the option name as well as the level. When level = SOL_SOCKET, the item will be searched for in the socket itself.

	For example, suppose we want to set the socket option to reuse the address to 1 (on/true), we pass in the "level" SOL_SOCKET and the value we want it set to.

	int value = 1;    
	setsockopt(mysocket, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));
	This will set the SO_REUSEADDR in my socket to 1.
	
	*/
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY; 
	// INADDR_ANY is used when you don't need to bind a socket to a specific IP
	// 0.0.0.0, defined by the symbolic constant INADDR_ANY.
	
	server_addr.sin_port = htons(8080);
	/*
	The htons function takes a 16-bit number in host byte order and returns a 16-bit number in network byte order used in TCP/IP networks (the AF_INET or AF_INET6 address family). 
	*/
	
	
	if( bind(fd_server, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1 )
	{
		perror("bind");
		close(fd_server);
		exit(1);
	}
	/*
	The system call for bind is:
		int bind(int socket, const struct sockaddr *address, socklen_t address_len);
	The first parameter, socket,
		is the socket that was created with the socket system call.
	For the second parameter, 
		the structure sockaddr is a generic container that just allows the OS to be able to read the first couple of bytes that identify the address family. The address family determines what variant of the sockaddr struct to use that contains elements that make sense for that specific communication type. For IP networking, we use struct sockaddr_in, which is defined in the header netinet/in.h. This structure defines:
	
	Before calling bind, we need to fill out this structure. The three key parts we need to set are:
	sin_family
		The address family we used when we set up the socket. In our case, it’s AF_INET.
	sin_port
		The port number (the transport address). You can explicitly assign a transport address (port) or allow the operating system to assign one. If you’re a client and won’t be receiving incoming connections, you’ll usually just let the operating system pick any available port number by specifying port 0. If you’re a server, you’ll generally pick a specific number since clients will need to know a port number to connect to.
	sin_addr
		The address for this socket. This is just your machine’s IP address. With IP, your machine will have one IP address for each network interface. For example, if your machine has both Wi-Fi and ethernet connections, that machine will have two addresses, one for each interface. Most of the time, we don’t care to specify a specific interface and can let the operating system use whatever it wants. The special address for this is 0.0.0.0, defined by the symbolic constant INADDR_ANY.
	Since the address structure may differ based on the type of transport used, the third parameter specifies the length of that structure. This is simply sizeof(struct sockaddr_in).
	
	*/
		
	if(listen(fd_server, 10) == -1)
		/*
	The listen system call tells a socket that it should be capable of accepting incoming connections
	The second parameter, backlog, defines the maximum number of pending connections that can be queued up before connections are refused.
	*/
	{
		perror("listen");
		close(fd_server);
		exit(1);
	}
	
	while(1)
	{
		fd_client = accept(fd_server, (struct sockaddr *) &client_addr, &sin_len);
		/*
			The accept() system call with the connection-based socket types(SOCK_STREAM, SOCK_SEQPACKET). It extracts the first connection request on queue of pending connections for the listening socket, sockfd, creates a new connected socket, and returns a new file descriptor referring to that socket. The newly created socket is not in the listening state. The original socket sockfd is unaffected by this call.
		
			int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen); 
				sockfd: The argument sockfd is a socket that has been created with socket(), bound to a local address with bind(), and is listening for connections after a listen().
				
				addr: The argument addr is a pointer to a sockaddr structure. This structure is filled in with the address of the peer socket, as known to the communications layer. The exact format of the address returned addr is determined by the socket’s address family. When addr is NULL,
				nothing is filled in; in this case, addrlen is not used, and should also be NULL.
				
				addrlen: The addrlen argument is a value-result argument: the caller must initialize it to contain the size (in bytes) of the structure pointed to by addr; on return it will contain the actual size of the peer address.
				
				Returns: On success, these system calls return a non-negative integer that is a file descriptor for the accepted socket. On error, -1 is returned, and errno is set appropriately.
		*/
		
		if(fd_client == -1)
		{
			perror("Connection failed....\n");
			continue;
		}
		
		printf("Got client connection....\n");
		
		if(!fork())
		{
			/* child process */
			close(fd_server);
			memset(buf, 0, 2048);
			read(fd_client, buf, 2047);
			
			printf("%s\n",buf);
			
			if(!strncmp(buf, "GET /favicon.ico", 16))
			{
				fdimg = open("favicon.ico", O_RDONLY);
				sendfile(fd_client, fdimg, NULL, 4000);
				close(fdimg);
			}
			
			else if(!strncmp(buf, "GET /doctest.jpg", 16))
			{
				fdimg = open("doctest.jpg", O_RDONLY);
				sendfile(fd_client, fdimg, NULL, 6000);
				/*
				The sendfile system call is an optimization. If you have a socket sockfd and a regular file filefd and you want to copy some file data to the socket (e.g. if you're a web server serving up a file), then you might write it like this:

				// Error checking omitted for expository purposes
				while(not done)
				{
					char buffer[BUFSIZE];
					int n = read(filefd, buffer, BUFSIZE);
					send(sockfd, buffer, n, 0);
				}
				However, this is inefficient: this involves the kernel copying the file data into userspace (in the read call) and then copying the same data back into kernel space (in the send call).

				The sendfile system call lets us skip all of that copying and have the kernel directly read the file data and send it on the socket in one fell swoop:

				sendfile(sockfd, filefd, NULL, BUFSIZE);
				
				*/
				close(fdimg);
			}
			else
				write(fd_client, webpage, sizeof(webpage)-1);
			
			
			printf("closing...\n");
			exit(0);
			
		}
		/*parent process */
		close(fd_client);
	}
	
	return  0;
}












