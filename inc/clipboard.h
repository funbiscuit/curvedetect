
#ifndef CURVEDETECT_CLIPBOARD_H
#define CURVEDETECT_CLIPBOARD_H

#include <string>

class Clipboard
{
private:
    Clipboard()=default;
    Clipboard(const Clipboard&);
    Clipboard& operator=(Clipboard&);
    ~Clipboard();

public:

    static Clipboard& get();

    void set_text(std::string text);

private:

    void init_platform();
    void cleanup_platform();

};

#endif //CURVEDETECT_CLIPBOARD_H
