#include "Application.h"

int main()
{
    Application application;

    while (!application.isFinished())
		application.loop();

	return EXIT_SUCCESS;
}
