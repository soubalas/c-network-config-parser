#include <stdio.h>

#include "interface.h"
#include "parser.h"

int main(void)
{
    struct Interface iface = {0};

    if (parse_config("config/network.conf", &iface) != 0) {
        return 1;
    }

    printf("Interface name: %s\n", iface.name);
    printf("IP Address: %s\n", iface.ip);
    printf("Mask address: %s\n", iface.mask);
    printf("Gateway address: %s\n", iface.gateway);
    printf("VLAN number: %d\n", iface.vlan);
    printf("MTU value: %d\n", iface.mtu);

    return 0;
}

