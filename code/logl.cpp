#include "logl_geometry_data.h"
#include "logl_shader.cpp"
#include "logl_camera.cpp"
#include "logl_opengl.cpp"
#include "logl_gltf.cpp"
#include "logl_livelink.cpp"

u32 lightShader = 0;
u32 sourceLightShader = 0;
u32 terrainShader = 0;
u32 animShader = 0;
u32 lightSourceCube;
gltf_model backpack = {};
gltf_model vampire = {};
Texture terrainTexture = {};
u32 terrain;
VABO *vabo = {};
network_state maya_livelink;

void game_update_and_render(my_window *window, game_memory *memory, game_input *input, gui_vars *gui)
{
    Assert((&input->controllers[0].terminator - &input->controllers[0].buttons[0]) ==
           (ArrayCount(input->controllers[0].buttons)));
    Assert(sizeof(Game_State) <= memory->permanent_storage_size);

    Game_State *game_state = (Game_State *)memory->permanent_storage;
    memory_arena *arena = &game_state->world_arena;
    if(!memory->is_initialized)
    {
        initialize_arena(&game_state->world_arena, memory->permanent_storage_size - sizeof(Game_State),
                        (u8 *)memory->permanent_storage + sizeof(Game_State));


        vabo = &game_state->opengl_buffer;
        vabo->VAO = PushArray(arena, 4, u32);
        vabo->count = PushArray(arena, 4, u32);
        vabo->VBO = PushArray(arena, 4, u32);
        vabo->EBO = PushArray(arena, 4, u32);

        LoadModel(&backpack, arena, "../data/backpack/backpack.glb");
        LoadModel(&vampire, arena, "../data/vampire/vampire.glb");

        char *vertShaderPath = "../shaders/light.vs";
        char *fragShaderPath = "../shaders/light.fs";
        lightShader = Shader(window->sdl_window, vertShaderPath, fragShaderPath);

        vertShaderPath = "../shaders/lightSource.vs";
        fragShaderPath = "../shaders/lightSource.fs";
        sourceLightShader = Shader(window->sdl_window, vertShaderPath, fragShaderPath);

        vertShaderPath = "../shaders/terrain.vs";
        fragShaderPath = "../shaders/terrain.fs";
        char *tcsShaderPath = "../shaders/terrain.tcs";
        char *tesShaderPath = "../shaders/terrain.tes";
        terrainShader = Shader(window->sdl_window, vertShaderPath, fragShaderPath, tcsShaderPath, tesShaderPath);

        vertShaderPath = "../shaders/anim_model.vs";
        fragShaderPath = "../shaders/light.fs";
        animShader = Shader(window->sdl_window, vertShaderPath, fragShaderPath);

        u32 nSquareVerts = ArrayCount(cubeVertices);
        u32 cube = GetVAOwithoutEBO(vabo, cubeVertices, nSquareVerts);
        lightSourceCube = GetVAOwithoutEBO(vabo, cubeVertices, nSquareVerts);

        //u32 terrain = GenerateHeightMapCPU(&vabo, &arena, "../data/iceland_WINDOW_HEIGHTmap.png");
        terrain = GenerateHeightMap(vabo, arena, "../data/iceland_heightmap.png", &terrainTexture, GL_MIRRORED_REPEAT);

        glEnable(GL_DEPTH_TEST);

        my_camera *debug_camera = &game_state->debug_camera;
        debug_camera->projection = glm::perspective(glm::radians(45.0f), f32(WINDOW_WIDTH / WINDOW_HEIGHT), 0.1f, 100.0f);

        glm::vec3 camPos   = glm::vec3(0.0f, 0.0f,  3.0f);
        glm::vec3 camUp    = glm::vec3(0.0f, 1.0f,  0.0f);
        *debug_camera = InitCamera(camPos, camUp);

        maya_livelink = InitNetwork(1234);

        game_state->state = PLAY;

        memory->is_initialized = true;
    }

    glClearColor(gui->clear_color.x * gui->clear_color.w, gui->clear_color.y * gui->clear_color.w, gui->clear_color.z * gui->clear_color.w, gui->clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    f32 timeValue = GetSecondsElapsed(input->uptime, input->frequency);
    f32 ab = GetAlphaBlend(1.0f);

    game_controller_input *newKeyboard = GetController(input, 0);
    my_camera *debug_camera = &game_state->debug_camera;

    if(newKeyboard->debug.endedDown && game_state->state != DEBUG)
    {
        game_state->state = DEBUG;
    }
    else if(newKeyboard->debug.endedDown && game_state->state != PLAY)
    {
        game_state->state = PLAY;
    }

    switch(game_state->state)
    {
        case DEBUG:
        {
            SDL_SetWindowRelativeMouseMode(window->sdl_window, 0);
            //SetCameraDebugDirection(&myWindow, new_input, &debug_camera, &lastMouseX, &lastMouseY);
        } break;

        case PLAY:
        {
            SDL_SetWindowRelativeMouseMode(window->sdl_window, 1);
            // TODO(marc): behaves weird around corners/diagonal movements.
            SetCameraDirection(window, input, debug_camera, &input->last_mouse_x, &input->last_mouse_y);
        } break;
    }

    ProcessCameraInputs(debug_camera, newKeyboard, input->dt);
    debug_camera->projection = glm::perspective(glm::radians(debug_camera->fov), f32(WINDOW_WIDTH / WINDOW_HEIGHT), 0.1f, 1000.0f);
    glm::mat4 view = MYDEBUGGetViewMatrix(debug_camera);

    glm::vec3 activeLightPositions[4];

    // point lights source cube
    UseShader(sourceLightShader);
    SetUniform(sourceLightShader, "view", view);
    SetUniform(sourceLightShader, "projection", debug_camera->projection);
    SetUniform(sourceLightShader, "color", glm::vec3(1.0));

    glm::mat4 model = glm::mat4(1.0f);
    glBindVertexArray(vabo->VAO[lightSourceCube]);
    for(u32 x = 0; x < ArrayCount(pointLightPositions); ++x)
    {
        f32 radius = 1.5f;
        f32 speed = 2.0f;
        activeLightPositions[x] = pointLightPositions[x] +
                                  glm::vec3(sin(timeValue * speed + x) * radius,
                                  0.0f,
                                  cos(timeValue * speed + x) * radius);
        model = glm::mat4(1.0f);
        model = glm::translate(model, activeLightPositions[x]);
        model = glm::scale(model, glm::vec3(0.2f));

        SetUniform(sourceLightShader, "model", model);

        glBindVertexArray(vabo->VAO[lightSourceCube]);
        glDrawArrays(GL_TRIANGLES, 0, vabo->count[lightSourceCube]);
    }

    // backpack
    SetCameraUniforms(lightShader, debug_camera, view);
    UpdateShaderLights(lightShader, debug_camera, timeValue, activeLightPositions, true);

    glm::mat4 backpackRoot = glm::mat4(1.0f);
    backpackRoot = glm::translate(glm::mat4(1.0f), glm::vec3(4, 0, 0));

    f32 angle = glm::radians(-90.0f);
    backpackRoot = glm::rotate(backpackRoot, angle, glm::vec3(0, 1, 0));

    UpdateModelMatrices(&backpack, backpackRoot);
    DrawModel(&backpack, lightShader);

    // vampire
    SetCameraUniforms(animShader, debug_camera, view);
    UpdateShaderLights(animShader, debug_camera, timeValue, activeLightPositions, true);

    glm::mat4 vampireRoot = glm::mat4(1.0f);
    vampireRoot = glm::translate(glm::mat4(1.0f), glm::vec3(0, -1.5, 0));
    vampireRoot = glm::rotate(vampireRoot, angle, glm::vec3(1, 0, 0));
    // vampireRoot = glm::rotate(vampireRoot, angle, glm::vec3(0, 1, 0));
    vampireRoot = glm::scale(vampireRoot, glm::vec3(100.02f));

    // f32 time = (f32)SDL_GetTicks() / 1000.0f;
    // vampire.nodes[66].rotation = glm::angleAxis(time, glm::vec3(0, 1, 0));

    SetUniform(animShader, "finalBonesMatrices", vampire.finalBonesMatrices, 100);

    UpdateAnimation(&vampire, 0, input->dt);

    UpdateLiveLink(&maya_livelink, &vampire);
    SmoothLiveLink(&vampire, input->dt);

    UpdateModelMatrices(&vampire);
    ComputeSkinning(&vampire);
    DrawModel(&vampire, animShader, vampireRoot);

    // terrain
    SetCameraUniforms(terrainShader, debug_camera, view);
    UpdateShaderLights(terrainShader, debug_camera, timeValue, activeLightPositions, false);

    SetUniform(terrainShader, "heightMap", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, terrainTexture.index);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0, -10, 0));
    model = glm::scale(model, glm::vec3(0.2));

    SetUniform(terrainShader, "model", model);

    SetUniform(terrainShader, "scale", gui->scale);
    SetUniform(terrainShader, "shift", gui->shift);

    SetUniform(terrainShader, "shaderR", gui->shader_color.x*gui->shader_color.w);
    SetUniform(terrainShader, "shaderG", gui->shader_color.y*gui->shader_color.w);
    SetUniform(terrainShader, "shaderB", gui->shader_color.z*gui->shader_color.w);

    //Log("%f %f\n", terrainTexture.width, terrainTexture.height);

    glm::vec2 uTexelSize = glm::vec2(terrainTexture.width, terrainTexture.height);
    SetUniform(terrainShader, "uTexelSize", uTexelSize);

    glBindVertexArray(vabo->VAO[terrain]);
    glDrawArrays(GL_PATCHES, 0, 20*vabo->rez*vabo->rez);
#if 0
    for(u32 strip = 0; strip < vabo.NUM_STRIPS; ++strip)
    {
        glDrawElements(GL_TRIANGLE_STRIP,
                       vabo.NUM_VERTS_PER_STRIP,
                       GL_UNSIGNED_INT,
                       (void*)(sizeof(u32) *
                        vabo.NUM_VERTS_PER_STRIP *
                        strip));
    }
#endif
}
