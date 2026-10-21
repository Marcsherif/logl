#if !defined(LOGL_CAMERA_H)

enum Camera_Mode
{
    camera_debug = 0,
    camera_fps = 1,
};

struct Camera
{
    glm::vec3 position;
    glm::vec3 vel;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;

    f32 yaw;
    f32 pitch;

    f32 speed;
    f32 sensitivity;
    f32 fov;
    f32 zoom;

    glm::mat4 projection;

    Camera_Mode options;
};

#define LOGL_CAMERA_H
#endif
