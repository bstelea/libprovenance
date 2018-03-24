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

#define MAX_SPADEJSON_BUFFER_EXP     13
#define MAX_SPADEJSON_BUFFER_LENGTH  ((1 << MAX_SPADEJSON_BUFFER_EXP)*sizeof(uint8_t))

static __thread char buffer[MAX_SPADEJSON_BUFFER_LENGTH];
#define BUFFER_LENGTH (MAX_SPADEJSON_BUFFER_LENGTH-strnlen(buffer, MAX_SPADEJSON_BUFFER_LENGTH))
static __thread char id[PROV_ID_STR_LEN];
static __thread char from[PROV_ID_STR_LEN];
static __thread char to[PROV_ID_STR_LEN];

static inline void __init_node(char* type, char* id){
  buffer[0]='\0';
  strncat(buffer, "{\n\"type\": \"", BUFFER_LENGTH);
  strncat(buffer, type, BUFFER_LENGTH);
  strncat(buffer, "\",\n", BUFFER_LENGTH);
  strncat(buffer, "\"id\": \"", BUFFER_LENGTH);
  strncat(buffer, id, BUFFER_LENGTH);
  strncat(buffer, "\",\n", BUFFER_LENGTH);
  strncat(buffer, "\"annotations\": {\n", BUFFER_LENGTH);
}

static inline void __close_node( void ){
  buffer[0]='\0';
  strncat(buffer, "}\n}\n", BUFFER_LENGTH);
}

static inline void __init_relation(char* type,
                    char* from,
                    char* to,
                    char* id
                  ) {
  buffer[0]='\0';
  strncat(buffer, "{\n\"type\": \"", BUFFER_LENGTH);
  strncat(buffer, type, BUFFER_LENGTH);
  strncat(buffer, "\",\n", BUFFER_LENGTH);
  strncat(buffer, "\"from\": \"", BUFFER_LENGTH);
  strncat(buffer, id, BUFFER_LENGTH);
  strncat(buffer, "\",\n", BUFFER_LENGTH);
  strncat(buffer, "\"to\": \"", BUFFER_LENGTH);
  strncat(buffer, id, BUFFER_LENGTH);
  strncat(buffer, "\",\n", BUFFER_LENGTH);
  strncat(buffer, "\"annotations\": {\n", BUFFER_LENGTH);
  strncat(buffer, "\"id\": \"", BUFFER_LENGTH);
  strncat(buffer, id, BUFFER_LENGTH);
  strncat(buffer, "\",\n", BUFFER_LENGTH);
}

#define NODE_START(type) ID_ENCODE(n->identifier.buffer, PROV_IDENTIFIER_BUFFER_LENGTH, id, PROV_ID_STR_LEN);\
                    __init_node(type, id)

#define NODE_END() __close_node()

#define RELATION_START(type)  ID_ENCODE(e->identifier.buffer, PROV_IDENTIFIER_BUFFER_LENGTH, id, PROV_ID_STR_LEN);\
                        ID_ENCODE(e->snd.buffer, PROV_IDENTIFIER_BUFFER_LENGTH, from, PROV_ID_STR_LEN);\
                        ID_ENCODE(e->rcv.buffer, PROV_IDENTIFIER_BUFFER_LENGTH, to, PROV_ID_STR_LEN);\
                        __init_relation(type, from, to, id)

#define RELATION_END() __close_node()

static inline char* __relation_to_spade_json(struct relation_struct* e) {
  return buffer;
}

char* used_to_spade_json(struct relation_struct* e) {
  RELATION_START("Used");
  RELATION_END();
  return buffer;
}

char* generated_to_spade_json(struct relation_struct* e) {
  RELATION_START("WasGeneratedBy");
  RELATION_END();
  return buffer;
}

char* informed_to_spade_json(struct relation_struct* e) {
  RELATION_START("WasInformedBy");
  RELATION_END();
  return buffer;
}

char* derived_to_spade_json(struct relation_struct* e) {
  RELATION_START("WasDerivedFrom");
  RELATION_END();
  return buffer;
}

char* disc_to_spade_json(struct disc_node_struct* n) {
  buffer['0'];
  return buffer;
}

char* proc_to_spade_json(struct proc_prov_struct* n) {
  NODE_START("Entity");
  NODE_END();
  return buffer;
}

char* task_to_spade_json(struct task_prov_struct* n) {
  NODE_START("Activity");
  NODE_END();
  return buffer;
}

char* inode_to_spade_json(struct inode_prov_struct* n) {
  NODE_START("Entity");
  NODE_END();
  return buffer;
}

char* sb_to_spade_json(struct sb_struct* n) {
  NODE_START("Entity");
  NODE_END();
  return buffer;
}

char* msg_to_spade_json(struct msg_msg_struct* n) {
  NODE_START("Entity");
  NODE_END();
  return buffer;
}

char* shm_to_spade_json(struct shm_struct* n) {
  NODE_START("Entity");
  NODE_END();
  return buffer;
}

char* packet_to_spade_json(struct pck_struct* n) {
  NODE_START("Entity");
  NODE_END();
  return buffer;
}

char* str_msg_to_spade_json(struct str_struct* n) {
  NODE_START("Entity");
  NODE_END();
  return buffer;
}

char* addr_to_spade_json(struct address_struct* n) {
  NODE_START("Entity");
  NODE_END();
  return buffer;
}

char* pathname_to_spade_json(struct file_name_struct* n) {
  NODE_START("Entity");
  NODE_END();
  return buffer;
}

char* iattr_to_spade_json(struct iattr_prov_struct* n) {
  NODE_START("Entity");
  NODE_END();
  return buffer;
}

char* xattr_to_spade_json(struct xattr_prov_struct* n) {
  NODE_START("Entity");
  NODE_END();
  return buffer;
}

char* pckcnt_to_spade_json(struct pckcnt_struct* n) {
  NODE_START("Entity");
  NODE_END();
  return buffer;
}

char* arg_to_spade_json(struct arg_struct* n) {
  NODE_START("Entity");
  NODE_END();
  return buffer;
}
