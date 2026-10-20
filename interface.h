#ifndef INTERFACE_H
#define INTERFACE_H

struct Interface
{
    char name[16];
    char ip[16];
    char mask[16];
    char gateway[16];
    int vlan;
    int mtu;
};

#endif
