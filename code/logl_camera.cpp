global_variable const f32 FOV = 90;
global_variable const f32 FOV_MAX = 180;
global_variable const f32 CAMSPEED = 5.0f;
global_variable const f32 SENSITIVITY = 0.1f;
global_variable const f32 YAW = 0.0f;
global_variable const f32 PITCH = 0.0f;

void
ProcessCameraInputs(my_camera *cam, game_controller_input *input, f32 deltaTime)
{
    // TODO(marc): implement diagonal speed
    cam->speed = CAMSPEED * deltaTime;
    if(input->moveLeft.endedDown)
    {
        cam->position -= glm::normalize(glm::cross(cam->front, cam->up)) * cam->speed;
//        Log("Position: %s\n", glm::to_string(cam->position).c_str());
    }

    if(input->moveRight.endedDown)
    {
        cam->position += glm::normalize(glm::cross(cam->front, cam->up)) * cam->speed;
    }

    if(input->moveForward.endedDown)
    {
        cam->position += cam->speed * cam->front;
//        Log("Position: %s\n", glm::to_string(cam->position).c_str());
    }

    if(input->moveBackward.endedDown)
    {
        cam->position -= cam->speed * cam->front;
    }

    if(input->fovIn.endedDown)
    {
        cam->fov -= 30;
        if(cam->fov < 1.0f)
        cam->fov = 1.0f;
    }

    if(input->fovOut.endedDown)
    {
        cam->fov += 30;
        if(cam->fov > FOV_MAX)
            cam->fov = FOV_MAX;
    }

    if(input->zoomIn.endedDown)
    {
        cam->zoom -= 30;
    }

    if(input->zoomOut.endedDown)
    {
        cam->zoom += 30;
    }
}

void
GetCameraDirection(my_window *window, game_input *input, my_camera *cam, f32 *lastMouseX, f32 *lastMouseY)
{
    f32 width = (f32)window->width;
    f32 height = (f32)window->height;
    const f32 edgeThreshold = 10.0f;

    if(input->mouseX <= edgeThreshold)
    {
        firstMouse = true;
        input->mouseX = width-edgeThreshold-1;
        SDL_WarpMouseInWindow(window->window, input->mouseX, input->mouseY);
    }
    if(input->mouseX >= width - edgeThreshold)
    {
        firstMouse = true;
        input->mouseX = edgeThreshold;
        SDL_WarpMouseInWindow(window->window, input->mouseX, input->mouseY);
    }

    if(input->mouseY <= edgeThreshold)
    {
        firstMouse = true;
        input->mouseY = height-edgeThreshold-1;
        SDL_WarpMouseInWindow(window->window, input->mouseX, input->mouseY);
    }
    if(input->mouseY >= height-edgeThreshold)
    {
        firstMouse = true;
        input->mouseY = edgeThreshold;
        SDL_WarpMouseInWindow(window->window, input->mouseX, input->mouseY);
    }

    if (firstMouse)
    {
        *lastMouseX = input->mouseX;
        *lastMouseY = input->mouseY;
        firstMouse = false;
    }

    f32 mouseOffsetX = input->mouseX - *lastMouseX;
    f32 mouseOffsetY = *lastMouseY - input->mouseY;
    *lastMouseX = input->mouseX;
    *lastMouseY = input->mouseY;
    mouseOffsetX *= cam->sensitivity;
    mouseOffsetY *= cam->sensitivity;

    cam->yaw   += mouseOffsetX;
    cam->pitch += mouseOffsetY;

    if(cam->pitch > 89.0f)
        cam->pitch =  89.0f;
    if(cam->pitch < -89.0f)
        cam->pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(cam->yaw)) * cos(glm::radians(cam->pitch));
    direction.y = sin(glm::radians(cam->pitch));
    direction.z = sin(glm::radians(cam->yaw)) * cos(glm::radians(cam->pitch));
    glm::vec3 camFront = glm::normalize(direction);

    cam->front = camFront;
}

glm::mat4
GetViewMatrix(my_camera *camera)
{
    return glm::lookAt(camera->position, camera->position + camera->front, camera->up);
}

glm::mat4
MYDEBUGGetViewMatrix(my_camera *camera)
{
    glm::vec3 eye = camera->position;
    glm::vec3 center = camera->position + camera->front;
    glm::vec3 up = camera->up;

    if(camera->options == camFPS)
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

my_camera inline
InitCamera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           f32 yaw = YAW, f32 pitch = PITCH, camOptions options = camDEBUG)
{
    my_camera myCam;

    myCam.position = position;
    myCam.up = up;

    myCam.yaw = yaw;
    myCam.pitch = pitch;

    myCam.speed = CAMSPEED;
    myCam.sensitivity = SENSITIVITY;
    myCam.fov = FOV;
    myCam.zoom = 0;

    myCam.options = options;

    return myCam;
}
