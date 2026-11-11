#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../parser.h"

/*
 * Test VLAN boundary values by generating a temporary configuration file.
 *
 * Parameters:
 *   vlan            - VLAN ID to test.
 *   expected_result - Expected parser return value:
 *                     0 for valid input, -1 for invalid input.
 *
 * Verifies that the parser accepts valid VLAN IDs and rejects values
 * outside the supported range (1-4094).
 *
 * The temporary configuration file is removed after each test.
 */

static void test_vlan_boundary(int vlan, int expected_result)
{
	const char *filename = "tests/temp_vlan.conf";

	FILE *file = fopen(filename, "w");
	assert(file != NULL);

	fprintf(file,
			"interface eth0\n"
			"ip 192.168.1.10\n"
			"mask 255.255.255.0\n"
			"gateway 192.168.1.1\n"
			"vlan %d\n"
			"mtu 1500\n",
			vlan);

	assert(fclose(file) == 0);

	struct Interface iface = {0};

	int result = parse_config(filename, &iface);

	if (expected_result == 0) {
		assert(result == 0);
		assert(iface.vlan == vlan);
	} else {
		assert(result == -1);
	}

	assert(remove(filename) == 0);

	printf("PASS: VLAN boundary test %d\n", vlan);
}

/*
 * Test MTU boundary values using a temporary configuration file.
 *
 * Parameters:
 *   mtu             - MTU value to test.
 *   expected_result - Expected parser return value:
 *                     0 for valid input, -1 for invalid input.
 *
 * Verifies that the parser accepts MTU values within the supported
 * range (576-9000) and rejects values outside that range.
 *
 * For valid input, also verifies that the parsed MTU is stored correctly.
 */

static void test_mtu_boundary(int mtu, int expected_result)
{
	const char *filename = "tests/temp_mtu.conf";

	FILE *file = fopen(filename, "w");
	assert(file != NULL);

	fprintf(file,
			"interface eth0\n"
			"ip 192.168.1.10\n"
			"mask 255.255.255.0\n"
			"gateway 192.168.1.1\n"
			"vlan 10\n"
			"mtu %d\n",
			mtu);

	assert(fclose(file) == 0);

	struct Interface iface = {0};

	int result = parse_config(filename, &iface);

	assert(result == expected_result);

	if (expected_result == 0) {
		assert(iface.mtu == mtu);
	}

	assert(remove(filename) == 0);

	printf("PASS: MTU boundary test %d\n", mtu);
}

/*
 * Test IPv4 subnet mask validation.
 *
 * Parameters:
 *   mask            - Subnet mask string to test.
 *   expected_result - Expected parser return value:
 *                     0 for valid input, -1 for invalid input.
 *
 * Verifies that the parser accepts valid contiguous subnet masks
 * and rejects noncontiguous or malformed IPv4 subnet masks.
 *
 * For valid input, also checks that the parsed mask matches the input.
 */

static void test_subnet_mask(const char *mask, int expected_result)
{
	const char *filename = "tests/temp_mask.conf";

	FILE *file = fopen(filename, "w");
	assert(file != NULL);

	fprintf(file,
			"interface eth0\n"
			"ip 192.168.1.10\n"
			"mask %s\n"
			"gateway 192.168.1.1\n"
			"vlan 10\n"
			"mtu 1500\n",
			mask);

	assert(fclose(file) == 0);

	struct Interface iface = {0};

	int result = parse_config(filename, &iface);

	assert(result == expected_result);

	if (expected_result == 0) {
		assert(strcmp(iface.mask, mask) == 0);
	}

	assert(remove(filename) == 0);

	printf("PASS: Subnet mask test %s\n", mask);
}

/*
 * Run the network configuration parser test suite.
 *
 * Covers:
 *   - Successful parsing of a valid configuration
 *   - Rejection of invalid IPv4 and gateway addresses
 *   - Rejection of invalid VLAN values
 *   - VLAN and MTU boundary validation
 *   - Valid and invalid IPv4 subnet masks
 *   - Duplicate gateway key detection
 *
 * Uses assert() to verify expected parser behavior.
 * Returns 0 if all assertions pass.
 */


int main(void)
{
	/* Test 1: Verify that a valid configuration is parsed correctly. */
	struct Interface iface = {0};

	int result = parse_config("config/network.conf", &iface);

	assert(result == 0);
	assert(strcmp(iface.name, "eth0") == 0);
	assert(strcmp(iface.ip, "192.168.1.10") == 0);
	assert(iface.vlan == 10);
	assert(iface.mtu == 1500);

	printf("PASS: Valid configuration parsed successfully\n");


	/* Test 2: Reject an invalid IPv4 address. */
	struct Interface invalid_iface = {0};

	int invalid_result =
		parse_config("tests/invalid_ip.conf", &invalid_iface);

	assert(invalid_result != 0);

	printf("PASS: Invalid IP address rejected successfully\n");


	/* Test 3: Reject an invalid gateway IPv4 address. */
	struct Interface invalid_gateway_iface = {0};

	int gateway_result = parse_config(
			"tests/invalid_gateway.conf",
			&invalid_gateway_iface
			);

	assert(gateway_result == -1);

	printf("PASS: Invalid gateway rejected successfully\n");


	/* Test 4: Reject a VLAN ID outside the valid range. */
	struct Interface invalid_vlan_iface = {0};

	int vlan_result =
		parse_config("tests/invalid_vlan.conf", &invalid_vlan_iface);

	assert(vlan_result == -1);

	printf("PASS: Invalid VLAN rejected successfully\n");


	/* Tests 5-8: Check VLAN lower and upper boundaries. */
	test_vlan_boundary(0, -1);
	test_vlan_boundary(1, 0);
	test_vlan_boundary(4094, 0);
	test_vlan_boundary(4095, -1);


	/* Tests 9-12: Check MTU lower and upper boundaries. */
	test_mtu_boundary(575, -1);
	test_mtu_boundary(576, 0);
	test_mtu_boundary(9000, 0);
	test_mtu_boundary(9001, -1);

	/* Tests 13-16: Accept valid contiguous subnet masks. */
	test_subnet_mask("255.255.255.0", 0);
	test_subnet_mask("255.255.255.128", 0);
	test_subnet_mask("255.255.255.192", 0);
	test_subnet_mask("255.255.0.0", 0);

	/* Tests 17-19: Reject invalid or malformed subnet masks. */
	test_subnet_mask("255.0.255.0", -1);
	test_subnet_mask("255.255.255.1", -1);
	test_subnet_mask("255.255.255.256", -1);

	/*
	 * Test 20: Reject duplicate gateway keys, even when the first
	 * gateway value is invalid and the second value is valid.
	 */
	struct Interface duplicate_gateway_iface = {0};

	int duplicate_result = parse_config(
			"tests/duplicate_gateway.conf",
			&duplicate_gateway_iface
			);

	assert(duplicate_result == -1);

	printf("PASS: Duplicate gateway rejected successfully\n");

	return 0;
}
