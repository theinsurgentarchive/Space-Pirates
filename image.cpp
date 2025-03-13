#include <fstream>
#include <sstream>
#include "image.h"
#include "jlo.h"
Image::~Image()=default;
Image::Image(const std::string& file_name) {
    if (file_name.empty())
        return;
    int adjusted_len = file_name.length() - 4;
    std::string ppm_name = file_name.substr(adjusted_len) == ".ppm" ? 
        file_name :
        file_name.substr(0,adjusted_len) + ".ppm";
    if (ppm_name != file_name)
        system(("convert " + file_name + " " + ppm_name).c_str());
    std::ifstream file {ppm_name};
    if (!file) {
        DERRORF("failed to open file: %s\n",file_name.c_str());
        exit(1);
    }
    std::string line;
    std::getline(file,line);
    while (std::getline(file,line) && (line[0] == '#' || line.length() < 2));
    std::istringstream(line) >> width >> height;
    std::getline(file,line); //skips max color
    uint64_t size {static_cast<uint64_t>(width * height * 3)};
    data = std::make_unique<unsigned char[]>(size);
    file.read(reinterpret_cast<char*>(data.get()),size);
    file.close();
    if (ppm_name != file_name)
        std::remove(ppm_name.c_str());
}