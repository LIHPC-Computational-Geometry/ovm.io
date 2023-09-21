#include <geogram/basic/logger.h>

#include <iostream>

using namespace GEO;

int main(int argc, char** argv) {

    GEO::initialize();
	Logger::out("I/O") << "Hello from ovm.io" << std::endl;
    return 0;
}
