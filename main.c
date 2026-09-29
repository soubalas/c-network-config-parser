#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Interface
{
    char name[16];
    char ip[16];
    char mask[16];
    int vlan;
    int mtu;
};

int main(void)
{
    struct Interface *iface;

    iface = malloc(sizeof(*iface));

    if (iface == NULL)
    {
        printf("Memory allocation failed\n");
        return 1;
    }

    strcpy(iface->name, "eth0");
    strcpy(iface->ip, "192.168.1.10");
    strcpy(iface->mask, "255.255.255.0");

    iface->vlan = 10;
    iface->mtu = 1500;

    printf("Interface : %s\n", iface->name);
    printf("IP        : %s\n", iface->ip);
    printf("Mask      : %s\n", iface->mask);
    printf("VLAN      : %d\n", iface->vlan);
    printf("MTU       : %d\n", iface->mtu);

    free(iface);
    iface = NULL;

    return 0;
}
