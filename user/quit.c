#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
	printf("Shutting down system...\n");
	quit();
	printf("Shutdown failed!\n");
	exit(1);
}
