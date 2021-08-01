#include <string>

#include <libtorrent/session.hpp>

namespace torrent
{
    extern std::string name;
    extern float progress;
    extern std::string speed;
    extern std::string time;
    extern int peers;

    extern std::string save_path;

    void add_magnet_link(const char*);
    bool events(lt::session&, lt::torrent_handle&);
}