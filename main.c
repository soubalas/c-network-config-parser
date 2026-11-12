#include <stdio.h>

#include "interface.h"
#include "parser.h"


/*
 * Read a network configuration file and display
 * the parsed interface settings.
 *
 * Usage:
 *   ./network_config
 *   ./network_config <config_file>
 *
 * If no filename is supplied, use the default
 * configuration file.
 */

int main(int argc, char *argv[])
{
	struct Interface iface = {0};

	const char *filename = "config/network.conf";

	/* Accept at most one command-line argument. */
	if (argc > 2) {
		fprintf(stderr, "Usage: %s [config_file]\n", argv[0]);
		return 1;
	}

	/* Use the filename provided by the user, if any. */
	if (argc == 2) {
		filename = argv[1];
	}

	if (parse_config(filename, &iface) != 0) {
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

