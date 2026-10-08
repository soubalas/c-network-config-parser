#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdint.h>

struct Interface
{
	char name[16];
	char ip[16];
	char mask[16];
	char gateway[16];
	int vlan;
	int mtu;
};


int interface_seen = 0;
int ip_seen = 0;
int mask_seen = 0;
int gateway_seen = 0;
int vlan_seen = 0;
int mtu_seen = 0;


int main(void)
{
	FILE *file;
	char line[256];
	struct Interface *iface;
	char *end;
	long vlan;
	long mtu;
	int config_valid = 1;
	struct in_addr addr;
	struct in_addr mask_addr;
	uint32_t mask;
	uint32_t inverse;

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
				if (interface_seen)
				{
					printf("Duplicate configuration key: interface\n");
					config_valid = 0;
				}

				else
				{
					strcpy(iface->name, value);
					interface_seen = 1;
				}
			}

			else if (strcmp(key, "ip") == 0)
			{     
				if (ip_seen)
				{
					printf("Duplicate configuration key: ip\n");
					config_valid = 0;
				}

				else if (inet_pton(AF_INET, value, &addr) != 1)
				{
					printf("Invalid IP address: %s\n", value);
					config_valid = 0;
				}

				else
				{
					strcpy(iface->ip, value);
					ip_seen = 1;
				}
			}

			else if (strcmp(key, "mask") == 0)
			{ 
				if (mask_seen)
				{
					printf("Duplicate configuration key: mask\n");
					config_valid = 0;
				}

				else if (inet_pton(AF_INET, value, &mask_addr) != 1)
				{
					printf("Invalid subnet mask: %s\n", value);
					config_valid = 0;
				}

				else
				{
					mask = ntohl(mask_addr.s_addr);
					inverse = ~mask;

					if ((inverse & (inverse + 1)) != 0)
					{
						printf("Invalid subnet mask: %s\n", value);
						config_valid = 0;
					}

					else
					{
						strcpy(iface->mask, value);
						mask_seen = 1;
					}
				}
			}

			else if (strcmp(key, "gateway") == 0)
			{ 
				if (gateway_seen)
				{
					printf("Duplicate configuration key: gateway\n");
					config_valid = 0;
				}

				else if (inet_pton(AF_INET, value, &addr) != 1)
				{
					printf("Invalid gateway address: %s\n", value);
					config_valid = 0;
				}

				else
				{
					strcpy(iface->gateway, value);
					gateway_seen = 1;
				}
			}

			else if (strcmp(key, "vlan") == 0)
			{
				if (vlan_seen)
				{
					printf("Duplicate configuration key: vlan\n");
					config_valid = 0;
				}

				else
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
						vlan_seen = 1;
					}
				}
			}

			else if (strcmp(key, "mtu") == 0)
			{
				if (mtu_seen)
				{
					printf("Duplicate configuration key: mtu\n");
					config_valid = 0;
				}

				else
				{
					mtu = strtol(value, &end, 10);

					if (*end != '\0')
					{
						printf("Invalid mtu value: %s\n", value);
						config_valid = 0;
					}

					else if (mtu < 576 || mtu > 9000)
					{
						printf("MTU out of range: %ld\n", mtu);
						config_valid = 0;
					}

					else
					{
						iface->mtu = mtu;
						mtu_seen = 1;
					}
				}
			}

			else
			{
				printf("Unknown configuration key: %s\n", key);
				config_valid = 0;
			}
		}
	}

	if (!interface_seen)
	{
		printf("Missing required configuration key: interface\n");
		config_valid = 0;
	}

	if (!ip_seen)
	{
		printf("Missing required configuration key: ip\n");
		config_valid = 0;
	}

	if (!mask_seen)
	{
		printf("Missing required configuration key: mask\n");
		config_valid = 0;
	}

	if (!gateway_seen)
	{
		printf("Missing required configuration key: gateway\n");
		config_valid = 0;
	}

	if (!vlan_seen)
	{
		printf("Missing required configuration key: vlan\n");
		config_valid = 0;
	}

	if (!mtu_seen)
	{
		printf("Missing required configuration key: mtu\n");
		config_valid = 0;
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
