#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <errno.h>

#include "parser.h"

/*
 * Parse a network interface configuration file.
 *
 * Parameters:
 *   filename - Path to the configuration file.
 *   iface    - Pointer to the structure that receives parsed values.
 *
 * Expected configuration format:
 *   interface eth0
 *   ip 192.168.1.10
 *   mask 255.255.255.0
 *   gateway 192.168.1.1
 *   vlan 10
 *   mtu 1500
 *
 * Validation:
 *   - All six configuration keys are required.
 *   - Duplicate and unknown keys are rejected.
 *   - IPv4 addresses must have valid dotted-decimal syntax.
 *   - Subnet masks must contain contiguous network bits.
 *   - VLAN IDs must be within the range 1-4094.
 *   - MTU values must be within the range 576-9000.
 *   - Interface names must fit in the destination buffer.
 *
 * Return values:
 *    0 - Configuration parsed successfully.
 *   -1 - Invalid arguments, file error, or configuration error.
 *
 * The output structure is cleared when configuration validation fails.
 */


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

	/*
	 * Track whether each required key has appeared.
	 *
	 * A key is marked as seen before validating its value.
	 * This allows duplicate detection even when the first value
	 * is invalid and avoids reporting a present key as missing.
	 */
	int interface_seen = 0;
	int ip_seen = 0;
	int mask_seen = 0;
	int gateway_seen = 0;
	int vlan_seen = 0;
	int mtu_seen = 0;
	size_t length;

	/* Reject invalid arguments before accessing either pointer. */
	if (filename == NULL || iface == NULL) {
		printf("Invalid parser argument\n");
		return -1;
	}

	/* Initialize the output structure to known zero values. */
	*iface = (struct Interface){0};

	/* Open the configuration file for reading. */
	file = fopen(filename, "r");

	if (file == NULL) {
		printf("Failed to open configuration file: %s\n", filename);
		return -1;
	}

	/* Read and process configuration data one buffer at a time. */
	while (fgets(line, sizeof(line), file) != NULL)
	{
		char *key;
		char *value;
		char *extra_value;

		/*
		 * Check whether fgets() filled the buffer without
		 * reading a newline character.
		 */
		size_t len = strlen(line);

		if (len == sizeof(line) - 1 && line[len - 1] != '\n')
		{
			int ch = fgetc(file);

			/*
			 * If the next character is neither a newline nor EOF,
			 * the physical line contains more data than the buffer
			 * can hold.
			 */

			if (ch != '\n' && ch != EOF)
			{
				fprintf(stderr, "Configuration line too long\n");
				config_valid = 0;

				/* Discard the remaining characters of this line. */
				while ((ch = fgetc(file)) != '\n' && ch != EOF)
				{
				}

				/* Skip tokenization of this invalid line. */
				continue;
			}
		}

		/*
		 * Split the input into whitespace-separated tokens.
		 *
		 * A valid configuration entry contains exactly two tokens:
		 * a configuration key and its corresponding value.
		 */

		key = strtok(line, " \t\r\n");
		value = strtok(NULL, " \t\r\n");
		extra_value = strtok(NULL, " \t\r\n");

		/* Ignore blank or whitespace-only lines. */
		if (key == NULL) {
			continue;
		}
		else if (value == NULL || extra_value != NULL) {
			/* Reject entries with missing or additional values. */
			printf("Invalid configuration line\n");
			config_valid = 0;
		}
		else {
			printf("Key   : %s\n", key);
			printf("Value : %s\n", value);

			/* Validate the interface name and its buffer length. */
			if (strcmp(key, "interface") == 0)
			{
				length = strlen(value);

				if (interface_seen) {
					printf("Duplicate configuration key: interface\n");
					config_valid = 0;
				}
				else {

					interface_seen = 1;
					/* Reserve space for the terminating null byte. */
					if (length >= sizeof(iface->name)) {
						printf("Interface name too long (maximum 15 characters)\n");
						config_valid = 0;
					}
					else {
						strcpy(iface->name, value);
					}
				}
			}

			/* Validate the IPv4 address using inet_pton(). */
			else if (strcmp(key, "ip") == 0)
			{
				if (ip_seen) {
					printf("Duplicate configuration key: ip\n");
					config_valid = 0;
				}
				else {
					ip_seen = 1;

					if (inet_pton(AF_INET, value, &addr) != 1) {
						printf("Invalid IP address: %s\n", value);
						config_valid = 0;
					}
					else {
						strcpy(iface->ip, value);
					}
				}
			}

			/* Validate IPv4 subnet mask syntax and bit continuity. */
			else if (strcmp(key, "mask") == 0)
			{
				if (mask_seen) {
					printf("Duplicate configuration key: mask\n");
					config_valid = 0;
				}
				else {
					mask_seen = 1;
					if (inet_pton(AF_INET, value, &mask_addr) != 1) {
						printf("Invalid subnet mask: %s\n", value);
						config_valid = 0;
					}
					else {
						/*
						 * Convert the mask from network byte order
						 * to host byte order before bitwise validation.
						 */

						mask = ntohl(mask_addr.s_addr);
						inverse = ~mask;

						/*
						 * A valid contiguous mask has leading 1 bits
						 * followed by 0 bits.
						 *
						 * Its inverse must contain only trailing 1 bits.
						 * The expression below detects noncontiguous bits.
						 */

						if ((inverse & (inverse + 1U)) != 0U) {
							printf("Invalid subnet mask: %s\n", value);
							config_valid = 0;
						}
						else {
							strcpy(iface->mask, value);
						}
					}
				}
			}

			/* Validate the gateway as an IPv4 address. */
			else if (strcmp(key, "gateway") == 0)
			{
				if (gateway_seen) {
					printf("Duplicate configuration key: gateway\n");
					config_valid = 0;
				}
				else {

					gateway_seen = 1;

					if (inet_pton(AF_INET, value, &addr) != 1) {
						printf("Invalid gateway address: %s\n", value);
						config_valid = 0;
					}
					else {
						strcpy(iface->gateway, value);
					}
				}
			}

			/* Parse and validate the VLAN ID (valid range: 1-4094). */
			else if (strcmp(key, "vlan") == 0)
			{
				if (vlan_seen) {
					printf("Duplicate configuration key: vlan\n");
					config_valid = 0;
				}
				else {
					vlan_seen = 1;

					/*
					 * Reset errno before conversion so that an
					 * earlier error does not affect this check.
					 */

					errno = 0;
					vlan = strtol(value, &end, 10);

					if (errno == ERANGE) {
						/* Detect numeric overflow or underflow. */
						printf("VLAN value is too large or too small: %s\n", value);
						config_valid = 0;
					}
					else if (end == value || *end != '\0') {
						/* Reject nonnumeric or partially numeric input. */
						printf("Invalid VLAN value: %s\n", value);
						config_valid = 0;
					}
					else if (vlan < 1 || vlan > 4094) {
						printf("VLAN out of range: %ld\n", vlan);
						config_valid = 0;
					}
					else {
						iface->vlan = (int)vlan;
					}
				}
			}

			/* Parse and validate the MTU (valid range: 576-9000). */
			else if (strcmp(key, "mtu") == 0)
			{
				if (mtu_seen) {
					printf("Duplicate configuration key: mtu\n");
					config_valid = 0;
				}
				else {
					mtu_seen = 1;

					/* Convert the MTU string into a long integer. */
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
					}
				}
			}
			else {
				/* Reject configuration keys that are not supported. */
				printf("Unknown configuration key: %s\n", key);
				config_valid = 0;
			}
		}
	}

	/* Detect errors that occurred while reading the file. */
	if (ferror(file)) {
		printf("Error reading configuration file\n");
		config_valid = 0;
	}

	/*
	 * Verify that every mandatory configuration key was provided.
	 * Missing keys make the entire configuration invalid.
	 */

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

	/* Close the file and report any closing error. */
	if (fclose(file) != 0) {
		printf("Error closing configuration file\n");
		config_valid = 0;
	}

	/*
	 * Reject invalid configurations and clear any partially
	 * populated interface data before returning an error.
	 */

	if (!config_valid) {
		printf("Configuration parsing failed\n");
		*iface = (struct Interface){0};
		return -1;
	}

	return 0;
}
