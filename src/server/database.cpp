#include "database.h"
#include <iostream>
#include <sstream>

Database::Database() : connected(false) {
#ifdef ORACLE_ENABLED
    env = nullptr;
    conn = nullptr;
#endif
}

Database::~Database() {
    disconnect();
}

bool Database::connect(const std::string& username, 
                      const std::string& password, 
                      const std::string& connectionString) {
#ifdef ORACLE_ENABLED
    try {
        std::cout << "Connecting to Oracle database..." << std::endl;
        env = oracle::occi::Environment::createEnvironment(
            oracle::occi::Environment::DEFAULT);
        
        conn = env->createConnection(username, password, connectionString);
        
        if (conn) {
            connected = true;
            std::cout << "Successfully connected to Oracle database." << std::endl;
            return true;
        }
    } catch (oracle::occi::SQLException& ex) {
        std::cerr << "Oracle connection error: " << ex.getMessage() << std::endl;
        connected = false;
        return false;
    }
#else
    std::cout << "Oracle support not enabled. Using mock database." << std::endl;
    connected = true;
    return true;
#endif
    return false;
}

void Database::disconnect() {
    if (!connected) return;
    
#ifdef ORACLE_ENABLED
    if (conn) {
        env->terminateConnection(conn);
        conn = nullptr;
    }
    if (env) {
        oracle::occi::Environment::terminateEnvironment(env);
        env = nullptr;
    }
#endif
    
    connected = false;
    std::cout << "Disconnected from database." << std::endl;
}

std::string Database::executeQuery(int queryId, 
                                   const std::map<std::string, std::string>& params) {
    if (!connected) {
        return "ERROR: Not connected to database";
    }
    
#ifdef ORACLE_ENABLED
    std::string sql;
    
    // Predefined queries - prevents SQL injection
    switch (queryId) {
        case 1: // QUERY_GET_ALL_USERS
            sql = "SELECT user_id, username, email FROM users";
            break;
        case 2: // QUERY_GET_USER_BY_ID
            sql = "SELECT user_id, username, email FROM users WHERE user_id = :1";
            break;
        case 3: // QUERY_GET_USERS_BY_DEPARTMENT
            sql = "SELECT user_id, username, department FROM users WHERE department = :1";
            break;
        case 4: // QUERY_GET_PRODUCT_INFO
            sql = "SELECT product_id, name, price, stock FROM products WHERE product_id = :1";
            break;
        case 5: // QUERY_GET_ORDER_STATUS
            sql = "SELECT order_id, status, total_amount FROM orders WHERE order_id = :1";
            break;
        default:
            return "ERROR: Invalid query ID";
    }
    
    return executeOracleQuery(sql, params);
#else
    return executeMockQuery(queryId, params);
#endif
}

#ifdef ORACLE_ENABLED
std::string Database::executeOracleQuery(const std::string& sql, 
                                        const std::map<std::string, std::string>& params) {
    try {
        oracle::occi::Statement* stmt = conn->createStatement(sql);
        
        // Bind parameters
        int bindIndex = 1;
        for (const auto& param : params) {
            stmt->setString(bindIndex++, param.second);
        }
        
        oracle::occi::ResultSet* rs = stmt->executeQuery();
        
        std::ostringstream result;
        result << "RESULT|";
        
        // Get metadata
        std::vector<oracle::occi::MetaData> metadata = rs->getColumnListMetaData();
        int colCount = metadata.size();
        
        // Column headers
        for (int i = 1; i <= colCount; i++) {
            result << metadata[i-1].getString(oracle::occi::MetaData::ATTR_NAME);
            if (i < colCount) result << ",";
        }
        result << "|";
        
        // Rows
        int rowCount = 0;
        while (rs->next()) {
            for (int i = 1; i <= colCount; i++) {
                result << rs->getString(i);
                if (i < colCount) result << ",";
            }
            result << ";";
            rowCount++;
        }
        
        result << "|" << rowCount;
        
        stmt->closeResultSet(rs);
        conn->terminateStatement(stmt);
        
        return result.str();
        
    } catch (oracle::occi::SQLException& ex) {
        std::ostringstream error;
        error << "ERROR: Query execution failed: " << ex.getMessage();
        return error.str();
    }
}
#endif

std::string Database::executeMockQuery(int queryId, 
                                       const std::map<std::string, std::string>& params) {
    std::ostringstream result;
    result << "RESULT|";
    
    switch (queryId) {
        case 1: // QUERY_GET_ALL_USERS
            result << "user_id,username,email|";
            result << "1,john_doe,john@example.com;";
            result << "2,jane_smith,jane@example.com;";
            result << "3,bob_jones,bob@example.com;";
            result << "|3";
            break;
            
        case 2: // QUERY_GET_USER_BY_ID
            result << "user_id,username,email|";
            if (params.count("user_id")) {
                result << params.at("user_id") << ",john_doe,john@example.com;";
                result << "|1";
            } else {
                result << "|0";
            }
            break;
            
        case 3: // QUERY_GET_USERS_BY_DEPARTMENT
            result << "user_id,username,department|";
            if (params.count("department")) {
                result << "1,john_doe," << params.at("department") << ";";
                result << "2,jane_smith," << params.at("department") << ";";
                result << "|2";
            } else {
                result << "|0";
            }
            break;
            
        case 4: // QUERY_GET_PRODUCT_INFO
            result << "product_id,name,price,stock|";
            if (params.count("product_id")) {
                result << params.at("product_id") << ",Sample Product,99.99,50;";
                result << "|1";
            } else {
                result << "|0";
            }
            break;
            
        case 5: // QUERY_GET_ORDER_STATUS
            result << "order_id,status,total_amount|";
            if (params.count("order_id")) {
                result << params.at("order_id") << ",SHIPPED,299.99;";
                result << "|1";
            } else {
                result << "|0";
            }
            break;
            
        default:
            return "ERROR: Invalid query ID";
    }
    
    return result.str();
}
