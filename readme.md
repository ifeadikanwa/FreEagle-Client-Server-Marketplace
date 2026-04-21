# FreEagle – Client-Server Marketplace System

FreEagle is a TCP-based client-server application that allows users to post and request items, view listings, and receive real-time notifications when matching items become available.

---

## Overview

This project simulates a distributed marketplace system where multiple clients communicate with a central server using sockets.

Users can:
- Offer items
- Request items by category
- View available listings
- Remove offers or requests
- Receive real-time notifications when matching items are posted

---

## Key Features

- TCP client-server architecture
- Custom message protocol using C structs
- Real-time notifications using persistent sockets
- Multi-process client (fork-based notification listener)
- File-based persistence for offers and requests
- Category-based matching system
- Unique ID generation for offers

---

## System Architecture

### Server
- Handles all client requests
- Stores data in flat files
- Maintains active notifier connections
- Sends notifications to matching users

### Client
- Menu-driven interface
- Sends requests to server
- Runs a background process to receive notifications

---

## Core Concepts Demonstrated

- Socket programming (TCP)
- Concurrent processing using `fork()`
- Struct-based network communication
- File-based persistence (simulated database)
- State management across sessions
- Event-driven notification system

---

## File Structure

```
freeagle/
│
├── freeagle_server.c # Main server
├── handle_client.c # Request handler
├── freeagle_client.c # Client application
├── notification_manager.c # Notification system
│
├── shared.h # Shared structs and enums
├── quit_with_error.c # Error handling
├── quit_with_error.h
│
├── save_offer_to_file_db.c
├── save_request_to_file_db.c
├── remove_offer_from_file_db.c
├── remove_request_from_file_db.c
├── get_offer_list.c
├── get_next_offer_id.c
├── convert_category_to_string.c
│
├── offers_db.txt # Stored offers
├── requests_db.txt # Stored requests
├── next_offer_id.txt # Offer ID counter
│
├── README.md
```

---

## How It Works

### Communication Protocol

Clients and server communicate using structured messages defined in `shared.h`.

Example request types:
- `login`
- `offer`
- `request`
- `getList`
- `removeOffer`
- `logout`

Responses include:
- `ackOffer`
- `ackList`
- `notification`

---

### Real-Time Notifications

- Client forks after login
- Child process maintains a persistent socket
- Server tracks active notifier sockets
- When a new offer is created:
  - Server checks matching requests
  - Sends notifications to relevant users

---

### Persistence Layer

Instead of a database, the system uses text files:

- `offers_db.txt` → stores offers  
- `requests_db.txt` → stores user requests  
- `next_offer_id.txt` → tracks unique IDs  

Example save logic:
- Offers are appended to file  
- Requests are deduplicated before saving 

---

## How to Compile

```bash
# Server
gcc freeagle_server.c handle_client.c notification_manager.c \
save_request_to_file_db.c save_offer_to_file_db.c \
remove_request_from_file_db.c remove_offer_from_file_db.c \
get_offer_list.c get_next_offer_id.c convert_category_to_string.c \
quit_with_error.c -o server

# Client
gcc freeagle_client.c convert_category_to_string.c quit_with_error.c -o client
```

## How to Run

```bash
# Start server
./server 8000

# Run client
./client 127.0.0.1 8000
```

## Example Flow

1. User logs in
2. User requests "books"
3. Another user posts a book offer
4. First user instantly receives notification


## Limitations

- No authentication
- File-based storage (not scalable)
- No concurrency handling on file writes
- No encryption (plain TCP)


## Future Improvements

- Replace file storage with a database (PostgreSQL)
- Add authentication system
- Use threads or async I/O instead of fork
- Add REST API layer
- Improve input validation
- Add UI (web or mobile frontend)

## Why This Project Matters

This project demonstrates:
- Strong understanding of systems programming
- Real-world networking concepts
- State management across distributed components
- Building a full client-server system from scratch

## Author

Ify Eze