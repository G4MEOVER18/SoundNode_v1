#include "playlist.h"
#include <dirent.h>
#include <string.h>

static bool ends_with_mp3(const char* s) {
  size_t n = strlen(s);
  if (n < 4) return false;
  const char* e = s + (n - 4);
  return (strcasecmp(e, ".mp3") == 0);
}

std::vector<std::string> playlist_scan_mp3(const char* dir) {
  std::vector<std::string> out;
  DIR* d = opendir(dir);
  if (!d) return out;

  while (true) {
    struct dirent* ent = readdir(d);
    if (!ent) break;
    if (ent->d_type == DT_DIR) continue;
    if (!ends_with_mp3(ent->d_name)) continue;

    std::string p = std::string(dir) + "/" + ent->d_name;
    out.push_back(p);
  }

  closedir(d);
  return out;
}
