#include "GraphicFunctions.h"


static bool GLEW_IS_OK = false;


namespace eng {
    bool glew_is_ok() noexcept {
        return GLEW_IS_OK = GLEW_IS_OK ? true : glewInit() == GLEW_OK;
    }
}
