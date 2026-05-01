#include "types.h"
#include "user.h"
#include "socket.h"

int
main (int argc, char *argv[])
{
    if (fork() == 0) {
        printf(1, "ifconfig command 1 in ns %d\n", getnsid());
        char* args[] = { "ifconfig", "net1", "172.16.101.2/24", 0 };
        printf(1, "in child check ns is %d\n", getnsid());
        exec("ifconfig", args);
        exit();
    }
    
    wait();
    
    if (fork() == 0) {
        printf(1, "ifconfig command 2 in ns %d\n", getnsid());
        char* args[] = { "ifconfig", "net1", "up", nullptr };
        printf(1, "in child check ns is %d\n", getnsid());
        exec("ifconfig", args);
        exit();
    }

    wait();
    int soc, peerlen, ret;
    struct sockaddr_in self, peer;
    unsigned char *addr;
    char buf[2048];
    
    int nsid = getnsid();
    printf(1, "Starting UDP Echo Server\n");
    soc = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (soc == 1) {
        printf(1, "socket: failure\n");
        exit();
    }
    printf(1, "socket: success, soc=%d\n", soc);
    self.sin_family = AF_INET;
    ip_addr_t ip_addr;
    ip_addr_pton("172.16.101.2", &ip_addr);
    self.sin_addr = ip_addr;
    // self.sin_addr = INADDR_ANY;
    self.sin_port = hton16(7);
    if (bind(soc, (struct sockaddr *)&self, sizeof(self)) == -1) {
        printf(1, "bind: failure\n");
        close(soc);
        exit();
    }
    addr = (unsigned char *)&self.sin_addr;
    printf(1, "bind: success, self=%d.%d.%d.%d:%d\n", addr[0], addr[1], addr[2], addr[3], ntoh16(self.sin_port));
    printf(1, "waiting for message...\n");
    while (1) {
        peerlen = sizeof(peer);
        ret = recvfrom(soc, buf, sizeof(buf), (struct sockaddr *)&peer, &peerlen);
        if (ret <= 0) {
            printf(1, "EOF\n");
            break;
        }
        addr = (unsigned char *)&peer.sin_addr;
        printf(1, "recvfrom: %d bytes data received, peer=%d.%d.%d.%d:%d\n", ret, addr[0], addr[1], addr[2], addr[3], ntoh16(peer.sin_port));
        hexdump(buf, ret);
        sendto(soc, buf, ret, (struct sockaddr *)&peer, peerlen);
    }
    close(soc);  
    exit();
}
