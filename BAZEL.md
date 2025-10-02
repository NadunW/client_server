# Bazel Build System

This project uses [Bazel](https://bazel.build/) as its build system.

## Quick Start

```bash
# Build both server and client
bazel build //:server //:client

# Run the server
./bazel-bin/server 8080

# Run the client
./bazel-bin/client 127.0.0.1 8080
```

## Building with Oracle Support

To build with Oracle database support:

```bash
export ORACLE_HOME=/path/to/oracle/instantclient

bazel build --config=oracle //:server //:client \
  --action_env=ORACLE_HOME=$ORACLE_HOME \
  --linkopt=-L$ORACLE_HOME/lib \
  --copt=-I$ORACLE_HOME/include
```

## Configuration

The project includes a `.bazelrc` file with predefined configurations:

- `--config=oracle` - Enables Oracle database support
- `--config=opt` - Optimized build
- `--config=dbg` - Debug build with symbols

## Bazel Targets

- `//:server` - Backend server application
- `//:client` - Frontend client application

## Version

This project uses Bazel version specified in `.bazelversion` (currently 6.4.0).

## Why Bazel?

Bazel provides:
- Fast, incremental builds
- Reproducible builds across different machines
- Better dependency management
- Support for multiple languages and platforms
- Hermetic builds (isolated from system state)

For more information, see [Bazel documentation](https://bazel.build/docs).
