
#include <errno.h>

/*
 * I'm not sure where to put these generic filler functions for drivers,
 * file system operations, etc.
 */

int NOP()
{
	return 0;
}

int NOTSUP()
{
	return -ENOTSUP;
}

