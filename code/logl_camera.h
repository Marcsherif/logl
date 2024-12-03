#if !defined(LOGL_CAMERA_H)

enum camOptions
{
    camDEBUG = 0,
    camFPS = 1,
};

struct my_camera
{
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;

    f32 yaw;
    f32 pitch;

    f32 speed;
    f32 sensitivity;
    f32 fov;
    f32 zoom;

    camOptions options;
};

#define LOGL_CAMERA_H
#endif
