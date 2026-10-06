#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Interface
{
    char name[16];
    char ip[16];
    char mask[16];
    char gateway[16];
    int vlan;
    int mtu;
};


int main(void)
{
    FILE *file;
    char line[256];
    struct Interface *iface;
    char *end;
    long vlan;
    int config_valid = 1;

    iface = calloc(1, sizeof(*iface));

    if(iface == NULL)
    {
        printf("Memory allocation failed\n");
	return 1;
    }

    file = fopen("config/network.conf", "r");

    if (file == NULL)
    {
        printf("Failed to open configuration file\n");
	free(iface);
        return 1;
    }
    while (fgets(line, sizeof(line), file) != NULL)
    {
        char *key;
        char *value;

        key = strtok(line, " \n");
        value = strtok(NULL, " \n");

        if (key != NULL && value != NULL)
        {
            printf("Key   : %s\n", key);
            printf("Value : %s\n", value);
        
	    if (strcmp(key, "interface") == 0)
	    {
		    strcpy(iface->name, value);
	    }
	    else if (strcmp(key, "ip") == 0)
	    {
		    strcpy(iface->ip, value);
	    }
	    else if (strcmp(key, "mask") == 0)
	    {
		    strcpy(iface->mask, value);
	    }
	    else if (strcmp(key, "gateway") == 0)
	    {
		    strcpy(iface->gateway, value);
	    }
	    else if (strcmp(key, "vlan") == 0)
	    {
		    vlan = strtol(value, &end, 10);

		    if (*end != '\0')
	            {
                        printf("Invalid VLAN value: %s\n", value);
			config_valid = 0;
	            }
		    else if (vlan < 1 || vlan > 4094)
                    {
                        printf("VLAN out of range: %ld\n", vlan);
			config_valid = 0;
                    }
		    else
		    {
		        iface->vlan = vlan;
		    }
	    }
	    else if (strcmp(key, "mtu") == 0)
	    {
		    iface->mtu = atoi(value);
	    }
	}
    }

    if (!config_valid)
    {
        printf("Configuration parsing failed\n");
        fclose(file);
        free(iface);
        return 1;
    }

    printf("Interface name: %s\n", iface->name);
    printf("Ip Address: %s\n", iface->ip);
    printf("Mask address: %s\n", iface->mask);
    printf("Gateway address: %s\n", iface->gateway);
    printf("Vlan number: %d\n", iface->vlan);
    printf("Mtu value: %d\n", iface->mtu);

    fclose(file);

    free(iface);
    iface = NULL;

    return 0;
}
