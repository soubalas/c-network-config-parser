#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <errno.h>

#include "parser.h"

int parse_config(const char *filename, struct Interface *iface)
{
	FILE *file;
	char line[256];
	char *end;
	long vlan;
	long mtu;
	int config_valid = 1;
	struct in_addr addr;
	struct in_addr mask_addr;
	uint32_t mask;
	uint32_t inverse;
	int interface_seen = 0;
	int ip_seen = 0;
	int mask_seen = 0;
	int gateway_seen = 0;
	int vlan_seen = 0;
	int mtu_seen = 0;
	size_t length;

	if (filename == NULL || iface == NULL) {
		printf("Invalid parser argument\n");
		return -1;
	}

	*iface = (struct Interface){0};

	file = fopen(filename, "r");

	if (file == NULL) {
		printf("Failed to open configuration file: %s\n", filename);
		return -1;
	}

	while (fgets(line, sizeof(line), file) != NULL)
	{
		char *key;
		char *value;
		char *extra_value;

		key = strtok(line, " \t\r\n");
		value = strtok(NULL, " \t\r\n");
		extra_value = strtok(NULL, " \t\r\n");

		if (key == NULL) {
			continue;
		}
		else if (value == NULL || extra_value != NULL) {
			printf("Invalid configuration line\n");
			config_valid = 0;
		}
		else {
			printf("Key   : %s\n", key);
			printf("Value : %s\n", value);

			if (strcmp(key, "interface") == 0)
			{
				length = strlen(value);

				if (interface_seen) {
					printf("Duplicate configuration key: interface\n");
					config_valid = 0;
				}
				else if (length >= sizeof(iface->name)) {
					printf("Interface name too long (maximum 15 characters)\n");
					config_valid = 0;
				}
				else {
					strcpy(iface->name, value);
					interface_seen = 1;
				}
			}
			else if (strcmp(key, "ip") == 0)
			{
				if (ip_seen) {
					printf("Duplicate configuration key: ip\n");
					config_valid = 0;
				}
				else if (inet_pton(AF_INET, value, &addr) != 1) {
					printf("Invalid IP address: %s\n", value);
					config_valid = 0;
				}
				else {
					strcpy(iface->ip, value);
					ip_seen = 1;
				}
			}
			else if (strcmp(key, "mask") == 0)
			{
				if (mask_seen) {
					printf("Duplicate configuration key: mask\n");
					config_valid = 0;
				}
				else if (inet_pton(AF_INET, value, &mask_addr) != 1) {
					printf("Invalid subnet mask: %s\n", value);
					config_valid = 0;
				}
				else {
					mask = ntohl(mask_addr.s_addr);
					inverse = ~mask;

					if ((inverse & (inverse + 1)) != 0) {
						printf("Invalid subnet mask: %s\n", value);
						config_valid = 0;
					}
					else {
						strcpy(iface->mask, value);
						mask_seen = 1;
					}
				}
			}
			else if (strcmp(key, "gateway") == 0)
			{
				if (gateway_seen) {
					printf("Duplicate configuration key: gateway\n");
					config_valid = 0;
				}
				else if (inet_pton(AF_INET, value, &addr) != 1) {
					printf("Invalid gateway address: %s\n", value);
					config_valid = 0;
				}
				else {
					strcpy(iface->gateway, value);
					gateway_seen = 1;
				}
			}
			else if (strcmp(key, "vlan") == 0)
			{
				if (vlan_seen) {
					printf("Duplicate configuration key: vlan\n");
					config_valid = 0;
				}
				else {
					errno = 0;
					vlan = strtol(value, &end, 10);

					if (errno == ERANGE) {
						printf("VLAN value is too large or too small: %s\n", value);
						config_valid = 0;
					}
					else if (end == value || *end != '\0') {
						printf("Invalid VLAN value: %s\n", value);
						config_valid = 0;
					}
					else if (vlan < 1 || vlan > 4094) {
						printf("VLAN out of range: %ld\n", vlan);
						config_valid = 0;
					}
					else {
						iface->vlan = (int)vlan;
						vlan_seen = 1;
					}
				}
			}
			else if (strcmp(key, "mtu") == 0)
			{
				if (mtu_seen) {
					printf("Duplicate configuration key: mtu\n");
					config_valid = 0;
				}
				else {
					errno = 0;
					mtu = strtol(value, &end, 10);

					if (errno == ERANGE) {
						printf("MTU value is too large or too small: %s\n", value);
						config_valid = 0;
					}
					else if (end == value || *end != '\0') {
						printf("Invalid MTU value: %s\n", value);
						config_valid = 0;
					}
					else if (mtu < 576 || mtu > 9000) {
						printf("MTU out of range: %ld\n", mtu);
						config_valid = 0;
					}
					else {
						iface->mtu = (int)mtu;
						mtu_seen = 1;
					}
				}
			}
			else {
				printf("Unknown configuration key: %s\n", key);
				config_valid = 0;
			}
		}
	}

	if (ferror(file)) {
		printf("Error reading configuration file\n");
		config_valid = 0;
	}

	if (!interface_seen) {
		printf("Missing required configuration key: interface\n");
		config_valid = 0;
	}
	if (!ip_seen) {
		printf("Missing required configuration key: ip\n");
		config_valid = 0;
	}
	if (!mask_seen) {
		printf("Missing required configuration key: mask\n");
		config_valid = 0;
	}
	if (!gateway_seen) {
		printf("Missing required configuration key: gateway\n");
		config_valid = 0;
	}
	if (!vlan_seen) {
		printf("Missing required configuration key: vlan\n");
		config_valid = 0;
	}
	if (!mtu_seen) {
		printf("Missing required configuration key: mtu\n");
		config_valid = 0;
	}

	if (fclose(file) != 0) {
		printf("Error closing configuration file\n");
		config_valid = 0;
	}

	if (!config_valid) {
		printf("Configuration parsing failed\n");
		*iface = (struct Interface){0};
		return -1;
	}

	return 0;
}
