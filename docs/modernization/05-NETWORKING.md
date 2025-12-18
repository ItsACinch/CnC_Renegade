# Phase 5: Networking & Multiplayer Modernization

## Objective

Replace the defunct GameSpy SDK with modern alternatives for server browsing, authentication, and NAT traversal.

## Current GameSpy Integration

### Services Used
| Service | Purpose | Status |
|---------|---------|--------|
| Query & Reporting | Server browser listing | GameSpy shutdown 2014 |
| CD Key Auth | Player authentication | Service defunct |
| Master Server | Server discovery | Service defunct |
| NAT Negotiation | Firewall traversal | Via WOL, also defunct |

### Key Files
```
Code/Commando/
├── GameSpy_QnR.cpp/h        # Query & Reporting
├── gamespyadmin.cpp/h       # State management
├── gamespyauthmgr.cpp/h     # Authentication manager
├── CDKeyAuth.cpp            # CD key handling
├── GameSpyBanList.cpp/h     # Player banning
└── gamespysc*.cpp/h         # Network events
```

### Current Authentication Flow
```
Server Start → Register with Master Server
Client Connect → Server sends challenge
Client → Computes MD5 response with CD key
Server → Validates with GameSpy auth server
Result → Accept/Reject player
```

## Replacement Strategy

### Option A: Custom Master Server (Recommended)
- Full control over infrastructure
- Open-source server list protocol
- No external dependencies

### Option B: OpenSpy
- GameSpy-compatible replacement
- Community-maintained
- Drop-in replacement for GameSpy SDK

### Option C: Steam Integration
- Leverages existing Steam ownership
- Rich feature set (matchmaking, VAC, etc.)
- Requires Steamworks SDK integration

### Recommendation: Custom Master Server + LAN Discovery

## Implementation Plan

### 5.1 Server List Protocol

Simple JSON-based protocol over HTTP/HTTPS:

```cpp
// Server Registration
POST /api/servers
{
    "name": "My Server",
    "map": "C&C_Field",
    "players": 12,
    "maxPlayers": 32,
    "gameMode": "CNC",
    "password": false,
    "address": "auto",  // Server detects public IP
    "port": 4848,
    "version": "1.037"
}

// Server List
GET /api/servers
[
    {
        "id": "uuid",
        "name": "Server Name",
        "address": "1.2.3.4",
        "port": 4848,
        "map": "C&C_Field",
        "players": 12,
        "maxPlayers": 32,
        "ping": null,  // Client measures
        "gameMode": "CNC",
        "password": false
    }
]

// Heartbeat (every 60 seconds)
PUT /api/servers/{id}/heartbeat
{
    "players": 14,
    "map": "C&C_Walls_Flying"
}
```

### 5.2 Master Server Implementation

```cpp
// Code/Commando/MasterServerClient.h

class MasterServerClient {
    std::string masterServerUrl = "https://master.example.com";
    std::string serverId;
    HANDLE heartbeatThread = nullptr;
    bool running = false;

public:
    // Server-side
    bool RegisterServer(const ServerInfo& info);
    void UpdateServer(const ServerInfo& info);
    void UnregisterServer();

    // Client-side
    std::vector<ServerInfo> GetServerList();
    void RefreshServerList(std::function<void(std::vector<ServerInfo>)> callback);

private:
    static DWORD WINAPI HeartbeatThread(LPVOID param);
    std::string HttpRequest(const std::string& method, const std::string& path,
                           const std::string& body = "");
};
```

### 5.3 HTTP Client Using WinHTTP

```cpp
// Code/Commando/HttpClient.cpp

#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

std::string HttpClient::Request(const std::string& method,
                                const std::string& url,
                                const std::string& body) {
    URL_COMPONENTS urlComp = { sizeof(urlComp) };
    wchar_t hostName[256], urlPath[1024];
    urlComp.lpszHostName = hostName;
    urlComp.dwHostNameLength = 256;
    urlComp.lpszUrlPath = urlPath;
    urlComp.dwUrlPathLength = 1024;

    std::wstring wurl(url.begin(), url.end());
    WinHttpCrackUrl(wurl.c_str(), 0, 0, &urlComp);

    HINTERNET hSession = WinHttpOpen(L"Renegade/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, nullptr, nullptr, 0);

    HINTERNET hConnect = WinHttpConnect(hSession, hostName,
        urlComp.nPort, 0);

    std::wstring wmethod(method.begin(), method.end());
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, wmethod.c_str(),
        urlPath, nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
        urlComp.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0);

    // Send request
    WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        (LPVOID)body.c_str(), body.length(), body.length(), 0);

    WinHttpReceiveResponse(hRequest, nullptr);

    // Read response
    std::string response;
    DWORD bytesRead;
    char buffer[4096];
    while (WinHttpReadData(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        response.append(buffer, bytesRead);
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return response;
}
```

### 5.4 LAN Discovery (UDP Broadcast)

```cpp
// Code/Commando/LANDiscovery.h

class LANDiscovery {
    SOCKET broadcastSocket = INVALID_SOCKET;
    SOCKET listenSocket = INVALID_SOCKET;
    static const int DISCOVERY_PORT = 4849;

public:
    // Server broadcasts presence
    void BroadcastServer(const ServerInfo& info);

    // Client listens for broadcasts
    void StartListening();
    void StopListening();
    std::vector<ServerInfo> GetDiscoveredServers();

private:
    void BroadcastThread();
    void ListenThread();
};

// Broadcast packet structure
#pragma pack(push, 1)
struct LANBroadcastPacket {
    uint32_t magic = 0x52454E47;  // "RENG"
    uint16_t version = 1;
    uint16_t port;
    char serverName[64];
    char mapName[32];
    uint8_t players;
    uint8_t maxPlayers;
    uint8_t gameMode;
    uint8_t hasPassword;
};
#pragma pack(pop)
```

### 5.5 Player Authentication (Simplified)

Without GameSpy, authentication options:

#### Option A: No Authentication (LAN-style)
```cpp
// Just accept any player with valid client
bool AuthenticatePlayer(PlayerInfo& player) {
    return player.clientVersion == SERVER_VERSION;
}
```

#### Option B: Simple Password
```cpp
bool AuthenticatePlayer(PlayerInfo& player, const std::string& serverPassword) {
    if (serverPassword.empty()) return true;
    return player.password == serverPassword;
}
```

#### Option C: Account System (if running custom backend)
```cpp
bool AuthenticatePlayer(PlayerInfo& player) {
    // POST to auth server
    auto response = HttpClient::Post("/api/auth/validate", {
        {"token", player.authToken},
        {"name", player.name}
    });
    return response["valid"].get<bool>();
}
```

### 5.6 NAT Traversal

Modern approach using STUN/TURN:

```cpp
// Code/Commando/NATTraversal.h

class NATTraversal {
    std::string stunServer = "stun.l.google.com:19302";

public:
    // Determine NAT type and public address
    struct NATInfo {
        std::string publicIP;
        uint16_t publicPort;
        enum Type { Open, FullCone, Symmetric, Restricted } type;
    };

    NATInfo DiscoverNAT();

    // UDP hole punching for peer connection
    bool PunchHole(const std::string& peerIP, uint16_t peerPort);

    // For symmetric NAT, use relay
    bool ConnectViaRelay(const std::string& relayServer,
                         const std::string& sessionId);
};
```

### 5.7 Ban List (Keep Existing)

The existing `GameSpyBanList` can be retained with minimal changes:

```cpp
// Code/Commando/BanList.cpp

class BanList {
    struct BanEntry {
        enum Type { IP, Name, Hash } type;
        std::string value;
        bool allow;  // true = allow (whitelist), false = deny
    };

    std::vector<BanEntry> entries;

public:
    bool LoadFromFile(const char* filename);
    bool SaveToFile(const char* filename);

    bool IsPlayerBanned(const std::string& ip,
                        const std::string& name,
                        const std::string& hash);

    void BanPlayer(const std::string& value, BanEntry::Type type);
    void UnbanPlayer(const std::string& value, BanEntry::Type type);
};
```

### 5.8 Replace GameSpy Events

| Original Event | Replacement |
|---------------|-------------|
| `cGameSpyScChallengeEvent` | Remove or replace with simple auth |
| `cGameSpyCsChallengeResponseEvent` | Remove or replace with simple auth |

### 5.9 Server Query Protocol (Replace QnR)

Simple UDP query for server info:

```cpp
// Query packet (client sends)
struct QueryPacket {
    uint32_t magic = 0x51525950;  // "QRYP"
    uint8_t type;  // 0=basic, 1=rules, 2=players
};

// Response packet (server sends)
struct QueryResponse {
    uint32_t magic = 0x52535050;  // "RSPP"
    uint8_t type;
    // ... type-specific data follows
};
```

## Migration Path

### Phase 5a: Remove GameSpy Dependencies
1. Comment out GameSpy SDK includes
2. Stub GameSpy API calls
3. Verify game still compiles and runs (LAN only)

### Phase 5b: Implement LAN Discovery
1. Add UDP broadcast system
2. Update server browser UI
3. Test LAN multiplayer

### Phase 5c: Implement Master Server
1. Deploy master server (Node.js/Python/Go)
2. Add HTTP client to game
3. Add server registration
4. Add server list fetching
5. Test internet multiplayer

### Phase 5d: Optional Enhancements
1. NAT traversal
2. Player accounts
3. Statistics tracking

## Master Server Reference Implementation

Simple Node.js server:

```javascript
// master-server/index.js
const express = require('express');
const app = express();
app.use(express.json());

const servers = new Map();

app.post('/api/servers', (req, res) => {
    const id = crypto.randomUUID();
    const server = {
        ...req.body,
        id,
        address: req.ip,
        lastHeartbeat: Date.now()
    };
    servers.set(id, server);
    res.json({ id });
});

app.get('/api/servers', (req, res) => {
    const now = Date.now();
    const activeServers = [...servers.values()]
        .filter(s => now - s.lastHeartbeat < 120000);
    res.json(activeServers);
});

app.put('/api/servers/:id/heartbeat', (req, res) => {
    const server = servers.get(req.params.id);
    if (server) {
        Object.assign(server, req.body, { lastHeartbeat: Date.now() });
        res.json({ success: true });
    } else {
        res.status(404).json({ error: 'Server not found' });
    }
});

// Cleanup old servers every minute
setInterval(() => {
    const now = Date.now();
    for (const [id, server] of servers) {
        if (now - server.lastHeartbeat > 120000) {
            servers.delete(id);
        }
    }
}, 60000);

app.listen(3000);
```

## File Changes

| Original File | Action |
|--------------|--------|
| GameSpy_QnR.cpp/h | Remove |
| gamespyadmin.cpp/h | Remove |
| gamespyauthmgr.cpp/h | Remove |
| CDKeyAuth.cpp | Remove |
| gamespysc*.cpp/h | Remove |
| GameSpyBanList.cpp/h | Keep, rename to BanList |

## New Files

| File | Purpose |
|------|---------|
| MasterServerClient.cpp/h | Master server communication |
| LANDiscovery.cpp/h | LAN game discovery |
| HttpClient.cpp/h | HTTP requests |
| BanList.cpp/h | Renamed ban list |

## Verification

1. LAN games discoverable and joinable
2. Internet games listed in server browser
3. Server registration and heartbeat working
4. Player banning still functions
5. No connection issues through firewalls (basic)

## Estimated Effort

- Remove GameSpy code: 1-2 days
- LAN discovery: 2-3 days
- Master server client: 3-4 days
- Master server deployment: 1-2 days
- Testing: 3-4 days
- **Total: 10-15 days**

## Risks

1. **NAT traversal complexity**: Full NAT traversal is non-trivial
2. **Server hosting**: Need to maintain master server
3. **DDoS protection**: Public master server needs protection
4. **Legacy client compatibility**: Changes break old clients
