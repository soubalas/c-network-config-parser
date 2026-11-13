# C Network Configuration Parser

A command-line network configuration parser written in C for Linux.

The project reads a text-based configuration file, validates network interface settings, and stores the parsed values in a C structure.

It demonstrates practical Embedded C, Linux, networking, defensive programming, and automated testing concepts.

## Features

- Parse network interface configuration files using standard C file I/O
- Validate IPv4 addresses and default gateways
- Validate contiguous IPv4 subnet masks
- Validate VLAN IDs (1–4094)
- Validate MTU values (576–9000)
- Detect duplicate and missing configuration keys
- Reject malformed configuration entries and overlong lines
- Support configuration files without a final newline
- Enable optional debug logging using conditional compilation
- Run automated tests with AddressSanitizer and UndefinedBehaviorSanitizer

## Configuration Format

Example configuration:

```ini
interface eth0
ip 192.168.1.10
mask 255.255.255.0
gateway 192.168.1.1
vlan 10
mtu 1500
```

All six configuration keys are required.

## Build

Requirements:
- Linux
- GCC
- GNU Make

Build the application:

```bash
make
```

## Run

Run with the default configuration file:

```bash
./network_config
```

Run with a custom configuration file:

```bash
./network_config config/network.conf
```

## Testing

Run the automated test suite:

```bash
make test
```

Run tests with detailed debug logging:

```bash
make debug
```

Run tests with AddressSanitizer and UndefinedBehaviorSanitizer:

```bash
make sanitize
```

The current test suite contains 24 test cases covering valid configurations, invalid values, boundary conditions, subnet masks, duplicate keys, overlong lines, and missing final newlines.

## Project Structure

```text
c-network-config-parser/
├── config/
│   └── network.conf
├── tests/
│   ├── test_parser.c
│   └── *.conf
├── interface.h
├── main.c
├── parser.c
├── parser.h
├── Makefile
├── README.md
└── LICENSE
```

## Technical Concepts

This project demonstrates:

- C structures, pointers, and string handling
- File operations using `fopen`, `fgets`, and `fclose`
- IPv4 validation using `inet_pton`
- Bitwise operations for subnet mask validation
- Defensive input validation and error handling
- Conditional compilation using `#ifdef DEBUG`
- GCC compiler and linker flags
- Makefile-based build automation
- Automated testing and sanitizer-based debugging

## License

See the [LICENSE](LICENSE) file for licensing information.
