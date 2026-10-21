// Resolve runtime libraries and assets relative to the executable.
internal void
load_runtime()
{
    wchar_t exe_path[MAX_PATH];
    DWORD length = GetModuleFileNameW(nullptr, exe_path, array_count(exe_path));
    if (!length || length >= array_count(exe_path))
        throw_error("Cannot resolve executable path.");

    wchar_t *separator = wcsrchr(exe_path, L'\\');
    if (!separator)
        throw_error("Cannot resolve executable directory.");

    *separator = L'\0';
    if (!SetCurrentDirectoryW(exe_path))
        throw_error("Cannot select executable directory.");

    const wchar_t *libraries[] = {L"SDL3.dll", L"opengl32.dll"};
    for (const wchar_t *library : libraries)
    {
        wchar_t path[MAX_PATH];
        if (swprintf_s(path, L"%s\\..\\bin\\%s", exe_path, library) < 0)
            throw_error("Runtime library path is too long.");

        if (!LoadLibraryExW(path, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH))
            throw_error("Cannot load runtime DLLs from bin.");
    }
}
