#include "logl_livelink.h"

#pragma pack(push, 1)
struct Live_Link_Packet
{
    char bone_name[64];
    float matrix[16];
};
#pragma pack(pop)

internal Live_Link_State
init_network(int port)
{
    Live_Link_State state = {};
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) return state;
    state.initialized = true;

    state.listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Set to non-blocking mode
    u_long mode = 1;
    ioctlsocket(state.listen_socket, FIONBIO, &mode);

    sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons((u_short)port);

    if(bind(state.listen_socket, (sockaddr*)&server_addr, sizeof(server_addr)))
    {
        assert_slow(!"[InitNetwork] Network error");
    };
    listen(state.listen_socket, SOMAXCONN);

    log_message("[Live Link] Listening on Port: %d\n", port);

    return state;
}

internal void
clear_live_link_targets(Gltf_Model *model)
{
    for (size_t i = 0; i < model->node_count; ++i)
        model->nodes[i].has_live_target = false;
}

internal void
shutdown_live_link(Live_Link_State *net)
{
    if(net->client_socket != INVALID_SOCKET) closesocket(net->client_socket);
    if(net->listen_socket != INVALID_SOCKET) closesocket(net->listen_socket);
    if(net->initialized) WSACleanup();
    *net = {};
}

internal bool
has_live_link_pose(Live_Link_State *net, Gltf_Model *model)
{
    if (!net->connected) return false;
    for (size_t i = 0; i < model->node_count; ++i)
        if (model->nodes[i].has_live_target) return true;
    return false;
}

internal void
update_live_link(Live_Link_State *net, Gltf_Model *model)
{
    if (!net->connected)
    {
        net->client_socket = accept(net->listen_socket, NULL, NULL);
        if (net->client_socket != INVALID_SOCKET)
        {
            net->connected = true;
            clear_live_link_targets(model);
            u_long mode = 1;
            ioctlsocket(net->client_socket, FIONBIO, &mode);
            log_message("[LiveLink] Maya Connected!\n");
        }
    }

    if (net->connected)
    {
        Live_Link_Packet packet;
        int bytes = recv(net->client_socket, (char*)&packet, sizeof(Live_Link_Packet), 0);

        if (bytes == sizeof(Live_Link_Packet))
        {
            packet.bone_name[sizeof(packet.bone_name) - 1] = '\0';
            auto it = model->bone_mapping.find(packet.bone_name);

            printf("Received update for Bone %s\n", packet.bone_name);

            // if(packet.boneIndex < model->node_count)
            if(it != model->bone_mapping.end())
            {
                Gltf_Node *node = &model->nodes[it->second];

                glm::mat4 raw_maya_matrix = glm::make_mat4(packet.matrix);

                glm::mat4 correction = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1, 0, 0));
                glm::mat4 face_forward = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0, 1, 0));

                glm::mat4 live_matrix = face_forward * correction * raw_maya_matrix;

                node->target_translation = glm::vec3(live_matrix[3]) * 0.01f;
                node->target_rotation = glm::quat_cast(live_matrix);
                node->has_live_target = true;

                log_message("Bone %s Target: %f, %f, %f\n",
                        packet.bone_name,
                        node->target_translation.x,
                        node->target_translation.y,
                        node->target_translation.z);

                log_message("[LiveLink] %s matrix: %s\n", packet.bone_name, glm::to_string(live_matrix).c_str());
            }
        } else if (bytes == 0 || (bytes == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)) {
            closesocket(net->client_socket);
            net->connected = false;
            clear_live_link_targets(model);
            log_message("[LiveLink] Maya Disconnected.\n");
        }
    }
}

void smooth_live_link(Live_Link_State *net, Gltf_Model *model, float dt) {
    if (!net->connected) return;
    // Higher = Snappier
    // Lower = Softer
    float lerp_speed = 0.5f;
    float t = 1.0f - exp(-lerp_speed * dt);

    for(u32 i = 0; i < model->node_count; ++i) {
        Gltf_Node *node = &model->nodes[i];
        if (!node->has_live_target) continue;

        node->translation = glm::mix(node->translation, node->target_translation, t);
        node->rotation = glm::slerp(node->rotation, node->target_rotation, t);
    }
}
