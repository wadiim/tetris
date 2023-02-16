#include "utils.h"
#include "term.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

void die(const char *msg)
{
	switch_to_normal_buffer();
	perror(msg);
	exit(errno);
}
