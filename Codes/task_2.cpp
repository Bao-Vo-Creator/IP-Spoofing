#include <sys/socket.h>	/* these headers are for a Linux system, but */
#include <netinet/in.h>	/* the names on other systems are easy to guess.. */
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <cstdio>

unsigned short checksum(unsigned short *addr, int len)
{
    int nleft = len;
    int sum = 0;
    unsigned short *w = addr;
    unsigned short answer = 0;

    while (nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }

    if (nleft == 1)
    {
        *(unsigned char *)(&answer) = *(unsigned char *)w;
        sum += answer;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    answer = ~sum;
    return (answer);
}

int main(){
  /*Open the raw socket and sned/receive TCP packets*/
  int s = socket (AF_INET, SOCK_RAW, IPPROTO_RAW);	/* open raw socket */
  char buffer[400];	/* this buffer will contain ip header, tcp header,
			   and payload. we'll point an ip header structure
			   at its beginning, and a tcp header structure after
			   that to write the header values into it */
  //struct ip *iph = (struct ip *)buffer;
  //struct icmp *icmph = (struct icmp *)(iph + 1);
  
  struct ip *iph = (struct ip *) buffer;
  struct icmp *icmph = (struct icmp *) (buffer + sizeof (struct ip));
  struct sockaddr_in sin;
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = inet_addr ("10.0.2.4");

  memset (buffer, 0, 400);	/* zero out the buffer */

  /*filling in the vlues for the ip header*/
  iph->ip_hl = 5; /*The length of the ip header will be 20 bytes*/
  iph->ip_v = 4; /*version of the ip protocol to use*/
  iph->ip_tos = 0; /*We are using the regular service*/
  iph->ip_len = sizeof (struct ip) + sizeof (struct icmp); /* There is no payload in this packet */
  iph->ip_id = 12345;	/* arbitrary id since we are not fragmenting the datagram/buffer */ /*remove if dont work*/
  iph->ip_off = 0; /* again not fragmenting the datagram/buffer*/
  iph->ip_ttl = 255; /*give it the longest time possible to get to other host*/
  iph->ip_p = IPPROTO_ICMP; /* specify we are using the icmp protocol */  /*remove if dont work*/
  iph->ip_sum = 0;		/* set it to 0 before computing the actual checksum later */
  iph->ip_src.s_addr = inet_addr ("8.8.8.8");/* SYN's can be blindly spoofed */
  iph->ip_dst.s_addr = sin.sin_addr.s_addr; /*destination addr, we are just sending the packet to ourselves*/

  /*filling in the values for the icmp header*/
  icmph->icmp_type = ICMP_ECHO;  /*remove if not qwork*/
  icmph->icmp_code = 0;
  icmph->icmp_id = 1234;
  icmph->icmp_seq = 0;
  icmph->icmp_cksum = checksum((u_short*)icmph, sizeof(struct icmp));
   {				/* lets do it the ugly way.. */
    int one = 1;
    const int *val = &one;
    if (setsockopt (s, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
      printf ("Warning: Cannot set HDRINCL!\n");
  }

  if (sendto (s, buffer, iph->ip_len, 0, (struct sockaddr *) &sin, sizeof (sin)) < 0){

    printf("error\n");
  }
  else{
    printf(".");
  }
	

  return 0;
}