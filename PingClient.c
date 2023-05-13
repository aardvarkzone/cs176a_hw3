/*
    line 21 taken from https://stackoverflow.com/questions/11405819/does-struct-hostent-have-a-field-h-addr
    all header files, error function, and initial UDP connection from https://www.linuxhowtos.org/data/6/client_udp.c
    referenced slides from class
    buffer population: https://stackoverflow.com/questions/69009464/populating-a-buffer-in-c

*/

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define h_addr h_addr_list[0]

// error function
void error(const char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[]) {
    // ./PingClient, ID address, Port Number = 3 arguments
    if (argc != 3) {
       fprintf(stderr, "usage %s hostname port\n", argv[0]);
       exit(0);
    }

    char* address = argv[1];
    char* port_number = argv[2];

    int sock = 0;
    unsigned int length = 0;
    struct sockaddr_in from;
    struct sockaddr_in server;
    struct hostent *host_name;
    char buffer[1024];
    
    double roundtrip[10];
    // negative value means invalid transmission
    for(int i = 0; i < 10; i++) { roundtrip[i] = -1.0; }

    // UDP socket opener
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0) { error("ERROR opening socket"); }

    struct timeval send_time;
    struct timeval receive_time;
    struct timeval timeout; 


    timeout.tv_usec = 0;
    timeout.tv_sec = 1;
   
    // identify the server
    host_name = gethostbyname(address);
    if(host_name == NULL) { error("ERROR unknown host"); }

    // timeout after 1 second
    int msg = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    if(msg < 0) { error("ERROR setting socket options"); }

    server.sin_family = AF_INET;
    bcopy((char*)host_name->h_addr, (char*)&server.sin_addr, host_name->h_length);
    server.sin_port = htons(atoi(port_number));

    // UDP length requirement
    length = sizeof(struct sockaddr_in);

    for(int i = 1; i <= 10; i++) {

        bzero(buffer, 1024);
        gettimeofday(&send_time, NULL);
        
        sprintf(buffer, "PING %d %ld.%d", i, send_time.tv_sec, send_time.tv_usec);

        msg = sendto(sock, buffer, 1024, 0, (const struct sockaddr *)&server, length);
        if(msg < 0) { error("ERROR sending"); }
        
        bzero(buffer, 1024);
        msg = recvfrom(sock, buffer, 1023, 0, (struct sockaddr *)&from, &length);

        if(msg >= 0) {
            //time 
            gettimeofday(&receive_time, NULL);
            // convert microsecond time difference to milliseconds
            double rtt_sample = (double) (0.001 * (receive_time.tv_usec - send_time.tv_usec));

            if(rtt_sample < 0) { rtt_sample += 1000; }
            roundtrip[i - 1] = rtt_sample;

            printf("PING received from %s: seq#=%d time=%.3f ms\n", 
                argv[1], 
                i, 
                rtt_sample);
        }
        else {
            // timeout --> proceed to next
            printf("Request timeout for seq#=%d\n", i);
            continue;
        }

    }

    // ping stats
    int counter = 0;
    double sum = 0.0;
    double min = 10000000.0;
    double max = -10000000.0;
    for(int i = 0; i < 10; i++) {
        if(roundtrip[i] > 0) {    // if roundtriptime is valid
            if(roundtrip[i] > max) { max = roundtrip[i]; }
            if(roundtrip[i] < min) { min = roundtrip[i]; }
            sum += roundtrip[i];
            counter++;
        }
    }
    double avg = sum / counter; // only averages valid roundtriptimes

    printf("--- ping statistics ---\n");
    printf("10 packets transmitted, %d received, %d%% packet loss rtt min/avg/max = %.3f %.3f %.3f ms\n", 
            counter, 
            (10 - counter) * 10, 
            min, 
            avg, 
            max);

    // close UDP socket
    close(sock);
    return 0;
}

