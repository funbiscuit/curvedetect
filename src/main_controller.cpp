

#include <iostream>
#include <string>

#include "main_controller.h"
#include "tinyfiledialogs.h"

void MainController::open_image()
{

    char const * lFilterPatterns[6] = { "*.jpg", "*.jpeg", "*.png", "*.bmp", "*.tiff", "*.gif" };

    char const * filename = tinyfd_openFileDialog(
            "Choose image",
            "",
            6,
            lFilterPatterns,
            nullptr,
            0);

    if (filename)
    {
        std::string path = filename;

        std::cout << "open: "<<filename<<"\n";

        image=std::make_shared<Image>(path);

        if(!image->is_loaded())
        {
            tinyfd_messageBox("Can't open image", "Opened file is not an image.\nTry again.", "ok", "error", 0);
            image= nullptr;
        }
    }
}