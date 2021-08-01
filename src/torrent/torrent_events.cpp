#include "torrent_handler.h"
#include "../video/video_player.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <filesystem>

#include <libtorrent/alert_types.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/torrent_status.hpp>
#include <libtorrent/write_resume_data.hpp>
#include <libtorrent/error_code.hpp>
#include <libtorrent/torrent_flags.hpp>

std::string video_path; //TODO: make vector of paths
void set_video_path(const lt::torrent_handle& h)
{
    auto files = h.torrent_file().get()->files();
    int64_t largest_size = 0;

    // TODO: add all video files in a queue
    // let user select which ones to add
    for (size_t i = 0; i < files.num_files(); i++)
    {
        int64_t size = files.file_size(i);
        if (size > largest_size) // TODO: check if file is video
        {
            video_path = files.file_path(i);

            std::stringstream stream;
            stream << std::filesystem::path(video_path).filename();
            torrent::name = stream.str();
            
            largest_size = size;
        }
    }
}

bool torrent::events(lt::session& ses, lt::torrent_handle& h)
{
    bool done = false;

    std::vector<lt::alert *> alerts;
    ses.pop_alerts(&alerts);

    for (lt::alert const *a : alerts)
    {
        if (auto at = lt::alert_cast<lt::add_torrent_alert>(a))
        {
            h = at->handle;
            h.set_flags(lt::torrent_flags::sequential_download);
        }

        if (auto at = lt::alert_cast<lt::metadata_received_alert>(a))
        {

        }

        // if we receive the finished alert or an error, we're done
        if (lt::alert_cast<lt::torrent_finished_alert>(a))
        {
            h.save_resume_data(lt::torrent_handle::save_info_dict);
            done = true;
        }
        if (lt::alert_cast<lt::torrent_error_alert>(a))
        {
            std::cout << a->message() << std::endl;
            done = true;
            h.save_resume_data(lt::torrent_handle::save_info_dict);
        }

        // when resume data is ready, save it
        if (auto rd = lt::alert_cast<lt::save_resume_data_alert>(a))
        {
            std::ofstream of(".resume_file", std::ios_base::binary);
            of.unsetf(std::ios_base::skipws);
            auto const b = write_resume_data_buf(rd->params);
            of.write(b.data(), int(b.size()));
            if (done)
                return true;
        }

        if (lt::alert_cast<lt::save_resume_data_failed_alert>(a))
        {
            if (done)
                return true;
        }

        if (auto st = lt::alert_cast<lt::state_update_alert>(a))
        {
            if (st->status.empty())
                continue;

            // we only have a single torrent, so we know which one
            // the status is for
            lt::torrent_status const &s = st->status[0];
            // std::cout << '\r' << state(s.state) << ' '
            //           << (s.download_payload_rate / 1000) << " kB/s "
            //           << (s.total_done / 1000) << " kB ("
            //           << (s.progress_ppm / 10000) << "%) downloaded ("
            //           << s.num_peers << " peers)\x1b[K";
            // std::cout.flush();
            
            torrent::progress = s.progress;
            torrent::peers = s.num_peers;

            double speed = s.download_payload_rate / 1000000.0;
            std::stringstream stream;
            stream << std::fixed << std::setprecision(2) << speed;
            torrent::speed = stream.str() + " mb/s";

            if (torrent::name.empty())
            {
                if (torrent::progress > .01f)
                {
                    set_video_path(h);

                    if (!video_path.empty() && !torrent::save_path.empty())
                    {   
                        std::stringstream source_path;
                        source_path << torrent::save_path << "/" << video_path;
                        std::cout << "Source path: " << source_path.str() << std::endl;
                        set_video_source(source_path.str().c_str());
                    }
                }
            }

        }
    }

    return false;
}