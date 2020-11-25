/*
Developer: Abdulrahman
Email: AbdulramanAlandijani@my.unt.edu
Student ID: 11118717
Due date: 11-10-2020
Program Discription: TCP segment simulation; the program stimulates the process
of transmitting a message in a 3 way handshake TCP segment on a client-server based network 
communication program.
*/
#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
 
#define BUFLEN 1048//Max length of buffer

//this structure represesnt the segments.
struct header
{
    unsigned short int src;
    unsigned short int des;
    unsigned int seq;
    unsigned int ack;
    unsigned short int res : 4, offset : 6, flags : 6;
    unsigned short int rec;
    unsigned short int cksum;
    unsigned short int ptr;
    unsigned int opt;
    char payload[1024];
}header;//structure tag

enum
{
    FIN = 0x01,
    SYN = 0x02,
    ACK = 0x10,
};

//functions declaration.
void die(char *s);
unsigned short int find_checksum(struct header segment);
void display(struct header segment, FILE* log_file);

int main(int argc, char** argv)
{
    int s_socket, conn_fd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t addr_len;
    struct header segment, ack_segment, close_segment;
    int port_number = atoi(argv[1]);

    time_t t;
    srand((unsigned)time(&t));

    unsigned int client_checksum;
    int recvdata, sentdata;
    char buff[BUFLEN];
    int temp;

    FILE* out_file = fopen("server.out", "w");

///////////////////////////////////////////////////////////////////////////////////////////////

    /* 
        ***********************
        establishing connection
        ***********************
    */

///////////////////////////////////////////////////////////////////////////////////////////////

    //create a TCP socket
    if ((s_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        die("socket");
    }

    //zero out the structure
    bzero(&servaddr, sizeof(servaddr));
    //initialize socket structure
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port_number);

    //bind socket to port
    if ((bind(s_socket, (struct sockaddr *)&servaddr, sizeof(servaddr))) < 0)
    {
        die("bind");
    }

    listen(s_socket, 10);
    printf("Listening for connections on port #%d...\n", port_number);
    fprintf(out_file, "Listening for connections on port #%d...\n", port_number);

    addr_len = sizeof(cliaddr);
    if ((conn_fd = accept(s_socket, (struct sockaddr *)&cliaddr, &addr_len)) < 0)
    {
        die("accept");
    }


    /* 
        **************************
        handle received connection 
        **************************
    */

///////////////////////////////////////////////////////////////////////////////////////////////

    bzero(buff, BUFLEN);
    bzero(&segment, sizeof(segment));
    if((recvdata = read(conn_fd, &segment, sizeof(segment))) < 0)                               //First read
    {
        die("read");
    }

    //if the checksum recieved was the same as calculated.
    client_checksum = 0;
    client_checksum = find_checksum(segment);
    if(client_checksum == segment.cksum)
    {
        // Print contents of segment
        printf("\nConnection segment recieved from the client:\n");
        fprintf(out_file, "\nConnction segment recieved from the client:\n");
        display(segment, out_file);
        printf("\nCalculated checksum: 0X%04X\n", client_checksum);
        fprintf(out_file, "\nCalculated checksum: 0X%04X\n", client_checksum);
    }
    else
    {
        printf("checksum error1\n");
        exit(1);
    }

///////////////////////////////////////////////////////////////////////////////////////////////

    //populate connection granted segment
    segment.src = port_number;
    segment.des = htons(cliaddr.sin_port);
    segment.ack = segment.seq + 1;  //set ACK to client seq num + 1
    segment.seq = rand();                //set seq to random number
    segment.flags = (SYN | ACK);         //set SYN bit and ACK bit flags to 1
    segment.rec = 0;
    segment.cksum = 0;  //compute checksum
    segment.ptr = 0;
    segment.opt = 0;
    segment.offset = sizeof(segment);
    memset(segment.payload, '\0', sizeof(segment.payload));
    segment.cksum = find_checksum(segment);

    printf("\nSending connection granted segment:\n");
    fprintf(out_file, "\nSending connection granted segment:\n");
    display(segment, out_file);

    //sending segment
    if ((sentdata = send(conn_fd, &segment, sizeof(segment), 0)) < 0)                           //first send
    {
        die("connection granted");
    }

///////////////////////////////////////////////////////////////////////////////////////////////

    /*
        *****************************
        handle acknowledgment segment 
        *****************************
    */

///////////////////////////////////////////////////////////////////////////////////////////////

    //receive acknowledgment segment in buffer
    bzero(&ack_segment, sizeof(ack_segment));
    if ((recvdata = read(conn_fd, &ack_segment, sizeof(ack_segment))) < 0)                      //second read
    {
        die("receive Acknowledgment");
    }

    //if the checksum recieved was the same as calculated.
    client_checksum = 0;
    client_checksum = find_checksum(ack_segment);
    if(client_checksum == ack_segment.cksum)
    {
        // Print contents of segment
        printf("\nAcknowledgment segment received:\n");
        fprintf(out_file, "\nAcknowledgment segment received\n");
        display(ack_segment, out_file);
        printf("\nCalculated checksum: 0X%04X\n", client_checksum);
        fprintf(out_file, "\nCalculated checksum: 0X%04X\n", client_checksum);
    }
    else
    {
        printf("checksum error2\n");
        exit(1);
    }
    printf("\n***connection established***\n");
    fprintf(out_file, "\n***connection established***\n");

///////////////////////////////////////////////////////////////////////////////////////////////

    /* 
        **********************
        handle closing request
        ********************** 
    */

///////////////////////////////////////////////////////////////////////////////////////////////

    //receive acknowledgment segment in buffer
    bzero(&close_segment, sizeof(close_segment));
    if ((recvdata = read(conn_fd, &close_segment, sizeof(close_segment))) < 0)              //third read
    {
        die("read");
    }

    unsigned int client_seq = close_segment.seq;

    //if the checksum recieved was the same as calculated.
    client_checksum = 0;
    client_checksum = find_checksum(close_segment);
    if(client_checksum == close_segment.cksum)
    {
        // Print contents of segment
        printf("\nClose segment received:\n");
        fprintf(out_file, "\nClose segment received\n");
        display(close_segment, out_file);
        printf("\nCalculated checksum: 0X%04X\n", client_checksum);
        fprintf(out_file, "\nCalculated checksum: 0X%04X\n", client_checksum);
    }
    else
    {
        printf("checksum error3\n");
        exit(1);
    }

///////////////////////////////////////////////////////////////////////////////////////////////

    //populating closing segment
    close_segment.src = port_number;
    close_segment.des = htons(cliaddr.sin_port);
    close_segment.ack = close_segment.seq + 1;
    close_segment.seq = 512;
    close_segment.flags = ACK;
    close_segment.rec = 0;
    close_segment.cksum = 0;
    close_segment.ptr = 0;
    close_segment.opt = 0;
    close_segment.offset = sizeof(close_segment);
    memset(close_segment.payload, '\0', sizeof(close_segment.payload));
    close_segment.cksum = find_checksum(close_segment);

    printf("\nSending Acknowledgment segment:\n");
    fprintf(out_file, "\nSending Acknowledgment segment:\n");
    display(close_segment, out_file);

    //sending segment
    if ((sentdata = send(conn_fd, &close_segment, sizeof(close_segment), 0)) < 0)               //second send
    {
        die("write");
    }

///////////////////////////////////////////////////////////////////////////////////////////////

    //populating closing segment
    close_segment.src = port_number;
    close_segment.des = htons(cliaddr.sin_port);
    close_segment.ack = client_seq + 1;
    close_segment.seq = 512;
    close_segment.flags = FIN;
    close_segment.rec = 0;
    close_segment.cksum = 0;
    close_segment.ptr = 0;
    close_segment.opt = 0;
    close_segment.offset = sizeof(close_segment);
    memset(close_segment.payload, '\0', sizeof(close_segment.payload));
    close_segment.cksum = find_checksum(close_segment);


    printf("\nSending 2nd Acknowledgment segment:\n");
    fprintf(out_file, "\nSending 2nd Acknowledgment segment:\n");
    display(close_segment, out_file);

    //sending segment
    if ((sentdata = send(conn_fd, &close_segment, sizeof(close_segment), 0)) < 0)               //last send
    {
        die("write");
    }

/////////////////////////////////////////////////////////////////////////////////////////////////

    bzero(&close_segment, sizeof(close_segment));
    if ((recvdata = read(conn_fd, &close_segment, sizeof(close_segment))) < 0)                  //last read
    {
        die("read");
    }

    //if the checksum recieved was the same as calculated.
    client_checksum = 0;
    client_checksum = find_checksum(close_segment);
    if(client_checksum == close_segment.cksum)
    {
        // Print contents of segment
        printf("\nAcknowledgment segment recieved:\n");
        fprintf(out_file, "\nAcknowledgment segment recieved:\n");
        display(close_segment, out_file);
        printf("\nCalculated checksum: 0X%04X\n", client_checksum);
        fprintf(out_file, "\nCalculated checksum: 0X%04X\n", client_checksum);
    }
    else
    {
        printf("checksum error4");
        exit(1);
    }
    printf("\n***Connection is closed***\n");
    fprintf(out_file, "\n***Connection is closed***\n");

///////////////////////////////////////////////////////////////////////////////////////////////

    //close socket and files
    close(s_socket);
    close(out_file);
    return 0;
}



void die(char *s)
{
    perror(s);
    exit(1);
}

//function used to calculate the checksum
unsigned short int find_checksum(struct header segment)
{
    segment.cksum = 0;
    unsigned short int sum = 0;
    unsigned short int checksum;
    unsigned short int check[524];

    memcpy(check, &segment, 1048);
    //summing loop
    for(int i = 0; i < 524; i++)
    {
        sum += check[i];
    }
    //calculate the sum
    checksum = sum >> 16;
    sum = sum & 0x0000FFFF;
    sum = checksum + sum;
    //add the sum to the check sum
    checksum = sum >> 16;
    sum = sum & 0x0000FFFF;
    checksum = checksum + sum;
    //return the calculated results
    return 0xFFFF^checksum;
}

//function used to display results
void display(struct header segment, FILE* log_file)
{
    /*
        display the reults to the user
    */
    printf("Source port number: \t\t%d\n", segment.des);
    printf("Destination port number: \t%d\n", segment.src);
    printf("Sequence number: \t\t%d\n", segment.seq);
    printf("ACK number: \t\t\t%d\n", segment.ack);
    if (segment.flags & SYN)
    {
        printf("Header flags:   \t\tSYN = 1\n");
    }
    if (segment.flags & ACK)
    {
        printf("Header flags:   \t\tACK = 1\n");
    }
    if (segment.flags & FIN)
    {
        printf("Header flags:   \t\tFIN = 1\n");
    }
    printf("Flags value: \t\t\t%x\n", segment.flags);
    printf("Receive window: \t\t%d\n", segment.rec);
    printf("Checksum: \t\t\t0X%04X\n", segment.cksum);
    printf("pointer: \t\t\t%d\n", segment.ptr);
    printf("Options: \t\t\t%d\n", segment.opt);
    printf("Offset: \t\t\t%d\n", segment.offset);
    printf("Payload: \t\t\t%s\n", segment.payload);
    
    /*
        print the results to the log file
    */
    fprintf(log_file, "Source port number: \t\t%d\n", segment.des);
    fprintf(log_file, "Destination port number: \t%d\n", segment.src);
    fprintf(log_file, "Sequence number: \t\t\t%d\n", segment.seq);
    fprintf(log_file, "ACK number: \t\t\t\t%d\n", segment.ack);
    if (segment.flags & SYN)
    {
        fprintf(log_file, "Header flags:   \t\t\tSYN = 1\n");
    }
    if (segment.flags & ACK)
    {
        fprintf(log_file, "Header flags:   \t\t\tACK = 1\n");
    }
    if (segment.flags & FIN)
    {
        fprintf(log_file, "Header flags:   \t\t\tFIN = 1\n");
    }
    fprintf(log_file, "Flags value: \t\t\t\t%x\n", segment.flags);
    fprintf(log_file, "Receive window: \t\t\t%d\n", segment.rec);
    fprintf(log_file, "Checksum: \t\t\t\t\t0X%04X\n", segment.cksum);
    fprintf(log_file, "pointer: \t\t\t\t\t%d\n", segment.ptr);
    fprintf(log_file, "Options: \t\t\t\t\t%d\n", segment.opt);
    fprintf(log_file, "Offset: \t\t\t\t\t%d\n", segment.offset);
    fprintf(log_file, "Payload: \t\t\t\t\t%s\n", segment.payload);
}