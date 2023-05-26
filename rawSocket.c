#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);


    close(raw_socket);

    return 0;
}
