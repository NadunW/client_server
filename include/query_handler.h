#ifndef QUERY_HANDLER_H
#define QUERY_HANDLER_H

#include <string>
#include <map>
#include "database.h"

// Query IDs - predefined queries to prevent SQL injection
enum QueryId {
    QUERY_GET_ALL_USERS = 1,
    QUERY_GET_USER_BY_ID = 2,
    QUERY_GET_USERS_BY_DEPARTMENT = 3,
    QUERY_GET_PRODUCT_INFO = 4,
    QUERY_GET_ORDER_STATUS = 5
};

class QueryHandler {
public:
    QueryHandler(Database* db);
    
    // Handle client request - parses request and executes appropriate query
    std::string handleRequest(const std::string& request);
    
private:
    Database* database;
    
    // Get SQL query template for a given query ID
    std::string getQueryTemplate(int queryId);
    
    // Parse request string into query ID and parameters
    bool parseRequest(const std::string& request, 
                     int& queryId, 
                     std::map<std::string, std::string>& params);
    
    // Validate query ID
    bool isValidQueryId(int queryId);
};

#endif // QUERY_HANDLER_H
