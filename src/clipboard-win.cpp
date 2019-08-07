

#ifdef _WIN32

#include "clipboard.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <windows.h>

void Clipboard::init_platform()
{
    //nothing to do
}

void Clipboard::cleanup_platform()
{
    //nothing to do
}


void Clipboard::set_text(std::string text)
{
    //on windows this works okay
    glfwSetClipboardString(nullptr, text.c_str());
}

bool Clipboard::get_image(ImageData& imageData)
{
    bool loaded = false;

    if(OpenClipboard(nullptr))
    {
        HANDLE hDib = GetClipboardData(CF_DIB);

        if(hDib) {
            auto *bmpinfo = (BITMAPINFO *) GlobalLock(hDib);
            if (bmpinfo) {
                HDC hdc = GetDC(nullptr);
                int offset = (bmpinfo->bmiHeader.biBitCount > 8) ?
                             0 : sizeof(RGBQUAD) * (1 << bmpinfo->bmiHeader.biBitCount);
                const char *bits = (const char *) (bmpinfo) + bmpinfo->bmiHeader.biSize + offset;
                HBITMAP hbitmap = CreateDIBitmap(hdc, &bmpinfo->bmiHeader, CBM_INIT, bits, bmpinfo, DIB_RGB_COLORS);

                //convert to 32 bits format (if it's not already 32bit)
                BITMAP bm;
                GetObject(hbitmap, sizeof(bm), &bm);
                int w = bm.bmWidth;
                int h = bm.bmHeight;
                char *bits32 = new char[w * h * 4];

                BITMAPINFOHEADER bmpInfoHeader = {sizeof(BITMAPINFOHEADER), w, h, 1, 32};
                GetDIBits(hdc, hbitmap, 0, h, bits32, (BITMAPINFO *) &bmpInfoHeader, DIB_RGB_COLORS);
                ReleaseDC(nullptr, hdc);

                //convert BGRA (upside down) to normal RGB
                imageData.pixels = new uint8_t[w * h * 3];
                for (int row = 0; row < h; ++row) {
                    for (int col = 0; col < w; ++col) {
                        imageData.pixels[(row * w + col) * 3] = (uint8_t) bits32[((h - row - 1) * w + col) * 4 + 2];
                        imageData.pixels[(row * w + col) * 3 + 1] = (uint8_t) bits32[((h - row - 1) * w + col) * 4 + 1];
                        imageData.pixels[(row * w + col) * 3 + 2] = (uint8_t) bits32[((h - row - 1) * w + col) * 4];
                    }
                }
                imageData.width = w;
                imageData.height = h;
                loaded=true;

                //cleanup
                delete[]bits32;
                GlobalUnlock(hDib);
            }
        }
        CloseClipboard();
    }

    return loaded;
}

#endif
