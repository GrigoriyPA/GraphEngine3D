#include "Kernel.hpp"


// Kernel
namespace gre {
    // Constructors
    Kernel::Kernel() noexcept {
        for (size_t i = 0; i < 3; ++i) {
            for (size_t j = 0; j < 3; ++j) {
                kernel_[i][j] = 0.0;
            }
        }

        kernel_[1][1] = 1.0;
    }

    Kernel::Kernel(GLuint offset) noexcept : Kernel() {
        offset_ = offset;
    }

    double* Kernel::operator[](size_t index) {
        GRE_ENSURE(index <= 2, GreOutOfRange, "index out of range");

        return kernel_[index];
    }

    const double* Kernel::operator[](size_t index) const {
        GRE_ENSURE(index <= 2, GreOutOfRange, "index out of range");

        return kernel_[index];
    }

    bool Kernel::operator==(const Kernel& other) const noexcept {
        for (uint32_t i = 0; i < 3; ++i) {
            for (uint32_t j = 0; j < 3; ++j) {
                if (!equality(kernel_[i][j], other[i][j])) {
                    return false;
                }
            }
        }
        return true;
    }

    bool Kernel::operator!=(const Kernel& other) const noexcept {
        return !(*this == other);
    }

    // Seters
    void Kernel::set_offset(GLuint offset) noexcept {
        offset_ = offset;
    }

    // Getters
    GLuint Kernel::get_offset() const noexcept {
        return offset_;
    }

    // Uploading into shader

    // POST shader expected
    void Kernel::set_uniforms(const Shader& shader) const {
        shader.set_uniform_i("offset", offset_);

        std::vector<GLfloat> data;
        data.reserve(9);
        for (size_t j = 0; j < 3; ++j) {
            for (size_t i = 0; i < 3; ++i) {
                data.push_back(static_cast<GLfloat>(kernel_[i][j]));
            }
        }

        shader.set_uniform_1fv("kernel", 9, &(data[0]));
    }

    // External operators
    std::istream& operator>>(std::istream& fin, Kernel& kernel) {
        fin >> kernel.offset_;
        for (size_t i = 0; i < 3; ++i) {
            for (size_t j = 0; j < 3; ++j) {
                fin >> kernel[i][j];
            }
        }
        return fin;
    }
}  // namespace gre
