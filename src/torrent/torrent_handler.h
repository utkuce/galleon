#include <string>

namespace torrent
{
    extern std::string name;
    extern float progress;
    extern std::string speed;
    extern std::string time;
    extern int peers;

    void add_magnet_link(const char*);
}