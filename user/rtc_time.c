#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
	int timestamp = rtcrealtime();
	printf("Current UNIX timestamp: %d\n", timestamp);
	exit(0);
}
