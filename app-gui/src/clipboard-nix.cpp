
#ifdef __linux__

#include "clipboard.h"

#include <iostream>
#include <X11/Xlib.h>
#include <thread>
#include <mutex>
#include <X11/Xatom.h>
#include <climits>
#include <stb_image.h>




std::mutex _clipboard_x11_mutex;
std::string _clipboard_x11_current_text;
bool _clipboard_x11_stop_thread=false;
bool _clipboard_x11_thread_started=false;



bool _clipboard_x11_get_image(Display *display, Window window, const char *fmtname, ImageData& imageData)
{
    uint8_t *result;
    unsigned long ressize, restail;
    int resbits;
    Atom bufid = XInternAtom(display, "CLIPBOARD", False),
            fmtid = XInternAtom(display, fmtname, False),
            propid = XInternAtom(display, "XSEL_DATA", False),
            incrid = XInternAtom(display, "INCR", False);
    XEvent event;

    XSelectInput (display, window, PropertyChangeMask);
    XConvertSelection(display, bufid, fmtid, propid, window, CurrentTime);
    do {
        XNextEvent(display, &event);
    } while (event.type != SelectionNotify || event.xselection.selection != bufid);

    if (event.xselection.property)
    {
        XGetWindowProperty(display, window, propid, 0, LONG_MAX/4, True, AnyPropertyType,
                           &fmtid, &resbits, &ressize, &restail, &result);
        if (fmtid != incrid)
        {
            imageData.pixels = stbi_load_from_memory(result,
                                                     (int)ressize,
                                                     &imageData.width, &imageData.height,
                                                     nullptr,
                                                     3);//paste image in rgb format
            XFree(result);
        }
        else
        {
            //get all data piece by piece
            uint8_t* data=nullptr;
            int totalSize=0;
            do {
                do {
                    XNextEvent(display, &event);
                } while (event.type != PropertyNotify || event.xproperty.atom != propid || event.xproperty.state != PropertyNewValue);

                XGetWindowProperty(display, window, propid, 0, LONG_MAX/4, True, AnyPropertyType,
                                   &fmtid, &resbits, &ressize, &restail, &result);
                //not very good way to append data, but it works
                auto* newData=new uint8_t[totalSize+ressize];
                memcpy(newData, data, totalSize);
                memcpy(newData+totalSize, result, ressize);
                if(data)
                    delete[](data);
                data=newData;
                totalSize+=ressize;

                XFree(result);
            } while (ressize > 0);


            imageData.pixels = stbi_load_from_memory(data,
                                                     totalSize,
                                                     &imageData.width, &imageData.height,
                                                     nullptr,
                                                     3);//paste image in rgb format

            delete[](data);
        }

        return true;
    }
    else // request failed, e.g. owner can't convert to the target format
        return false;
}

void _clipboard_x11_start_copy_loop()
{
    Display* display = XOpenDisplay(nullptr);
    int N = DefaultScreen(display);
    Window window = XCreateSimpleWindow(display, RootWindow(display, N), 0, 0, 1, 1, 0,
                                        BlackPixel(display, N), WhitePixel(display, N));
    Atom selection = XInternAtom(display, "CLIPBOARD", 0);

    Atom targets_atom, text_atom, UTF8, XA_TEXT_PLAIN;

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
                _clipboard_x11_mutex.unlock();
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

    XDestroyWindow(display, window);
    XCloseDisplay(display);
}

void Clipboard::set_text(std::string text)
{
    // simple call to glfwSetClipboardString(window, text) doesn't work on linux (manjaro, kde)
    // after calling glfwSetClipboardString we can't save points to file,
    // save dialog gets very laggy and crashes with any input
    // so copy manually via xlib

    //start copy thread, if it's not running
    if(!_clipboard_x11_thread_started)
    {
        _clipboard_x11_thread_started= true;
        std::thread t(&_clipboard_x11_start_copy_loop);
        t.detach();
    }

    _clipboard_x11_mutex.lock();
    _clipboard_x11_current_text = text;
    _clipboard_x11_mutex.unlock();
}

bool Clipboard::get_image(ImageData& imageData)
{
    Display *display = XOpenDisplay(nullptr);
    unsigned long color = BlackPixel(display, DefaultScreen(display));
    Window window = XCreateSimpleWindow(display, DefaultRootWindow(display), 0,0, 1,1, 0, color, color);

    std::vector<const char*> formats{"image/png","image/jpeg","image/jpg","image/bmp","image/tiff" };

    bool loaded=false;

    for (auto& format : formats)
    {
        loaded = _clipboard_x11_get_image(display, window, format, imageData);
        if(loaded)
            break;
    }

    XDestroyWindow(display, window);
    XCloseDisplay(display);

    return loaded;
}

void Clipboard::init_platform()
{

}

void Clipboard::cleanup_platform()
{

}



#endif