# Build configuration for client-server application

# Server (backend) executable
cc_binary(
    name = "server",
    srcs = [
        "src/server/server.cpp",
        "src/server/database.cpp",
        "src/server/query_handler.cpp",
    ],
    hdrs = [
        "include/database.h",
        "include/query_handler.h",
    ],
    includes = ["include"],
    linkopts = ["-lpthread"],
    copts = ["-std=c++17"],
    local_defines = select({
        ":oracle_enabled": ["ORACLE_ENABLED"],
        "//conditions:default": [],
    }),
    deps = select({
        ":oracle_enabled": [":oracle_libs"],
        "//conditions:default": [],
    }),
)

# Client (frontend) executable
cc_binary(
    name = "client",
    srcs = ["src/client/client.cpp"],
    linkopts = ["-lpthread"],
    copts = ["-std=c++17"],
)

# Oracle library dependencies (when oracle=true is set)
# Note: When building with Oracle support, you need to pass:
#   --action_env=ORACLE_HOME=/path/to/oracle
#   --linkopt=-L$ORACLE_HOME/lib
#   --copt=-I$ORACLE_HOME/include
cc_library(
    name = "oracle_libs",
    linkopts = [
        "-locci",
        "-lclntsh",
        "-lnnz12",
    ],
)

# Configuration for Oracle support
# Enable with: bazel build --define oracle=true ...
config_setting(
    name = "oracle_enabled",
    define_values = {"oracle": "true"},
)
