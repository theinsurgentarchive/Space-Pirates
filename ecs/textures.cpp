#include "textures.h"
#include "dirent.h"
#include <iostream>
//Start - TextureInfo
Texture::Texture(uint16_t w, uint16_t h) : width {w}, height {h} {
    texture = std::make_shared<GLuint>();
}
//Start - TextureLoader
TextureLoader::TextureLoader(const char* f) : _folder_name {f} {}

void TextureLoader::_find_texture_names(std::vector<std::string>& t_file_names)
{
    DIR* dir = opendir(_folder_name);
	if (dir == nullptr) {
		std::cerr << "missing textures folder at: " << _folder_name << '\n';
		exit(1);
		return;
	}

    struct dirent* entry;
	while ((entry = readdir(dir)) != nullptr) {
			if (entry->d_type == DT_REG) {
			std::string s { entry->d_name };
			auto idx = s.find_last_of(".");
			if (idx != std::string::npos) {
				std::string f_ext = s.substr(idx + 1);
				if (f_ext == "jpg" || f_ext == "png") {
					t_file_names.push_back(s);
				}
			}
		}
	}
	closedir(dir);
}

void TextureLoader::load_textures(std::unordered_map<std::string,std::shared_ptr<Texture>>& textures)
{
    std::vector<std::string> t_file_names;
    _find_texture_names(t_file_names);

    for (auto tfn : t_file_names) {
        char buf[128];
        sprintf(buf,"%s/%s",_folder_name,tfn.c_str());
        Image img { buf };
        std::shared_ptr<Texture> pt = std::make_shared<Texture>(img.width, img.height);
        //Start - Gordon
        glGenTextures(1, pt->texture.get());
        glBindTexture(GL_TEXTURE_2D,*pt->texture);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

        unsigned char *data = buildAlphaData(&img);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        free (data);
        //End - Gordon
        std::clog << "loaded texture: " << tfn << '\n';
        textures.insert({tfn, std::move(pt)});
    }
}