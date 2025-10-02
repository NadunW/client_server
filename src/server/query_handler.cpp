#include "query_handler.h"
#include <sstream>
#include <iostream>

QueryHandler::QueryHandler(Database* db) : database(db) {}

std::string QueryHandler::handleRequest(const std::string& request) {
    int queryId;
    std::map<std::string, std::string> params;
    
    std::cout << "Handling request: " << request << std::endl;
    
    if (!parseRequest(request, queryId, params)) {
        return "ERROR: Invalid request format. Expected: QUERY|<id>|<key1>=<value1>|<key2>=<value2>|...";
    }
    
    if (!isValidQueryId(queryId)) {
        return "ERROR: Invalid query ID";
    }
    
    if (!database->isConnected()) {
        return "ERROR: Database not connected";
    }
    
    return database->executeQuery(queryId, params);
}

std::string QueryHandler::getQueryTemplate(int queryId) {
    switch (queryId) {
        case QUERY_GET_ALL_USERS:
            return "Get all users from the database";
        case QUERY_GET_USER_BY_ID:
            return "Get user by ID (requires: user_id)";
        case QUERY_GET_USERS_BY_DEPARTMENT:
            return "Get users by department (requires: department)";
        case QUERY_GET_PRODUCT_INFO:
            return "Get product information (requires: product_id)";
        case QUERY_GET_ORDER_STATUS:
            return "Get order status (requires: order_id)";
        default:
            return "Unknown query";
    }
}

bool QueryHandler::parseRequest(const std::string& request, 
                                int& queryId, 
                                std::map<std::string, std::string>& params) {
    params.clear();
    
    // Expected format: QUERY|<queryId>|<param1>=<value1>|<param2>=<value2>|...
    std::istringstream iss(request);
    std::string token;
    
    // First token should be "QUERY"
    if (!std::getline(iss, token, '|') || token != "QUERY") {
        return false;
    }
    
    // Second token is the query ID
    if (!std::getline(iss, token, '|')) {
        return false;
    }
    
    try {
        queryId = std::stoi(token);
    } catch (...) {
        return false;
    }
    
    // Remaining tokens are parameters in key=value format
    while (std::getline(iss, token, '|')) {
        size_t eqPos = token.find('=');
        if (eqPos != std::string::npos) {
            std::string key = token.substr(0, eqPos);
            std::string value = token.substr(eqPos + 1);
            params[key] = value;
        }
    }
    
    return true;
}

bool QueryHandler::isValidQueryId(int queryId) {
    return queryId >= QUERY_GET_ALL_USERS && queryId <= QUERY_GET_ORDER_STATUS;
}
