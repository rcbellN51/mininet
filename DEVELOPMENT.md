# Development

`mininet` is an original, lightweight terminal-based network monitor developed specifically for MINIX 3.

The project was created to provide a native MINIX network monitoring utility using the networking and system interfaces available on MINIX rather than relying on Linux-specific facilities or compatibility layers for network statistics.

## Development process

The project was developed through iterative AI-assisted collaboration with ChatGPT. Much of the source code, network statistics collection logic, terminal interface implementation, build configuration, and documentation was generated or refined during this collaboration.

The project maintainer directed development, selected features, evaluated implementation approaches, reviewed results, and compiled, tested, debugged, and validated the resulting implementation on the target system.

Development and testing were performed on MINIX 3.4.0 (i386) running in a QEMU/KVM virtual machine.

The development environment included:

- Clang and BSD make
- ncursesw from pkgsrc
- X11 with the dwm window manager
- xterm and remote SSH terminal sessions
- 256-color terminal output and Unicode rendering
- The separately developed `citrus-compat` compatibility library

The resulting program is a native C application and does not require AI services to build or operate.

## Technical implementation

Development involved investigating MINIX's available network interface statistics and adapting them to a continuously updating terminal display.

The implementation uses `getifaddrs()`, `AF_LINK`, and `struct if_data` to retrieve interface statistics.

Development included:

- Discovering available network interfaces.
- Reading cumulative byte and packet counters.
- Calculating receive and transmit throughput from counter differences and elapsed time.
- Calculating receive and transmit packet rates.
- Implementing an adjustable sampling interval.
- Developing adaptive throughput meters.
- Maintaining a bounded history of throughput samples.
- Rendering Unicode block graphs using ncursesw.
- Implementing interactive interface selection.
- Designing a compact 256-color terminal interface.
- Integrating the `citrus-compat` library for UTF-8 locale initialization.
- Developing a Makefile supporting compilation, cleanup, and installation.

The network statistics collector uses native MINIX interfaces and does not require a Linux compatibility layer.

## Source organization

The application is divided into separate modules for network statistics collection, history management, and terminal presentation.

| Module | Responsibility |
|---|---|
| `main.c` | Application initialization, sampling loop, and keyboard input |
| `collector.c` | Network interface discovery and statistics collection |
| `history.c` | Throughput sample history and adaptive scaling |
| `ui.c` | ncurses interface, throughput meters, and Unicode graphs |

The corresponding header files provide the interfaces between modules.

## Internationalization compatibility

MINIX 3.4.0 includes Citrus internationalization modules, but its libc does not provide working dynamic Citrus module loading for the required UTF-8 locale operations.

The separately maintained [citrus-compat](https://github.com/rcbellN51/citrus-compat) project restores those interfaces without replacing system libraries.

`mininet` links against the compatibility library to support Unicode rendering through ncursesw.

The compatibility library is a build dependency and is not included in the `mininet` source repository.

## Testing and validation

The implementation was tested directly on MINIX 3.4.0 throughout development.

Testing included:

- Repeated compilation using Clang and BSD make.
- Inspecting available network interfaces and their counters.
- Verifying throughput calculations during network activity.
- Testing receive and transmit traffic independently.
- Testing interface selection and history resets.
- Testing adjustable sampling intervals.
- Verifying adaptive meter behavior.
- Testing terminal colors and Unicode block rendering.
- Testing native xterm and remote SSH terminal sessions.
- Verifying successful operation with the statically linked `citrus-compat` library.

Development also included standalone diagnostic programs for network statistics, terminal colors, ncurses, and Unicode rendering.

These diagnostic programs remain local development tools and are not included in the distributed repository.

No automated regression test suite is currently included in the project.

## Limitations

The project has been developed and tested on MINIX 3.4.0 (i386).

Compatibility with other MINIX releases, architectures, or operating systems has not been established.

The interface requires ncursesw and a terminal capable of displaying the required colors and Unicode characters.

The application displays statistics provided by the operating system's network interfaces. Available information and counter behavior depend on the underlying MINIX network drivers.

The project remains experimental.

## Licensing

The original `mininet` source code is released under the MIT License.

See [LICENSE](LICENSE) for details.
