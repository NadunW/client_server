#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include <csignal>
#include "database.h"
#include "query_handler.h"

// Global flag for graceful shutdown
volatile sig_atomic_t running = 1;

void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ". Shutting down gracefully..." << std::endl;
    running = 0;
}

void handleClient(int clientSocket, QueryHandler* queryHandler) {
    char buffer[4096];
    
    std::cout << "Client connected." << std::endl;
    
    while (running) {
        memset(buffer, 0, sizeof(buffer));
        
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytesRead <= 0) {
            if (bytesRead == 0) {
                std::cout << "Client disconnected." << std::endl;
            } else {
                std::cerr << "Error receiving data from client." << std::endl;
            }
            break;
        }
        
        std::string request(buffer);
        
        // Handle special commands
        if (request == "EXIT" || request == "QUIT") {
            std::cout << "Client requested disconnect." << std::endl;
            break;
        }
        
        // Process the request
        std::string response = queryHandler->handleRequest(request);
        
        // Send response back to client
        send(clientSocket, response.c_str(), response.length(), 0);
    }
    
    close(clientSocket);
    std::cout << "Client connection closed." << std::endl;
}

void printUsage() {
    std::cout << "\nServer Usage:" << std::endl;
    std::cout << "  ./server [port]" << std::endl;
    std::cout << "\nDefault port: 8080" << std::endl;
    std::cout << "\nSupported Query IDs:" << std::endl;
    std::cout << "  1 - Get all users" << std::endl;
    std::cout << "  2 - Get user by ID (requires user_id parameter)" << std::endl;
    std::cout << "  3 - Get users by department (requires department parameter)" << std::endl;
    std::cout << "  4 - Get product info (requires product_id parameter)" << std::endl;
    std::cout << "  5 - Get order status (requires order_id parameter)" << std::endl;
    std::cout << "\nRequest format: QUERY|<id>|<param1>=<value1>|<param2>=<value2>" << std::endl;
}

int main(int argc, char* argv[]) {
    // Setup signal handlers for graceful shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // Parse command line arguments
    int port = 8080;
    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
        } catch (...) {
            std::cerr << "Invalid port number. Using default: 8080" << std::endl;
            port = 8080;
        }
    }
    
    printUsage();
    
    // Initialize database
    Database db;
    
    // Try to connect to Oracle database if available
    // Users should set these as environment variables or configure appropriately
    const char* dbUser = std::getenv("DB_USER");
    const char* dbPass = std::getenv("DB_PASS");
    const char* dbConnStr = std::getenv("DB_CONN_STRING");
    
    if (dbUser && dbPass && dbConnStr) {
        std::cout << "\nAttempting to connect to Oracle database..." << std::endl;
        if (!db.connect(dbUser, dbPass, dbConnStr)) {
            std::cerr << "Failed to connect to Oracle database. Using mock data." << std::endl;
        }
    } else {
        std::cout << "\nDatabase credentials not provided. Using mock database." << std::endl;
        std::cout << "Set DB_USER, DB_PASS, and DB_CONN_STRING environment variables to use Oracle." << std::endl;
        db.connect("", "", ""); // Will use mock database
    }
    
    // Create query handler
    QueryHandler queryHandler(&db);
    
    // Create socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Error creating socket." << std::endl;
        return 1;
    }
    
    // Set socket options to reuse address
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Error setting socket options." << std::endl;
        close(serverSocket);
        return 1;
    }
    
    // Bind socket
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error binding socket to port " << port << std::endl;
        close(serverSocket);
        return 1;
    }
    
    // Listen for connections
    if (listen(serverSocket, 5) < 0) {
        std::cerr << "Error listening on socket." << std::endl;
        close(serverSocket);
        return 1;
    }
    
    std::cout << "\nServer listening on port " << port << "..." << std::endl;
    std::cout << "Press Ctrl+C to stop the server.\n" << std::endl;
    
    // Accept client connections
    std::vector<std::thread> clientThreads;
    
    while (running) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        
        // Set socket to non-blocking for accept so we can check running flag
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        
        if (clientSocket < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                // Timeout - check if we should continue running
                continue;
            }
            if (running) {
                std::cerr << "Error accepting client connection." << std::endl;
            }
            continue;
        }
        
        // Handle client in a new thread
        clientThreads.push_back(std::thread(handleClient, clientSocket, &queryHandler));
    }
    
    // Wait for all client threads to finish
    for (auto& thread : clientThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    // Cleanup
    close(serverSocket);
    
    std::cout << "Server shut down successfully." << std::endl;
    
    return 0;
}
