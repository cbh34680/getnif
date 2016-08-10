/*
 * gcc -O2 -std=c11 -o getnif getnif.c
 *
 * Usage:
 *	./getnif -d eth0 -a
 *	./getnif -d eth0 -n -a -b
 *	./getnif -a -m
 *
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <getopt.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <linux/sockios.h>

static int print_if(int s, const char* dev, int tnum);

static int disp_hwaddr;
static int disp_address;
static int disp_broadcast;
static int disp_netmask;
static int disp_network;

int main(int argc, char** argv)
{
	int ret = EXIT_FAILURE;

	int s = -1;
	int c = -1;

	char* dev = NULL;
	int tnum = 0;

	while ((c = getopt(argc, argv, "d:habmn")) != -1)
	{
		switch(c)
		{
			case 'h':
			{
				disp_hwaddr = 1;
				++tnum;
				break;
			}
			case 'a':
			{
				disp_address = 1;
				++tnum;
				break;
			}
			case 'b':
			{
				disp_broadcast = 1;
				++tnum;
				break;
			}
			case 'm':
			{
				disp_netmask = 1;
				++tnum;
				break;
			}
			case 'n':
			{
				disp_network = 1;
				++tnum;
				break;
			}
			case 'd':
			{
				free(dev);

				dev = strdup(optarg);
				if (! dev)
					break;

				break;
			}
		}
	}

	if (tnum)
	{
		s = socket(AF_INET, SOCK_DGRAM, 0);
		if (s == -1)
			goto EXIT_LABEL;

		if (dev)
		{
			print_if(s, dev, tnum);
		}
		else
		{
			struct ifreq ifs[100];
			struct ifconf ifc = { .ifc_len=sizeof(ifs), .ifc_ifcu={ .ifcu_req=ifs } };

			if (ioctl(s, SIOCGIFCONF, &ifc) == -1)
				goto EXIT_LABEL;

			struct ifreq* end = ifs + (ifc.ifc_len / sizeof(struct ifreq));

			for (struct ifreq* ifr=ifc.ifc_req; ifr<end; ++ifr)
			{
				if (ifr->ifr_addr.sa_family == AF_INET)
				{
					if (strcmp(ifr->ifr_ifrn.ifrn_name, "lo") != 0)
					{
						print_if(s, ifr->ifr_ifrn.ifrn_name, tnum);
						break;
					}
				}
			}
		}
	}
	else
	{
		fprintf(stderr, "Usage: %s -d DEVIDE {-h|-a|-b|-m|-n}\n", argv[0]);
	}

	ret = EXIT_SUCCESS;

EXIT_LABEL:
	if (ret != EXIT_SUCCESS)
	{
		perror("main");
	}

	if (dev)
	{
		free(dev);
		dev = NULL;
	}

	if (s != -1)
	{
		close(s);
		s = -1;
	}

	return ret;
}

static int print_if(int s, const char* dev, int tnum)
{
	int ret = -1;

	struct ifreq ifr;
	int enable = 1;
	struct sockaddr_in *sin = NULL;

	if (disp_hwaddr)
	{
		memset(&ifr, 0, sizeof(ifr));
		strcpy(ifr.ifr_name, dev);

		if (ioctl(s, SIOCGIFHWADDR, &ifr) == -1)
			goto EXIT_LABEL;

		if (tnum > 1)
			printf("h,");

		unsigned char* addr = ifr.ifr_hwaddr.sa_data;
		printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
			addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
	}

	if (disp_address)
	{
		memset(&ifr, 0, sizeof(ifr));
		strcpy(ifr.ifr_name, dev);

		if (ioctl(s, SIOCGIFADDR, &ifr) == -1)
			goto EXIT_LABEL;

		if (tnum > 1)
			printf("a,");

		sin = (struct sockaddr_in *)&ifr.ifr_addr;
		printf("%s\n", inet_ntoa(sin->sin_addr));
	}

	if (disp_broadcast)
	{
		memset(&ifr, 0, sizeof(ifr));
		strcpy(ifr.ifr_name, dev);

		if (ioctl(s, SIOCGIFBRDADDR, &ifr) == -1)
			goto EXIT_LABEL;

		if (tnum > 1)
			printf("b,");

		sin = (struct sockaddr_in *)&ifr.ifr_broadaddr;
		printf("%s\n", inet_ntoa(sin->sin_addr));
	}

	if (disp_netmask)
	{
		memset(&ifr, 0, sizeof(ifr));
		strcpy(ifr.ifr_name, dev);

		if (ioctl(s, SIOCGIFNETMASK, &ifr) == -1)
			goto EXIT_LABEL;

		if (tnum > 1)
			printf("m,");

		sin = (struct sockaddr_in *)&ifr.ifr_netmask;
		printf("%s\n", inet_ntoa(sin->sin_addr));
	}

	if (disp_network)
	{
		memset(&ifr, 0, sizeof(ifr));
		strcpy(ifr.ifr_name, dev);

		if (ioctl(s, SIOCGIFADDR, &ifr) == -1)
			goto EXIT_LABEL;

		struct sockaddr_in a = *((struct sockaddr_in *)&ifr.ifr_netmask);
//
		memset(&ifr, 0, sizeof(ifr));
		strcpy(ifr.ifr_name, dev);

		if (ioctl(s, SIOCGIFNETMASK, &ifr) == -1)
			goto EXIT_LABEL;

		sin = (struct sockaddr_in *)&ifr.ifr_netmask;
//
		if (tnum > 1)
			printf("n,");

		sin->sin_addr.s_addr &= a.sin_addr.s_addr;
		printf("%s\n", inet_ntoa(sin->sin_addr));
	}

	ret = 0;

EXIT_LABEL:
	if (ret != 0)
	{
		fprintf(stderr, "print_if(%d, '%s', %d)\n", s, dev, tnum);
	}

	return ret;
}

