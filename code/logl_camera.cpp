global_variable const f32 camera_fov = 90;
global_variable const f32 camera_fov_max = 180;
global_variable const f32 camera_speed = 5.0f;
global_variable const f32 camera_sensitivity = 0.1f;
global_variable const f32 camera_yaw = 0.0f;
global_variable const f32 camera_pitch = 0.0f;

void
process_camera_inputs(Camera *cam, Game_Controller_Input *input, f32 delta_time)
{
    glm::vec3 move_dir(0.0f);
    cam->speed = camera_speed;

    if (input->speed_up.ended_down)     cam->speed = camera_speed*10;

    if(input->move_left.ended_down)     move_dir -= glm::normalize(glm::cross(cam->front, cam->up));
    if(input->move_right.ended_down)    move_dir += glm::normalize(glm::cross(cam->front, cam->up));
    if(input->move_forward.ended_down)  move_dir += cam->front;
    if(input->move_backward.ended_down) move_dir -= cam->front;

    if (glm::length(move_dir) > 0.0f)
    {
        move_dir = glm::normalize(move_dir);
        cam->vel = move_dir * cam->speed;
    }
    else
    {
        cam->vel *= 1.0f / (1.0f + delta_time * 6.0f);
    }

    cam->position += cam->vel * delta_time;

    //if (glm::length(move_dir) > 0.0f)
    //{
    //    move_dir = glm::normalize(move_dir);
    //}
    //cam->position += move_dir * camera_speed * delta_time;

    if(input->fov_in.ended_down)
    {
        cam->fov -= 2;
        if(cam->fov < 1.0f)
        cam->fov = 1.0f;
    }

    if(input->fov_out.ended_down)
    {
        cam->fov += 2;
        if(cam->fov > camera_fov_max)
            cam->fov = camera_fov_max;
    }

    if(input->zoom_in.ended_down)
    {
        cam->zoom -= 2;
    }

    if(input->zoom_out.ended_down)
    {
        cam->zoom += 2;
    }
}

void
set_camera_debug_direction(Window *window, Game_Input *input, Camera *cam, f32 *last_mouse_x, f32 *last_mouse_y)
{
    f32 width = (f32)window->width;
    f32 height = (f32)window->height;
    const f32 edge_threshold = 10.0f;

    if(input->mouse_x <= edge_threshold)
    {
        first_mouse = true;
        input->mouse_x = width-edge_threshold-1;
        SDL_WarpMouseInWindow(window->sdl_window, input->mouse_x, input->mouse_y);
    }
    if(input->mouse_x >= width - edge_threshold)
    {
        first_mouse = true;
        input->mouse_x = edge_threshold;
        SDL_WarpMouseInWindow(window->sdl_window, input->mouse_x, input->mouse_y);
    }

    if(input->mouse_y <= edge_threshold)
    {
        first_mouse = true;
        input->mouse_y = height-edge_threshold-1;
        SDL_WarpMouseInWindow(window->sdl_window, input->mouse_x, input->mouse_y);
    }
    if(input->mouse_y >= height-edge_threshold)
    {
        first_mouse = true;
        input->mouse_y = edge_threshold;
        SDL_WarpMouseInWindow(window->sdl_window, input->mouse_x, input->mouse_y);
    }

    if (first_mouse)
    {
        *last_mouse_x = input->mouse_x;
        *last_mouse_y = input->mouse_y;
        first_mouse = false;
    }

    f32 mouse_offset_x = input->mouse_x - *last_mouse_x;
    f32 mouse_offset_y = *last_mouse_y - input->mouse_y;
    *last_mouse_x = input->mouse_x;
    *last_mouse_y = input->mouse_y;
    mouse_offset_x *= cam->sensitivity;
    mouse_offset_y *= cam->sensitivity;

    cam->yaw   += mouse_offset_x;
    cam->pitch += mouse_offset_y;

    if(cam->pitch > 89.0f)
        cam->pitch =  89.0f;
    if(cam->pitch < -89.0f)
        cam->pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(cam->yaw)) * cos(glm::radians(cam->pitch));
    direction.y = sin(glm::radians(cam->pitch));
    direction.z = sin(glm::radians(cam->yaw)) * cos(glm::radians(cam->pitch));
    glm::vec3 cam_front = glm::normalize(direction);

    cam->position = direction*cam->zoom;
    cam->up.z = direction.z;

    cam->front = glm::vec3(-1, -1, -1);
}

void
set_camera_direction(Window *window, Game_Input *input, Camera *cam, f32 *last_mouse_x, f32 *last_mouse_y)
{
    f32 width = (f32)window->width;
    f32 height = (f32)window->height;
    const f32 edge_threshold = 10.0f;

    if(input->mouse_x <= edge_threshold)
    {
        first_mouse = true;
        input->mouse_x = width-edge_threshold-1;
        SDL_WarpMouseInWindow(window->sdl_window, input->mouse_x, input->mouse_y);
    }
    if(input->mouse_x >= width - edge_threshold)
    {
        first_mouse = true;
        input->mouse_x = edge_threshold;
        SDL_WarpMouseInWindow(window->sdl_window, input->mouse_x, input->mouse_y);
    }

    if(input->mouse_y <= edge_threshold)
    {
        first_mouse = true;
        input->mouse_y = height-edge_threshold-1;
        SDL_WarpMouseInWindow(window->sdl_window, input->mouse_x, input->mouse_y);
    }
    if(input->mouse_y >= height-edge_threshold)
    {
        first_mouse = true;
        input->mouse_y = edge_threshold;
        SDL_WarpMouseInWindow(window->sdl_window, input->mouse_x, input->mouse_y);
    }

    if (first_mouse)
    {
        *last_mouse_x = input->mouse_x;
        *last_mouse_y = input->mouse_y;
        first_mouse = false;
    }

    f32 mouse_offset_x = input->mouse_x - *last_mouse_x;
    f32 mouse_offset_y = *last_mouse_y - input->mouse_y;
    *last_mouse_x = input->mouse_x;
    *last_mouse_y = input->mouse_y;
    mouse_offset_x *= cam->sensitivity;
    mouse_offset_y *= cam->sensitivity;

    cam->yaw   += mouse_offset_x;
    cam->pitch += mouse_offset_y;

    if(cam->pitch > 89.0f)
        cam->pitch =  89.0f;
    if(cam->pitch < -89.0f)
        cam->pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(cam->yaw)) * cos(glm::radians(cam->pitch));
    direction.y = sin(glm::radians(cam->pitch));
    direction.z = sin(glm::radians(cam->yaw)) * cos(glm::radians(cam->pitch));
    glm::vec3 cam_front = glm::normalize(direction);

    cam->front = cam_front;
}

glm::mat4
get_view_matrix(Camera *camera)
{
    return glm::lookAt(camera->position, camera->position + camera->front, camera->up);
}

glm::mat4
get_debug_view_matrix(Camera *camera)
{
    glm::vec3 eye = camera->position;
    glm::vec3 center = camera->position + camera->front;
    glm::vec3 up = camera->up;

    if(camera->options == camera_fps)
    {
        center.y -= eye.y;
        eye.y = 0;
    }

    glm::mat4 view = glm::mat4(1.0f);

    glm::vec3 f = glm::normalize(center - eye);
    glm::vec3 r = glm::normalize(glm::cross(f, up));
    glm::vec3 u = glm::cross(r, f);

    view[0][0] = r.x;
    view[1][0] = r.y;
    view[2][0] = r.z;

    view[0][1] = u.x;
    view[1][1] = u.y;
    view[2][1] = u.z;

    view[0][2] = -f.x;
    view[1][2] = -f.y;
    view[2][2] = -f.z;

    glm::mat4 pos = glm::mat4(1.0f);
    pos[3][0] = -eye.x;
    pos[3][1] = -eye.y-camera->zoom;
    pos[3][2] = -eye.z;

    view = view * pos;

    return view;
}

Camera inline
init_camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           f32 yaw = camera_yaw, f32 pitch = camera_pitch, Camera_Mode options = camera_debug)
{
    Camera my_cam = {};

    my_cam.position = position;
    my_cam.vel = glm::vec3(0.0f, 0.0f, 0.0f);
    my_cam.up = up;

    my_cam.yaw = yaw;
    my_cam.pitch = pitch;
    my_cam.front = glm::normalize(glm::vec3(
        cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
        sin(glm::radians(pitch)),
        sin(glm::radians(yaw)) * cos(glm::radians(pitch))));
    my_cam.right = glm::normalize(glm::cross(my_cam.front, up));

    my_cam.speed = camera_speed;
    my_cam.sensitivity = camera_sensitivity;
    my_cam.fov = camera_fov;
    my_cam.zoom = 0;

    my_cam.options = options;

    return my_cam;
}
