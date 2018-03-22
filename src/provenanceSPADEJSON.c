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

char* relation_to_spade_json(struct relation_struct* e) {

}

char* used_to_spade_json(struct relation_struct* e) {

}

char* generated_to_spade_json(struct relation_struct* e) {

}

char* informed_to_spade_json(struct relation_struct* e) {

}

char* derived_to_spade_json(struct relation_struct* e) {

}

char* disc_to_spade_json(struct disc_node_struct* n) {

}

char* proc_to_spade_json(struct proc_prov_struct* n) {

}

char* task_to_spade_json(struct task_prov_struct* n) {

}

char* inode_to_spade_json(struct inode_prov_struct* n) {

}

char* sb_to_spade_json(struct sb_struct* n) {

}

char* msg_to_spade_json(struct msg_msg_struct* n) {

}

char* shm_to_spade_json(struct shm_struct* n) {

}

char* packet_to_spade_json(struct pck_struct* n) {

}

char* str_msg_to_spade_json(struct str_struct* n) {

}

char* addr_to_spade_json(struct address_struct* n) {

}

char* pathname_to_spade_json(struct file_name_struct* n) {

}

char* iattr_to_spade_json(struct iattr_prov_struct* n) {

}

char* xattr_to_spade_json(struct xattr_prov_struct* n) {

}

char* pckcnt_to_spade_json(struct pckcnt_struct* n) {

}

char* arg_to_spade_json(struct arg_struct* n) {

}
