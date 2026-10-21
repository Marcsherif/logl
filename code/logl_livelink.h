#if !defined(LOGL_LIVELINK_H)
#define LOGL_LIVELINK_H

struct Live_Link_State
{
    SOCKET listen_socket = INVALID_SOCKET;
    SOCKET client_socket = INVALID_SOCKET;
    bool connected = false;
    bool initialized = false;
};

#endif
