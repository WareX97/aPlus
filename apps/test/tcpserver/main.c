#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


#define htons(x)  __builtin_bswap16(x)

int main(int argc, char** argv) {
    struct sockaddr_in s;
    memset(&s, 0, sizeof(s));

    s.sin_family = AF_INET;
    s.sin_port = htons(5000);
    s.sin_addr.s_addr = INADDR_ANY;

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0) {
        perror("socket");
        return -1;
    }

    if(bind(fd, (struct sockaddr*) &s, sizeof(s)) < 0) {
        perror("bind");
        return -1;
    }

    listen(fd, 5);
    
    int cfd;
    struct sockaddr_in c;
    socklen_t clen = sizeof(c);

    while((cfd = accept(fd, (struct sockaddr*) &c, &clen)) >= 0) {
        fprintf(stderr, "tcpserver: Accepted new client on: %d.%d.%d.%d:%d\n", (c.sin_addr.s_addr & 0xFF000000) >> 24, (c.sin_addr.s_addr & 0x00FF0000) >> 16, (c.sin_addr.s_addr & 0x0000FF00) >> 8, (c.sin_addr.s_addr & 0x000000FF), htons(c.sin_port));
        close(cfd);
    }

    return 0;
}