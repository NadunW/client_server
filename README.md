# Client-Server C++ Application with Oracle Database

A secure client-server architecture implemented in C++ that demonstrates best practices for preventing SQL injection attacks. The client sends query identifiers and parameters (not raw SQL) to the server, which executes predefined queries against an Oracle database.

## Features

- **Secure Query Execution**: Client sends query IDs and parameters, preventing SQL injection
- **Oracle Database Support**: Uses Oracle OCCI (C++ Call Interface) when available
- **Mock Database Mode**: Works without Oracle installation for testing and development
- **Multi-threaded Server**: Handles multiple concurrent client connections
- **Socket-based Communication**: TCP/IP networking for client-server interaction
- **Predefined Query Library**: Server maintains a whitelist of allowed queries

## Architecture

### Server (Backend)
- Accepts TCP socket connections from clients
- Parses incoming requests containing query IDs and parameters
- Executes predefined SQL queries against Oracle database
- Returns formatted results to clients
- Prevents SQL injection by never accepting raw SQL from clients

### Client (Frontend)
- Connects to server via TCP socket
- Provides interactive command-line interface
- Sends structured requests with query IDs and parameters
- Displays formatted query results
- Cannot send arbitrary SQL queries

## Security Design

The application prevents SQL injection through several mechanisms:

1. **Query ID System**: Clients reference queries by numeric ID, not SQL text
2. **Predefined Queries**: Server maintains a whitelist of allowed queries
3. **Parameter Binding**: Uses parameterized queries (prepared statements)
4. **No Raw SQL**: Client has no ability to send custom SQL statements

## Prerequisites

### Required
- C++17 compatible compiler (g++ 7.0+ or clang++ 5.0+)
- Bazel 5.0 or higher
- POSIX-compliant system (Linux, macOS, or Unix-like)

### Optional (for Oracle Database)
- Oracle Instant Client 12.1 or higher
- Oracle OCCI libraries
- Set `ORACLE_HOME` environment variable

## Building the Project

### Without Oracle Support (Mock Database)
```bash
bazel build //:server //:client
```

### With Oracle Support
```bash
# Set ORACLE_HOME to point to Oracle Instant Client
export ORACLE_HOME=/path/to/oracle/instantclient

# Build with Oracle support enabled (using .bazelrc config)
bazel build --config=oracle //:server //:client \
  --action_env=ORACLE_HOME=$ORACLE_HOME \
  --linkopt=-L$ORACLE_HOME/lib \
  --copt=-I$ORACLE_HOME/include
```

This creates two executables:
- `bazel-bin/server` - Backend server application
- `bazel-bin/client` - Frontend client application

## Running the Applications

### Start the Server

**Without Oracle (Mock Mode):**
```bash
./bazel-bin/server [port]
```

**With Oracle Database:**
```bash
export DB_USER=your_username
export DB_PASS=your_password
export DB_CONN_STRING=your_connection_string
./bazel-bin/server [port]
```

Default port is 8080.

Example connection strings:
- `localhost:1521/ORCL`
- `//hostname:1521/servicename`

### Start the Client

```bash
./bazel-bin/client [host] [port]
```

Default: `127.0.0.1` port `8080`

## Available Queries

The server supports the following predefined queries:

| Query ID | Description | Parameters | Example |
|----------|-------------|------------|---------|
| 1 | Get all users | None | `1` |
| 2 | Get user by ID | `user_id` | `2 123` |
| 3 | Get users by department | `department` | `3 Engineering` |
| 4 | Get product info | `product_id` | `4 456` |
| 5 | Get order status | `order_id` | `5 789` |

## Usage Examples

### Client Commands

```
client> 1
# Gets all users

client> 2 101
# Gets user with ID 101

client> 3 Sales
# Gets all users in Sales department

client> 4 2050
# Gets product with ID 2050

client> 5 3001
# Gets order with ID 3001

client> help
# Shows available commands

client> quit
# Disconnects and exits
```

### Request Protocol

Requests follow this format:
```
QUERY|<query_id>|<param1>=<value1>|<param2>=<value2>|...
```

Examples:
- `QUERY|1|` - Get all users (no parameters)
- `QUERY|2|user_id=123|` - Get user by ID
- `QUERY|3|department=Engineering|` - Get users by department

### Response Protocol

Successful responses:
```
RESULT|<column_headers>|<row_data>|<row_count>
```

Example:
```
RESULT|user_id,username,email|1,john_doe,john@example.com;2,jane_smith,jane@example.com;|2
```

Error responses:
```
ERROR: <error_message>
```

## Database Schema (Oracle)

For Oracle database mode, create tables with this schema:

```sql
-- Users table
CREATE TABLE users (
    user_id NUMBER PRIMARY KEY,
    username VARCHAR2(50),
    email VARCHAR2(100),
    department VARCHAR2(50)
);

-- Products table
CREATE TABLE products (
    product_id NUMBER PRIMARY KEY,
    name VARCHAR2(100),
    price NUMBER(10,2),
    stock NUMBER
);

-- Orders table
CREATE TABLE orders (
    order_id NUMBER PRIMARY KEY,
    status VARCHAR2(20),
    total_amount NUMBER(10,2)
);
```

## Extending the Application

### Adding New Queries

1. Add a new enum value in `include/query_handler.h`:
```cpp
enum QueryId {
    // ... existing queries
    QUERY_NEW_FEATURE = 6
};
```

2. Add SQL query in `src/server/database.cpp` in `executeQuery()`:
```cpp
case 6: // QUERY_NEW_FEATURE
    sql = "SELECT * FROM your_table WHERE condition = :1";
    break;
```

3. Add mock data in `executeMockQuery()` for testing without Oracle

4. Update client UI in `src/client/client.cpp` to support new command

## Troubleshooting

### Server won't start
- Check if port is already in use: `netstat -an | grep 8080`
- Try a different port: `./bazel-bin/server 9090`

### Client can't connect
- Verify server is running
- Check firewall settings
- Ensure correct host/port

### Oracle connection fails
- Verify `ORACLE_HOME` is set correctly
- Check Oracle Instant Client is installed
- Validate database credentials
- Test connection string with SQLPlus

### Build errors with Oracle
- Ensure OCCI libraries are in `$ORACLE_HOME/lib`
- Headers should be in `$ORACLE_HOME/include`
- Try building without Oracle first to isolate the issue

## Project Structure

```
client_server/
├── WORKSPACE                # Bazel workspace configuration
├── BUILD                    # Bazel build configuration
├── README.md               # This file
├── include/                # Header files
│   ├── database.h         # Database abstraction layer
│   └── query_handler.h    # Request parsing and query routing
└── src/
    ├── server/            # Server application
    │   ├── server.cpp    # Main server with socket handling
    │   ├── database.cpp  # Database implementation
    │   └── query_handler.cpp # Request handler implementation
    └── client/            # Client application
        └── client.cpp     # Interactive client interface
```

## License

This project is provided as-is for educational and development purposes.

## Security Notes

- This application demonstrates SQL injection prevention techniques
- Always use prepared statements/parameterized queries in production
- Keep database credentials secure (use environment variables, not hardcoded values)
- Consider using TLS/SSL for production deployments to encrypt network traffic
- Implement authentication and authorization for production use
- Add input validation for all parameters
- Use connection pooling for better performance in production