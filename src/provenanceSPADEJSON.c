/*
*
* Author: Thomas Pasquier <tfjmp2@cl.cam.ac.uk>
*
* Copyright (C) 2015-2018 University of Cambridge, Harvard University
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2, as
* published by the Free Software Foundation.
*
*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <netdb.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <fcntl.h>
#include <sys/utsname.h>
#include <linux/provenance_types.h>

#include "provenance.h"
#include "provenanceSPADEJSON.h"
#include "provenanceutils.h"
