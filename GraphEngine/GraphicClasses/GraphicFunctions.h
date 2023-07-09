#pragma once

#include <GL/glew.h>
#include "../CommonClasses/common_classes.h"


namespace gre {
	bool GLEW_IS_OK = false;

    bool glew_is_ok() noexcept {
		glewExperimental = GL_TRUE;
		return GLEW_IS_OK = GLEW_IS_OK ? true : glewInit() == GLEW_OK;
	}
	
	void check_gl_errors(const char* filename, uint32_t line, const char* func_name) {
		GLenum error_code = glGetError();
		if (error_code == GL_NO_ERROR) {
			return;
		}

		std::string error;
		switch (error_code) {
			case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
			case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
			case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
			case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
			case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
			case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
			default:                               error = "UNKNOWN"; break;
		}

		throw GreRuntimeError(filename, line, std::string(func_name) + ", GL error with name \"" + error + "\".\n\n");
	}

	void check_color_value(const char* filename, uint32_t line, const char* func_name, const Vec3& color) {
		if (color.x < 0.0 || 1.0 < color.x || color.y < 0.0 || 1.0 < color.y || color.z < 0.0 || 1.0 < color.z) {
			throw GreInvalidArgument(filename, line, std::string(func_name) + ", invalid color value.\n\n");
		}
	}
}
