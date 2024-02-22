#pragma once

#include "Defines.h"
#include <string>

namespace hro
{
  class HAPI Material
    {
    public:
        Material();
        ~Material();

        std::string name;

        float ambient_color[3];
        float diffuse_color[3];
        float specular_color[3];

        std::string diffuse_texture_path;
        std::string specular_texture_path;
        std::string normal_texture_path;
    };
}
