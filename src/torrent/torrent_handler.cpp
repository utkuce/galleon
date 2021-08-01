#include "torrent_handler.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <string>
#include <sstream>

#include <libtorrent/session.hpp>
#include <libtorrent/session_params.hpp>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/read_resume_data.hpp>
#include <libtorrent/magnet_uri.hpp>

std::string torrent::name;
float torrent::progress = 0.0f;
std::string torrent::speed = "0 mb/s";
std::string torrent::time = "-";
int torrent::peers = 0;

std::string torrent::save_path;

using clk = std::chrono::steady_clock;

// return the name of a torrent status enum
char const *state(lt::torrent_status::state_t s)
{
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#endif
    switch (s)
    {
    case lt::torrent_status::checking_files:
        return "checking";
    case lt::torrent_status::downloading_metadata:
        return "dl metadata";
    case lt::torrent_status::downloading:
        return "downloading";
    case lt::torrent_status::finished:
        return "finished";
    case lt::torrent_status::seeding:
        return "seeding";
    case lt::torrent_status::checking_resume_data:
        return "checking resume";
    default:
        return "<>";
    }
#ifdef __clang__
#pragma clang diagnostic pop
#endif
}

std::vector<char> load_file(char const *filename)
{
    std::ifstream ifs(filename, std::ios_base::binary);
    ifs.unsetf(std::ios_base::skipws);
    return {std::istream_iterator<char>(ifs), std::istream_iterator<char>()};
}

void torrent_thread(const char *magnet_link)
{
    try 
    {
        lt::settings_pack pack;
        pack.set_int(lt::settings_pack::alert_mask, lt::alert_category::error | lt::alert_category::storage | lt::alert_category::status);

        lt::session ses(pack);
        clk::time_point last_save_resume = clk::now();

        // load resume data from disk and pass it in as we add the magnet link
        std::ifstream ifs(".resume_file", std::ios_base::binary);
        ifs.unsetf(std::ios_base::skipws);
        std::vector<char> buf{std::istream_iterator<char>(ifs), std::istream_iterator<char>()};

        lt::add_torrent_params magnet = lt::parse_magnet_uri(magnet_link);
        if (buf.size())
        {
            lt::add_torrent_params atp = lt::read_resume_data(buf);
            if (atp.info_hashes == magnet.info_hashes)
                magnet = std::move(atp);
        }
        
        // set torrent save path
        std::stringstream save_path_ss;
        
        #ifdef unix
        save_path_ss << getenv("HOME") << "/Downloads/syncwatch";
        #elif defined(_WIN32)
        save_path_ss << getenv("HOMEDRIVE") << getenv("HOMEPATH") << "\\Downloads\\syncwatch";
        #endif

        magnet.save_path = save_path_ss.str(); 
        torrent::save_path = magnet.save_path;
        std::cout << "Saving files to: " << magnet.save_path << std::endl;

        // add torrent to session
        ses.async_add_torrent(std::move(magnet));

        // this is the handle we'll set once we get the notification of it being
        // added
        lt::torrent_handle h;

        for(;;)
        {
            bool events_done = torrent::events(ses, h);

            std::this_thread::sleep_for(std::chrono::milliseconds(200));

            // ask the session to post a state_update_alert, to update our
            // state output for the torrent
            ses.post_torrent_updates();

            // save resume data once every 30 seconds
            if (clk::now() - last_save_resume > std::chrono::seconds(30))
            {
                h.save_resume_data(lt::torrent_handle::save_info_dict);
                last_save_resume = clk::now();
            }

            if (events_done)
                return;
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        //TODO: show error box on UI
    }
}

void torrent::add_magnet_link(const char *magnet_link)
{
    std::thread t1(torrent_thread, magnet_link);
    t1.detach();
}