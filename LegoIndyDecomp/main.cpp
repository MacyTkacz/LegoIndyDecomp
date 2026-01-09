#include <iostream>
#include <utils.h>
#include <fileio/fileio.h>

int main() {

	HEAP_INIT(16);

	std::cout << "hello world!" << std::endl;

	HEAP_FREE();
	return 0;

}

