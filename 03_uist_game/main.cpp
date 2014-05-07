#include "Application.h"

int main(int argc, char *argv[])
{
    Application application(argc, argv);

    while (!application.isFinished())
		application.loop();

	return EXIT_SUCCESS;
}
