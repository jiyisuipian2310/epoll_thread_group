#ifndef __IEVENT_H__
#define __IEVENT_H__

class IEvent {
public:
    IEvent() {}
    virtual ~IEvent() {}
    virtual bool handle_input() = 0;
    virtual bool handle_output() = 0;
};

#endif
