#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{	
	printf("Rebooting system...\n");
	reboot();
	printf("Reboot failed!\n");
	exit(1);
}
