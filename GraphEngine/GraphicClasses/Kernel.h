#pragma once

#include <cassert>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>


class Kernel {
	eng::Matrix kernel = eng::Matrix(3, 3, 0);

public:
	Kernel() {
		kernel[1][1] = 1;
	}

	Kernel(std::vector < std::vector < double > > init) {
		if (init.size() != 3 || init[0].size() != 3) {
			std::cout << "ERROR::KERNEL::BUILDER\n" << "Invalid kernel size.\n";
			assert(0);
		}

		kernel = eng::Matrix(init);
	}

	Kernel(std::string kernel_path) {
		std::ifstream kernel_file(kernel_path + ".kernel");

		if (kernel_file.fail()) {
			std::cout << "ERROR::KERNEL::BUILDER\n" << "The kernel file does not exist.\n";
			assert(0);
		}

		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++)
				kernel_file >> kernel[i][j];
		}
	}

	void use(eng::Shader<eng::ShaderType::POST>* shader) {
		try {
			shader->set_uniform_1fv("kernel", 9, &std::vector<float>(kernel)[0]);
		}
		catch (const std::exception& error) {
			std::cout << "ERROR::KERNEL::USE\n" << "Unknown error, description:\n" << error.what() << "\n";
			assert(0);
		}
	}
};
