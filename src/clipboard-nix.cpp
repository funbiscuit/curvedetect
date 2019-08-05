
#ifdef __linux__

#include "clipboard.h"

#include <iostream>
#include <X11/Xlib.h>
#include <thread>
#include <mutex>


std::mutex _clipboard_x11_mutex;
std::string _clipboard_x11_current_text;
bool _clipboard_x11_stop_thread=false;
bool _clipboard_x11_thread_started=false;


void _clipboard_x11_start_copy_loop()
{
    Display* display = XOpenDisplay(nullptr);
    int N = DefaultScreen(display);
    Window window = XCreateSimpleWindow(display, RootWindow(display, N), 0, 0, 1, 1, 0,
                                      BlackPixel(display, N), WhitePixel(display, N));
    Atom selection = XInternAtom(display, "CLIPBOARD", 0);

    Atom targets_atom, text_atom, UTF8, XA_ATOM = 4, XA_STRING = 31, XA_TEXT_PLAIN;

    targets_atom = XInternAtom(display, "TARGETS", 0);
    text_atom = XInternAtom(display, "TEXT", 0);
    XA_TEXT_PLAIN = XInternAtom(display, "text/plain", 0);
    UTF8 = XInternAtom(display, "UTF8_STRING", 1);

    if (UTF8 == None)
        UTF8 = XA_STRING;

    XEvent event;
    int R;
    XSelectionEvent ev;
    XSelectionRequestEvent * xsr;
    unsigned char* text;
    int size;

    XSetSelectionOwner (display, selection, window, 0);

    if (XGetSelectionOwner (display, selection) != window)
        return;

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        XNextEvent (display, &event);
        switch (event.type)
        {
            case SelectionRequest:

                _clipboard_x11_mutex.lock();
                text = (unsigned char*)(_clipboard_x11_current_text.c_str());
                size = _clipboard_x11_current_text.length();
                _clipboard_x11_mutex.unlock();

                if (event.xselectionrequest.selection != selection)
                    break;

                xsr = &event.xselectionrequest;
                ev = {0};
                R = 0;

                ev.type = SelectionNotify, ev.display = xsr->display, ev.requestor = xsr->requestor,
                ev.selection = xsr->selection, ev.time = xsr->time, ev.target = xsr->target, ev.property = xsr->property;

                if (ev.target == targets_atom)
                {
                    R = XChangeProperty (ev.display, ev.requestor, ev.property,
                                         XA_ATOM, 32, PropModeReplace, (unsigned char*)&UTF8, 1);
                    
                    Atom possibleTargets[] = { UTF8, XA_STRING, text_atom, targets_atom, XA_TEXT_PLAIN };
    
                    XChangeProperty( ev.display, ev.requestor,
                                     ev.property,
                                     XA_ATOM,
                                     32,
                                     PropModeReplace,
                                     (unsigned char *) possibleTargets,
                                     sizeof(possibleTargets)/sizeof(possibleTargets[0])
                    );
                }
                else if (ev.target == XA_STRING || ev.target == text_atom || ev.target == XA_TEXT_PLAIN)
                    R = XChangeProperty(ev.display, ev.requestor, ev.property,
                            XA_STRING, 8, PropModeReplace, text, size);
                else if (ev.target == UTF8)
                    R = XChangeProperty(ev.display, ev.requestor, ev.property,
                            UTF8, 8, PropModeReplace, text, size);
                else
                    ev.property = None;

                if ((R & 2) == 0)
                    XSendEvent (display, ev.requestor, 0, 0, (XEvent *)&ev);
                break;
            case SelectionClear:
                _clipboard_x11_stop_thread=true;
                break;
        }
        //not used
        if(_clipboard_x11_stop_thread)
            break;
    }
    _clipboard_x11_stop_thread=false;
    _clipboard_x11_thread_started=false;
}

void Clipboard::set_text(std::string text)
{
    // simple call to glfwSetClipboardString(window, text) doesn't work on linux (manjaro, kde)
    // after calling glfwSetClipboardString we can't save points to file,
    // save dialog gets very laggy and crashes with any input
    // so copy manually via xlib
    
    //start thread again, if it ended
    init_platform();
    
    _clipboard_x11_mutex.lock();
    _clipboard_x11_current_text = text;
    _clipboard_x11_mutex.unlock();
}

void Clipboard::init_platform()
{
    if(!_clipboard_x11_thread_started)
    {
        _clipboard_x11_thread_started= true;
        std::thread t(&_clipboard_x11_start_copy_loop);
        t.detach();
    }
}

void Clipboard::cleanup_platform()
{

}



#endif