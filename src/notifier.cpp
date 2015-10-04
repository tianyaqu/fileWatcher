#include "notifier.h"
#include <cassert>
#include <unistd.h>
#include <iostream>

Notifier::Notifier():quit_(true)
{
    inotifier_fd = inotify_init();
    assert(inotifier_fd != -1);

    struct pollfd pfd = { inotifier_fd, POLLIN, 0 };
    poller_ .reset(new Poller(&pfd,1,5000));
}

Notifier::~Notifier()
{
    ::close(inotifier_fd);
}

int Notifier::add_watch(const std::string& path,u_int32_t mask)
{
    if(file_to_wd.count(path)  > 0)
    {
        return -1;
    }

    int wd = inotify_add_watch(inotifier_fd,path.c_str(),mask);
    if(wd > 0)
    {
        file_to_wd[path] = wd;
        wd_to_file[wd] = path;
    }

    return wd;
}

int Notifier::rm_watch(int wd)
{
    int ret = inotify_rm_watch(inotifier_fd,wd);
    if(ret >=0 )
    {
        auto x = wd_to_file.find(wd);
        std::string path = "";
        if(x != wd_to_file.end())
        {
            path = x->second;
            wd_to_file.erase(x);
            file_to_wd.erase(path);
        }
    }

    return ret;
}

int Notifier::rm_watch(const std::string &path)
{
    auto x = file_to_wd.find(path);
    if(x != file_to_wd.end())
    {
        int ret = rm_watch(x->second);
        return ret;
    }
    else
    {
        return -1;
    }
}

void Notifier::loop()
{
    while(!quit_)
    {
        if(!eventQueue_.empty())
        {
            //process();
        }

        poller_->poll(eventQueue_);
    }
}

void Notifier::start()
{
    quit_ = false;
    loop();
}

void Notifier::stop()
{
    quit_ = true;
    //set pipe_fd = 'stop'
}
