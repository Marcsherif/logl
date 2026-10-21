# LOGL | Real-Time Rendering & Animation

A C++ / OpenGL sandbox exploring the path from imported character data to animated pixels, with a Maya live link for interactive pose updates.

**C++ · OpenGL 4.5 · GLSL · SDL3 · glTF · Dear ImGui · Maya · C# · Python**

Started as a LearnOpenGL study, now extended with skeletal animation, tessellated terrain, and Maya integration.

## Lighting, materials, and camera flashlight

https://github.com/user-attachments/assets/21d35ed2-f758-431c-b07f-79786146c5e5

## Heightmap terrain and tessellation

https://github.com/user-attachments/assets/3efb1aa4-6f92-4315-9b88-cb0d955424e1

## Skeletal animation and Maya live link

https://github.com/user-attachments/assets/9140ea49-20b3-4606-8f1b-cfa04e0f4f40

## Technical highlights

| System | Implementation | Source |
| --- | --- | --- |
| Character animation | glTF interpolation, quaternion slerp, transform hierarchy, inverse bind matrices | [Animation](code/logl_animation.cpp) |
| GPU skinning | Linear blend skinning, four joint influences, 100-matrix palette | [Vertex shader](shaders/anim_model.vs) |
| Maya live link | Python joint callbacks, C# TCP sender, C++ receiver with pose smoothing | [Maya bridge](livelink/MayaLiveLinkWrapper.py), [receiver](code/logl_livelink.cpp) |
| Terrain | Heightmap displacement and distance-based tessellation | [Control shader](shaders/terrain.tcs), [evaluation shader](shaders/terrain.tes) |
| Lighting | Directional, point, and spot lights with texture maps | [Fragment shader](shaders/light.fs) |
| Runtime tools | Scene presets, animation scrubbing, skeleton inspection, joint weight visualization | [Workbench](code/logl_workbench.cpp) |

## Build and run

Requires Windows x64, MSVC, Windows SDK, and OpenGL 4.5.

Dependencies and demo assets are **not tracked in Git**. Supply these paths before building:

| Dependency | Local path |
| --- | --- |
| SDL3 | `ext/sdl3/include/SDL3/SDL.h`, `ext/sdl3/lib/SDL3.lib`, `bin/SDL3.dll` |
| OpenGL runtime | `bin/opengl32.dll` |
| GLAD (OpenGL 4.5) | `ext/glad/include/glad/glad.h`, `ext/glad/src/glad.c` |
| GLM | `ext/glm/include/glm/glm.hpp` |
| stb_image | `ext/stb_image/include/stb_image/stb_image.h` |
| cgltf | `ext/cgltf/include/cgltf/cgltf.h` |
| Dear ImGui | `ext/imgui/include/`, including implementation files and `backends/` |

Startup requires `data/vampire/vampire.glb`, its textures, and `data/iceland_heightmap.png`. Dependency versions are not pinned.

```bat
code\build.bat debug
code\run.bat
```

Use `release` instead of `debug` for optimization. Output: `build\LOGL.exe`; runtime DLLs load from `bin/`. Set `LOGL_MSVC_SETUP` to your x64 compiler setup script if automatic discovery fails.

| Input | Action |
| --- | --- |
| W / A / S / D, mouse | Move and look |
| Left Shift | Faster movement |
| C / V | Adjust field of view |
| F2 | Switch between workbench and free camera |
| Enter | Toggle fullscreen |
| Escape | Quit |

The workbench provides scene presets, animation playback, skeleton inspection, weight visualization, and rendering controls. Enable **Use live pose** for Maya input.

Limitations: first skin only, partial glTF interpolation support, experimental live-link transport, and fixed demo asset paths.

## Optional Maya bridge

Requires Maya 2026, .NET 8, and Maya Python with `clr` / pythonnet. Build with `dotnet build livelink/livelink.csproj --configuration Release`, then set `dll_path` in `livelink/MayaLiveLinkWrapper.py` to the resulting `LiveLink.dll`.

Launch LOGL, open the matching rig in Maya, and run the wrapper. It connects to `127.0.0.1:1234`. The wrapper removes existing Maya script jobs; use a dedicated demo session. Joint names and coordinate conversion remain asset-specific.

## Credits and licensing

Based on [LearnOpenGL](https://learnopengl.com/). See [LICENSE](LICENSE). Dependencies and demo assets retain their own licenses.
