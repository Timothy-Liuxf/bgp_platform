# bgp_platform

## How to Build

### Prerequisites

+ Linux operating system
+ C++ compiler which supports C++17
+ GNU Autoconf and GNU Automake
+ `libpq` and `libpqxx-6.4`

### Clone this repository

```bash
$ git clone --recursive https://github.com/Timothy-Liuxf/bgp_platform.git
```

or

```bash
$ git clone https://github.com/Timothy-Liuxf/bgp_platform.git
$ cd bgp_platform
$ git submodule update --init --recursive
```

### Configure build system

```bash
# Requires GNU Autoconf and GNU Automake

$ autoreconf -i
$ [ENV=VAL] ./configure
```

The `ENV` and `VAL` can be:

+ `BUILD_CONFIG`: Choose configuration.
  + `Debug` / `debug`: Debug configuration.
  + `Release` / `release` (default): Release configuration.
+ `CXX`: The C++ compiler to use. The default value is `g++`.

### Build the target

```bash
$ make -j$(nproc)
```

## How to Run

### Configuration

Write configurations in `config/config.json`. Take [`config/examples/config.json`](config/examples/config.json) for an example.

### Run the program

```bash
$ ./build/bin/bgp_platform
```
