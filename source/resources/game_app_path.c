#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

#ifdef __linux__
#include <limits.h>
#include <unistd.h>
#endif

char *_game_app_path;

char *game_app_path() {
  if (_game_app_path) {
    return _game_app_path;
  }
  size_t count = 1024;
  _game_app_path = zmalloc(sizeof(*_game_app_path) * count);
#ifdef WIN32
  GetModuleFileName(NULL, _game_app_path, count);
  count = strlen(_game_app_path);
#elif defined(__linux__)
  count = readlink("/proc/self/exe", _game_app_path, count);
  if (count == -1) {
    return NULL;
  }
#elif defined(__APPLE__)
  if (_NSGetExecutablePath(_game_app_path, &count) != 0) {
    return NULL;
  }
  count = strlen(_game_app_path);
#endif
  _game_app_path[count] = '\0';
  _game_app_path = realloc(_game_app_path, sizeof(*_game_app_path) * (count + 1));
  return _game_app_path;
}
