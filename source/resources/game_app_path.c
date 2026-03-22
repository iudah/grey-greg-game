#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

#ifdef __linux__
#include <libgen.h>
#include <limits.h>
#include <unistd.h>
#endif

#include <string.h>
#include <zot.h>

char *_game_app_path;

#ifdef WIN32
char *dirname(char *path) {
  char *dir = zstrdup(path);
  char *last_bslash = strrchr(dir, '\\');
  char *last_fslash = strrchr(dir, '/');
  if (last_bslash < last_fslash) {
    last_bslash = last_fslash;
  }
  *last_bslash = '\0';
  path = zstrdup(dir);
  zfree(dir);
  return path;
}
#endif

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
  char *tmp = dirname(_game_app_path);
  if (!tmp) tmp = ".";
  tmp = zstrdup(tmp);
  zfree(_game_app_path);
  _game_app_path = tmp;
  return _game_app_path;
}
