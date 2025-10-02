#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <map>

#ifdef ORACLE_ENABLED
#include <occi.h>
#endif

class Database {
public:
    Database();
    ~Database();
    
    // Initialize database connection
    bool connect(const std::string& username, 
                 const std::string& password, 
                 const std::string& connectionString);
    
    // Disconnect from database
    void disconnect();
    
    // Execute a predefined query by ID with parameters
    std::string executeQuery(int queryId, const std::map<std::string, std::string>& params);
    
    // Check if connected
    bool isConnected() const { return connected; }
    
private:
    bool connected;
    
#ifdef ORACLE_ENABLED
    oracle::occi::Environment* env;
    oracle::occi::Connection* conn;
    
    std::string executeOracleQuery(const std::string& sql, 
                                   const std::map<std::string, std::string>& params);
#endif
    
    // Mock query execution for when Oracle is not available
    std::string executeMockQuery(int queryId, const std::map<std::string, std::string>& params);
};

#endif // DATABASE_H
