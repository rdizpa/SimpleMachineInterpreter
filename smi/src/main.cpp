#include <iostream>
#include <fstream>
#include <sstream>

#include "core/interpreter.h"

int main(int argc, char *argv[]) {
	smi::interpreter::Interpreter interp;

	if (argc < 2) {
		std::cout << "Usage: smi [file]" << std::endl;
		return 0;
	}

	std::string filename = argv[1];
	
	try {
		std::ifstream ifs(filename);
		std::ostringstream buffer;

		buffer << ifs.rdbuf();

		ifs.close();

		if (interp.eval(buffer.str()) != smi::interpreter::INTERPRETER_OK) {
			std::cout << "Error: " << getLastError() << std::endl;
		}

		for (auto key : interp.getMemoryKeys()) {
			std::cout << key << "\t" << std::hex << interp.getMemoryValue(key) << std::endl;
		}
	}
	catch (std::exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
