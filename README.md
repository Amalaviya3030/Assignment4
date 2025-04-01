* Chat Application

A chat application that supports multiple clients and cross-machine communication.

* Features

- Multi-client support (up to 10 clients)
- ncurses-based UI with colored interface
- Real-time message delivery
- Username and IP display
- Message history (last 10 lines)
- Clean server and client termination
- Cross-machine communication support

* Requirements

- Linux environment (WSL or Linux VM)
- GCC compiler
- Make build system
- ncurses library
- pthread support

* Dependencies Installation

* On Ubuntu/Debian
sudo apt-get update
sudo apt-get install gcc make libncurses5-dev



* Building

make clean  # Clean previous builds
make        # Compile the application


* Running the Application

1. Start the server:

./bin/chat_server


2. Start client:

./bin/chat_client


3. Follow the prompts to:
   - Enter username
   - Enter server IP (default: 127.0.0.1)


* Server
- Press Ctrl+C to shutdown server
- Server displays all client messages with timestamps

* Client
- Type messages and press Enter to send
- Type `>>bye<<` to exit
- Messages are displayed with timestamps and usernames
- Last 10 lines of chat history are maintained

* Testing

1. Local Testing:

    Terminal 1
   ./bin/chat_server

    Terminal 2
   ./bin/chat_client
    Enter username and use localhost (127.0.0.1)

    Terminal 3
   ./bin/chat_client
    Enter different username and use localhost
   

2. Cross-machine Testing:
   - Start server on one machine
   - Note the server machine's IP address
   - Connect from other machines using server's IP

* Error Handling

- Server full notification (max 10 clients)
- Server disconnection detection
- Network error handling
- Clean termination with Ctrl+C

* Known Limitations

- Maximum 10 concurrent clients
- Messages limited to 1024 bytes
- Display limited to last 10 lines
- Requires Linux/Unix environment

* Author
[Aryankumar,uttam, anchita]

* Chat Server

This is the server component of the chat application.

* Files
- server.c: Main server implementation
- server_utils.c: Server utility functions
- server_utils.h: Server header file

* Features
- Multi-client support (up to 10 clients)
- Message broadcasting
- Client management
- Clean shutdown handling

* Implementation Details

* Threading
- Main thread for accepting connections
- Separate thread for each client


* Network
- TCP/IP socket server
- IPv4 support
- Port 8080 by default
- Connection queue management

* Message Handling
- Broadcast messages to all clients
- Username and IP tracking
- Timestamp addition


* Usage

./bin/chat_server


* Server Management
1. Server starts listening on all interfaces (0.0.0.0)
2. Accepts client connections up to MAX_CLIENTS
3. Manages client threads
4. Press Ctrl+C for clean shutdown

* Error Handling
- Client disconnection handling
- Full server handling
- Network error management
- Resource cleanup

* Limitations
- Maximum 10 concurrent clients
- Messages limited to 1024 bytes
- History limited to last 10 lines

* Author
[Aryankumar,uttam, anchita]

* Chat Client

This is the client component of the chat application.

* Files
- client.c: Main client implementation
- client_utils.c: Client utility functions
- client_utils.h: Client header file

* Features
- ncurses-based UI
- Real-time message display
- Username configuration
- IP address configuration
- Message history display
- Clean exit with `>>bye<<` command


### Network
- TCP/IP socket communication
- Server connection management
- Error handling for network issues

### UI Components
- Chat window for message display
- Input window for message typing
- Status messages and notifications

* Usage

./bin/chat_client


* Steps:
1. Enter username when prompted
2. Enter server IP address
3. Type messages and press Enter to send
4. Use `>>bye<<` to exit cleanly

* Error Handling
- Server disconnection detection
- Network error handling
- Invalid input handling

* Author
[Aryankumar,uttam, anchita]
