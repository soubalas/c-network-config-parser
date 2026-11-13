#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../parser.h"

/* Count completed test cases, not individual conditions. */
static int tests_passed = 0;
static int tests_failed = 0;

/* Report one test case and update the summary counters. */
static void record_test(const char *test_name, int passed)
{
	if (passed) {
		++tests_passed;
		printf("PASS: %s\n", test_name);
	} else {
		++tests_failed;
		printf("FAIL: %s\n", test_name);
	}
}

/* Display the final summary after every test has run. */
static void print_test_summary(void)
{
	printf("\n================================\n");
	printf("       TEST SUMMARY\n");
	printf("================================\n");
	printf("Total tests  : %d\n", tests_passed + tests_failed);
	printf("Passed tests : %d\n", tests_passed);
	printf("Failed tests : %d\n", tests_failed);
	printf("================================\n");
	printf("%s\n", tests_failed == 0 ? "ALL TESTS PASSED" : "SOME TESTS FAILED");
}

/* Parse a fixture and verify its expected return value. */
static void test_fixture(const char *filename, const char *name,
                         int expected_result)
{
	struct Interface iface = {0};
	int result = parse_config(filename, &iface);
	record_test(name, result == expected_result);
}

/* Test a valid configuration and verify the parsed fields. */
static void test_valid_configuration(void)
{
	struct Interface iface = {0};
	int result = parse_config("config/network.conf", &iface);

	record_test("Valid configuration parsed successfully",
			result == 0 &&
			strcmp(iface.name, "eth0") == 0 &&
			strcmp(iface.ip, "192.168.1.10") == 0 &&
			iface.vlan == 10 &&
			iface.mtu == 1500);
}

/*
 * Test VLAN boundaries using a temporary configuration file.
 * Expected result: 0 for valid VLANs, -1 for invalid VLANs.
 */
static void test_vlan_boundary(int vlan, int expected_result)
{
	const char *filename = "tests/temp_vlan.conf";
	char name[80];
	struct Interface iface = {0};
	int passed = 1;
	FILE *file = fopen(filename, "w");

	snprintf(name, sizeof(name), "VLAN boundary test %d", vlan);

	if (file == NULL) {
		passed = 0;
	} else {
		if (fprintf(file,
					"interface eth0\n"
					"ip 192.168.1.10\n"
					"mask 255.255.255.0\n"
					"gateway 192.168.1.1\n"
					"vlan %d\n"
					"mtu 1500\n", vlan) < 0) {
			passed = 0;
		}
		if (fclose(file) != 0) {
			passed = 0;
		}

		if (passed) {
			int result = parse_config(filename, &iface);
			passed = result == expected_result &&
				(expected_result != 0 || iface.vlan == vlan);
		}
		if (remove(filename) != 0) {
			passed = 0;
		}

	}

	record_test(name, passed);
}

/*
 * Test MTU boundaries using a temporary configuration file.
 * Expected result: 0 for valid MTUs, -1 for invalid MTUs.
 */
static void test_mtu_boundary(int mtu, int expected_result)
{
	const char *filename = "tests/temp_mtu.conf";
	char name[80];
	struct Interface iface = {0};
	int passed = 1;
	FILE *file = fopen(filename, "w");

	snprintf(name, sizeof(name), "MTU boundary test %d", mtu);

	if (file == NULL) {
		passed = 0;
	} else {
		if (fprintf(file,
					"interface eth0\n"
					"ip 192.168.1.10\n"
					"mask 255.255.255.0\n"
					"gateway 192.168.1.1\n"
					"vlan 10\n"
					"mtu %d\n", mtu) < 0) {
			passed = 0;
		}
		if (fclose(file) != 0) {
			passed = 0;
		}

		if (passed) {
			int result = parse_config(filename, &iface);
			passed = result == expected_result &&
				(expected_result != 0 || iface.mtu == mtu);
		}
		if (remove(filename) != 0) {
			passed = 0;
		}
	}

	record_test(name, passed);
}

/*
 * Test valid, noncontiguous, and malformed IPv4 subnet masks.
 * For valid input, also verify the stored mask string.
 */
static void test_subnet_mask(const char *mask, int expected_result)
{
	const char *filename = "tests/temp_mask.conf";
	char name[100];
	struct Interface iface = {0};
	int passed = 1;
	FILE *file = fopen(filename, "w");

	snprintf(name, sizeof(name), "Subnet mask test %s", mask);

	if (file == NULL) {
		passed = 0;
	} else {
		if (fprintf(file,
					"interface eth0\n"
					"ip 192.168.1.10\n"
					"mask %s\n"
					"gateway 192.168.1.1\n"
					"vlan 10\n"
					"mtu 1500\n", mask) < 0) {
			passed = 0;
		}
		if (fclose(file) != 0) {
			passed = 0;
		}

		if (passed) {
			int result = parse_config(filename, &iface);
			passed = result == expected_result &&
				(expected_result != 0 || strcmp(iface.mask, mask) == 0);
		}
		if (remove(filename) != 0) {
			passed = 0;
		}
	}

	record_test(name, passed);
}

/* Verify that an oversized physical line is rejected. */
static void test_long_line(void)
{
	test_fixture("tests/long_line.conf",
			"Overlong configuration line rejected successfully", -1);
}

/* Verify that EOF without a final newline is accepted. */
static void test_no_final_newline(void)
{
	struct Interface iface = {0};
	int result = parse_config("tests/no_final_newline.conf", &iface);

	record_test("Configuration without final newline accepted successfully",
			result == 0 && iface.mtu == 1500);
}

/* Run all 24 test cases and return failure status to make. */
int main(void)
{
	/* Tests 1-4: Valid input and invalid field values. */
	test_valid_configuration();
	test_fixture("tests/invalid_ip.conf", "Invalid IP address rejected successfully", -1);
	test_fixture("tests/invalid_gateway.conf", "Invalid gateway rejected successfully", -1);
	test_fixture("tests/invalid_vlan.conf", "Invalid VLAN rejected successfully", -1);

	/* Tests 5-8: VLAN boundary values. */
	test_vlan_boundary(0, -1);
	test_vlan_boundary(1, 0);
	test_vlan_boundary(4094, 0);
	test_vlan_boundary(4095, -1);

	/* Tests 9-12: MTU boundary values. */
	test_mtu_boundary(575, -1);
	test_mtu_boundary(576, 0);
	test_mtu_boundary(9000, 0);
	test_mtu_boundary(9001, -1);

	/* Tests 13-19: Valid and invalid subnet masks. */
	test_subnet_mask("255.255.255.0", 0);
	test_subnet_mask("255.255.255.128", 0);
	test_subnet_mask("255.255.255.192", 0);
	test_subnet_mask("255.255.0.0", 0);
	test_subnet_mask("255.0.255.0", -1);
	test_subnet_mask("255.255.255.1", -1);
	test_subnet_mask("255.255.255.256", -1);

	/* Test 20: Duplicate gateway key. */
	test_fixture("tests/duplicate_gateway.conf",
			"Duplicate gateway rejected successfully", -1);

	/* Tests 21-22: Input-buffer and EOF edge cases. */
	test_long_line();
	test_no_final_newline();

	/* Test 23: Reject a configuration missing a required key. */
	test_fixture("tests/missing_interface.conf",
			"Missing interface key rejected successfully", -1);

	/* Test 24: Reject a configuration key without a value. */
	test_fixture("tests/missing_value.conf",
			"Missing IP value rejected successfully", -1);

	print_test_summary();
	return tests_failed == 0 ? 0 : 1;
}
