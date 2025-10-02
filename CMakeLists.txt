cmake_minimum_required(VERSION 3.10)
project(ClientServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Oracle Instant Client (OCCI)
# Users need to set ORACLE_HOME environment variable or provide path
set(ORACLE_HOME $ENV{ORACLE_HOME})

if(ORACLE_HOME)
    message(STATUS "Oracle Home: ${ORACLE_HOME}")
    include_directories(${ORACLE_HOME}/include)
    link_directories(${ORACLE_HOME}/lib)
    set(ORACLE_LIBS occi clntsh nnz12)
    set(ORACLE_FOUND TRUE)
else()
    message(WARNING "ORACLE_HOME not set. Building without Oracle support. Set ORACLE_HOME to enable database features.")
    set(ORACLE_FOUND FALSE)
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

# Server application
add_executable(server 
    src/server/server.cpp
    src/server/database.cpp
    src/server/query_handler.cpp
)

if(ORACLE_FOUND)
    target_link_libraries(server ${ORACLE_LIBS} pthread)
    target_compile_definitions(server PRIVATE ORACLE_ENABLED)
else()
    target_link_libraries(server pthread)
endif()

# Client application
add_executable(client 
    src/client/client.cpp
)

target_link_libraries(client pthread)
