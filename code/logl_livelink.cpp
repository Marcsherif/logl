#pragma pack(push, 1)
struct live_link_packet
{
    int boneIndex;
    float matrix[16];
};
#pragma pack(pop)

struct network_state
{
    SOCKET listenSocket;
    SOCKET clientSocket;
    bool connected;
};

internal network_state
InitNetwork(int port)
{
    network_state state = {};
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return state;

    state.listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Set to non-blocking mode
    u_long mode = 1;
    ioctlsocket(state.listenSocket, FIONBIO, &mode);

    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons((u_short)port);

    bind(state.listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(state.listenSocket, SOMAXCONN);

    return state;
}

internal void
UpdateLiveLink(network_state *net, gltf_model *model)
{
    if (!net->connected)
    {
        net->clientSocket = accept(net->listenSocket, NULL, NULL);
        if (net->clientSocket != INVALID_SOCKET)
        {
            net->connected = true;
            u_long mode = 1;
            ioctlsocket(net->clientSocket, FIONBIO, &mode);
            printf("Maya Connected!\n");
        }
    }

    if (net->connected)
    {
        live_link_packet packet;
        int bytes = recv(net->clientSocket, (char*)&packet, sizeof(live_link_packet), 0);

        if (bytes == sizeof(live_link_packet))
        {
            printf("Received update for Bone %d\n", packet.boneIndex);

            if(packet.boneIndex < model->nodeCount)
            {
                gltf_node *node = &model->nodes[packet.boneIndex];

                glm::mat4 rawMayaMatrix = glm::make_mat4(packet.matrix);
                glm::mat4 correction = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1, 0, 0));
                glm::mat4 faceForward = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0, 1, 0));

                glm::mat4 liveMatrix = faceForward * correction * rawMayaMatrix;

                node->targetTranslation = glm::vec3(liveMatrix[3]) * 0.01f;

                node->targetRotation = glm::quat_cast(liveMatrix);
                printf("matrix: %s\n", glm::to_string(liveMatrix).c_str());

                // CRITICAL: We need to trigger a hierarchy update after this!
                // UpdateModelMatrices(model, identity);
            }
        } else if (bytes == 0 || (bytes == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)) {
            closesocket(net->clientSocket);
            net->connected = false;
            printf("Maya Disconnected.\n");
        }
    }
}

void SmoothLiveLink(gltf_model *model, float dt) {
    // Higher = Snappier
    // Lower = Softer
    float lerpSpeed = 0.5f;
    float t = 1.0f - exp(-lerpSpeed * dt);

    for(u32 i = 0; i < model->nodeCount; ++i) {
        gltf_node *node = &model->nodes[i];

        node->translation = glm::mix(node->translation, node->targetTranslation, t);
        node->rotation = glm::slerp(node->rotation, node->targetRotation, t);
    }
}
