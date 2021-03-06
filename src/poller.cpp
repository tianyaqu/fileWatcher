#include "poller.h"
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <iostream>
#include <cstring>

Poller::Poller(struct pollfd*fd,int size,int timeout=5000):fdSet_(fd,fd+size),timeout_(timeout)
{
}

Poller::~Poller()
{

}

int Poller::poll(std::queue<std::shared_ptr<WEvent>>& events)
{
    int result = ::poll((struct pollfd*)&fdSet_[0],fdSet_.size(),timeout_);
    if(result > 0 )
    {
        for(PollFdList::const_iterator it = fdSet_.begin();it < fdSet_.end();it++)
        {
            if(it->revents > 0)
            {
                // read events;
                onRead(*it,events);
            }
        }
    }
    return result;
}

void Poller::onRead(struct pollfd fd,std::queue<std::shared_ptr<WEvent>>& events)
{
    int bufsize;
    if (::ioctl(fd.fd, FIONREAD, &bufsize) < 0)
    {
        return;
    }
    char buffer[bufsize];
    char * offset = NULL;
    struct inotify_event* event;
    int len;
    if ((len = ::read(fd.fd, buffer, bufsize)) > 0)
    {
        offset = buffer;
        event = (struct inotify_event *) buffer;
        while (((char *) event - buffer) < len)
        {
            int tmp_len = sizeof(struct inotify_event) + event->len;
            event = (struct inotify_event *) (offset + tmp_len);

            //struct inotify_event *tmp = new (struct inotify_event);
            //std::memcpy(tmp,event,sizeof(struct inotify_event));
            //std::shared_ptr<struct inotify_event> event_ptr (tmp);
            std::shared_ptr<struct WEvent> event_ptr (new WEvent(event->wd,event->mask));
            events.push(event_ptr);
            offset += tmp_len;
            //std::cout<<maskToString(event->mask);
        }
    }
}

std::string Poller::maskToString(uint32_t mask) {
   std::string maskString = "";

 if(IN_ACCESS & mask)
   maskString.append("IN_ACCESS ");
 if(IN_ATTRIB & mask)
   maskString.append("IN_ATTRIB ");
 if(IN_CLOSE_WRITE & mask)
   maskString.append("IN_CLOSE_WRITE ");
 if(IN_CLOSE_NOWRITE & mask)
   maskString.append("IN_CLOSE_NOWRITE ");
 if(IN_CREATE & mask)
   maskString.append("IN_CREATE ");
 if(IN_DELETE & mask)
   maskString.append("IN_DELETE ");
 if(IN_DELETE_SELF & mask)
   maskString.append("IN_DELETE_SELF ");
 if(IN_MODIFY & mask)
   maskString.append("IN_MODIFY ");
 if(IN_MOVE_SELF & mask)
   maskString.append("IN_MOVE_SELF ");
 if(IN_MOVED_FROM & mask)
   maskString.append("IN_MOVED_FROM ");
 if(IN_MOVED_TO & mask)
   maskString.append("IN_MOVED_TO ");
 if(IN_OPEN & mask)
   maskString.append("IN_OPEN ");
 if(IN_ISDIR & mask)
   maskString.append("IN_ISDIR ");
 if(IN_UNMOUNT & mask)
   maskString.append("IN_UNMOUNT ");
 if(IN_Q_OVERFLOW & mask)
   maskString.append("IN_Q_OVERFLOW ");
 if(IN_CLOSE & mask)
   maskString.append("IN_CLOSE ");
 if(IN_IGNORED & mask)
   maskString.append("IN_IGNORED ");
 if(IN_ONESHOT & mask)
   maskString.append("IN_ONESHOT ");

 return maskString+"\n";
}
