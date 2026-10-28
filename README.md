# mininet

**mininet** is a lightweight, terminal-based network monitor developed for MINIX 3.

It provides real-time network interface statistics, including receive and transmit throughput, packet rates, cumulative traffic, and throughput history, using a compact ncurses interface.

The application is written in C and uses native MINIX networking interfaces rather than Linux-specific `/proc` facilities.

## Features

- Monitor available network interfaces.
- Display receive (RX) and transmit (TX) throughput in KiB/s.
- Display receive and transmit packet rates.
- Show cumulative received and transmitted data.
- Report packet counts and interface errors.
- Display adaptive throughput meters.
- Visualize recent RX and TX activity using Unicode block graphs.
- Switch between network interfaces interactively.
- Adjust the sampling interval while the program is running.
- Use a 256-color terminal interface.

## Requirements

Developed and tested on MINIX 3.4.0 (i386).

Build requirements:

- Clang
- BSD make
- ncursesw (pkgsrc)
- [citrus-compat](https://github.com/rcbellN51/citrus-compat)

The `citrus-compat` library restores access to MINIX's existing dynamic Citrus internationalization interfaces, allowing the application to initialize a UTF-8 locale for Unicode rendering.

The library is linked statically into the executable. No system libraries need to be replaced or modified.

## Building

Clone or place `mininet` and `citrus-compat` in adjacent directories:

```text
Projects/src/
├── citrus-compat/
└── mininet/
```

Build `citrus-compat` first:

```sh
cd ../citrus-compat
make
```

Then build `mininet`:

```sh
cd ../mininet
make
```

By default, the Makefile expects the compatibility library at:

```text
../citrus-compat/libcitrus-compat.a
```

An alternative path can be supplied:

```sh
make CITRUS_LIB=/path/to/libcitrus-compat.a
```

## Running

Run directly from the project directory:

```sh
./mininet
```

Root privileges are not required.

### Keyboard controls

| Key | Action |
|---|---|
| `Tab` | Switch to the next network interface |
| `+` | Increase the sampling interval |
| `-` | Decrease the sampling interval |
| `q` | Quit |

The default sampling interval is 2 seconds and can be adjusted between 0.5 and 10 seconds.

## Display

The interface includes:

- Current network interface and sampling interval.
- RX and TX throughput and packet rates.
- Colored throughput meters with adaptive scaling.
- Separate RX and TX Unicode throughput histories.
- Total received and transmitted traffic.
- Packet counters and interface error counts.

The history display uses separate scales for received and transmitted traffic to preserve detail when traffic levels differ significantly.

Switching interfaces resets the throughput history for the newly selected interface.

## Installation

To install under `/usr/local/bin`:

```sh
su -
cd /home/admin/Projects/src/mininet
make install
```

The installation prefix can be overridden using `PREFIX`.

## Development

Development history, technical implementation, testing, and provenance are documented in [DEVELOPMENT.md](DEVELOPMENT.md).

## Limitations

The project has been developed and tested on MINIX 3.4.0 (i386).

Compatibility with other MINIX releases, architectures, or operating systems has not been established.

The application requires a terminal with suitable Unicode and color support. The minimum supported terminal size is 50 columns by 24 rows.

The project remains experimental.

## License

Distributed under the MIT License.

Copyright (c) 2026 Number_51

See [LICENSE](LICENSE) for details.
