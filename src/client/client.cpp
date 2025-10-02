#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

class ServerClient {
public:
    ServerClient() : sockfd(-1), connected(false) {}
    
    ~ServerClient() {
        disconnect();
    }
    
    bool connect(const std::string& host, int port) {
        // Create socket
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            std::cerr << "Error creating socket." << std::endl;
            return false;
        }
        
        // Setup server address
        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        
        if (inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr) <= 0) {
            std::cerr << "Invalid address / Address not supported." << std::endl;
            close(sockfd);
            sockfd = -1;
            return false;
        }
        
        // Connect to server
        if (::connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            std::cerr << "Connection failed. Is the server running?" << std::endl;
            close(sockfd);
            sockfd = -1;
            return false;
        }
        
        connected = true;
        std::cout << "Connected to server at " << host << ":" << port << std::endl;
        return true;
    }
    
    void disconnect() {
        if (sockfd >= 0) {
            close(sockfd);
            sockfd = -1;
            connected = false;
            std::cout << "Disconnected from server." << std::endl;
        }
    }
    
    std::string sendRequest(const std::string& request) {
        if (!connected) {
            return "ERROR: Not connected to server";
        }
        
        // Send request
        if (send(sockfd, request.c_str(), request.length(), 0) < 0) {
            std::cerr << "Error sending request." << std::endl;
            return "ERROR: Failed to send request";
        }
        
        // Receive response
        char buffer[8192];
        memset(buffer, 0, sizeof(buffer));
        
        int bytesRead = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead < 0) {
            std::cerr << "Error receiving response." << std::endl;
            return "ERROR: Failed to receive response";
        }
        
        return std::string(buffer);
    }
    
    bool isConnected() const { return connected; }
    
private:
    int sockfd;
    bool connected;
};

void printUsage() {
    std::cout << "\n=== Client Usage ===" << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  1 - Get all users" << std::endl;
    std::cout << "  2 <user_id> - Get user by ID" << std::endl;
    std::cout << "  3 <department> - Get users by department" << std::endl;
    std::cout << "  4 <product_id> - Get product info" << std::endl;
    std::cout << "  5 <order_id> - Get order status" << std::endl;
    std::cout << "  help - Show this help" << std::endl;
    std::cout << "  quit - Disconnect and exit" << std::endl;
    std::cout << "\nNote: Client sends query IDs and parameters, NOT raw SQL." << std::endl;
    std::cout << "      This prevents SQL injection attacks.\n" << std::endl;
}

void parseResultAndPrint(const std::string& response) {
    if (response.substr(0, 6) == "ERROR:") {
        std::cout << "\n" << response << "\n" << std::endl;
        return;
    }
    
    if (response.substr(0, 7) != "RESULT|") {
        std::cout << "\nUnexpected response format: " << response << "\n" << std::endl;
        return;
    }
    
    // Parse response format: RESULT|<headers>|<rows>|<count>
    size_t pos1 = 7; // After "RESULT|"
    size_t pos2 = response.find('|', pos1);
    
    if (pos2 == std::string::npos) {
        std::cout << "\nInvalid response format\n" << std::endl;
        return;
    }
    
    std::string headers = response.substr(pos1, pos2 - pos1);
    
    pos1 = pos2 + 1;
    pos2 = response.find('|', pos1);
    
    if (pos2 == std::string::npos) {
        std::cout << "\nInvalid response format\n" << std::endl;
        return;
    }
    
    std::string rows = response.substr(pos1, pos2 - pos1);
    std::string count = response.substr(pos2 + 1);
    
    std::cout << "\n=== Query Results ===" << std::endl;
    std::cout << "Columns: " << headers << std::endl;
    std::cout << "---" << std::endl;
    
    if (rows.empty() || count == "0") {
        std::cout << "No results found." << std::endl;
    } else {
        // Split rows by semicolon
        size_t rowStart = 0;
        int rowNum = 1;
        while (rowStart < rows.length()) {
            size_t rowEnd = rows.find(';', rowStart);
            if (rowEnd == std::string::npos) break;
            
            std::string row = rows.substr(rowStart, rowEnd - rowStart);
            if (!row.empty()) {
                std::cout << "Row " << rowNum++ << ": " << row << std::endl;
            }
            
            rowStart = rowEnd + 1;
        }
    }
    
    std::cout << "---" << std::endl;
    std::cout << "Total rows: " << count << "\n" << std::endl;
}

std::string buildRequest(const std::string& input) {
    std::istringstream iss(input);
    std::string cmd;
    iss >> cmd;
    
    if (cmd == "1") {
        // Get all users - no parameters needed
        return "QUERY|1";
    } else if (cmd == "2") {
        // Get user by ID
        std::string userId;
        iss >> userId;
        if (userId.empty()) {
            std::cout << "Error: user_id required. Usage: 2 <user_id>" << std::endl;
            return "";
        }
        return "QUERY|2|user_id=" + userId;
    } else if (cmd == "3") {
        // Get users by department
        std::string dept;
        iss >> dept;
        if (dept.empty()) {
            std::cout << "Error: department required. Usage: 3 <department>" << std::endl;
            return "";
        }
        return "QUERY|3|department=" + dept;
    } else if (cmd == "4") {
        // Get product info
        std::string productId;
        iss >> productId;
        if (productId.empty()) {
            std::cout << "Error: product_id required. Usage: 4 <product_id>" << std::endl;
            return "";
        }
        return "QUERY|4|product_id=" + productId;
    } else if (cmd == "5") {
        // Get order status
        std::string orderId;
        iss >> orderId;
        if (orderId.empty()) {
            std::cout << "Error: order_id required. Usage: 5 <order_id>" << std::endl;
            return "";
        }
        return "QUERY|5|order_id=" + orderId;
    } else {
        std::cout << "Unknown command. Type 'help' for usage." << std::endl;
        return "";
    }
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    std::string host = "127.0.0.1";
    int port = 8080;
    
    if (argc > 1) {
        host = argv[1];
    }
    if (argc > 2) {
        try {
            port = std::stoi(argv[2]);
        } catch (...) {
            std::cerr << "Invalid port number. Using default: 8080" << std::endl;
            port = 8080;
        }
    }
    
    std::cout << "=== Database Client ===" << std::endl;
    std::cout << "Connecting to server at " << host << ":" << port << "..." << std::endl;
    
    ServerClient client;
    
    if (!client.connect(host, port)) {
        return 1;
    }
    
    printUsage();
    
    // Interactive loop
    std::string input;
    while (client.isConnected()) {
        std::cout << "client> ";
        std::getline(std::cin, input);
        
        // Trim whitespace
        input.erase(0, input.find_first_not_of(" \t\n\r"));
        input.erase(input.find_last_not_of(" \t\n\r") + 1);
        
        if (input.empty()) {
            continue;
        }
        
        if (input == "quit" || input == "exit") {
            break;
        }
        
        if (input == "help") {
            printUsage();
            continue;
        }
        
        // Build request from user input
        std::string request = buildRequest(input);
        
        if (request.empty()) {
            continue;
        }
        
        // Send request and get response
        std::string response = client.sendRequest(request);
        
        // Parse and display results
        parseResultAndPrint(response);
    }
    
    client.disconnect();
    std::cout << "Goodbye!" << std::endl;
    
    return 0;
}
