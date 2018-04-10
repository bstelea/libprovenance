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

#include "provenanceJSONcommon.h"

__thread char buffer[MAX_JSON_BUFFER_LENGTH];
#define BUFFER_LENGTH (MAX_JSON_BUFFER_LENGTH-strnlen(buffer, MAX_JSON_BUFFER_LENGTH))
static __thread char id[PROV_ID_STR_LEN];
static __thread char from[PROV_ID_STR_LEN];
static __thread char to[PROV_ID_STR_LEN];
char date[256];
pthread_rwlock_t  date_lock = PTHREAD_RWLOCK_INITIALIZER;

static inline void __init_node(char* type, char* id, const struct node_identifier* n){
  buffer[0]='\0';
  update_time();
  strncat(buffer, "\n{\n", BUFFER_LENGTH);
  __add_string_attribute("type", type, false);
  __add_string_attribute("id", id, true);
  strncat(buffer, ",\n\"annotations\": {\n", BUFFER_LENGTH);
  __add_uint64_attribute("node_id", n->id, false);
  __add_string_attribute("node_type", node_id_to_str(n->type), true);
  __add_uint32_attribute("boot_id", n->boot_id, true);
  __add_machine_id(n->machine_id, true);
  __add_uint32_attribute("version", n->version, true);
  __add_date_attribute(true);
}

static inline void __close_node( void ){
  strncat(buffer, "}\n},", BUFFER_LENGTH);
}

static inline void __init_relation(char* type,
                    char* from,
                    char* to,
                    char* id,
                    const struct relation_identifier* e
                  ) {
  buffer[0]='\0';
  update_time();
  strncat(buffer, "\n{\n", BUFFER_LENGTH);
  __add_string_attribute("type", type, false);
  __add_string_attribute("from", from, true);
  __add_string_attribute("to", to, true);
  strncat(buffer, ",\n\"annotations\": {\n", BUFFER_LENGTH);
  __add_string_attribute("id", id, false);
  __add_uint64_attribute("relation_id", e->id, true);
  __add_string_attribute("relation_type", relation_id_to_str(e->type), true);
  __add_uint32_attribute("boot_id", e->boot_id, true);
  __add_machine_id(e->machine_id, true);
  __add_date_attribute(true);
}

#define NODE_START(type) ID_ENCODE(n->identifier.buffer, PROV_IDENTIFIER_BUFFER_LENGTH, id, PROV_ID_STR_LEN);\
                    __init_node(type, id, &(n->identifier.node_id))

#define NODE_END() __close_node()

#define RELATION_START(type)  ID_ENCODE(e->identifier.buffer, PROV_IDENTIFIER_BUFFER_LENGTH, id, PROV_ID_STR_LEN);\
                        ID_ENCODE(e->snd.buffer, PROV_IDENTIFIER_BUFFER_LENGTH, from, PROV_ID_STR_LEN);\
                        ID_ENCODE(e->rcv.buffer, PROV_IDENTIFIER_BUFFER_LENGTH, to, PROV_ID_STR_LEN);\
                        __init_relation(type, from, to, id, &(e->identifier.relation_id))

#define RELATION_END() __close_node()

static inline void __relation_to_spade_json(struct relation_struct* e) {
  __add_uint64_attribute("jiffies", e->jiffies, true);
  if(e->allowed==FLOW_ALLOWED)
    __add_string_attribute("allowed", "true", true);
  else
    __add_string_attribute("allowed", "false", true);
  if(e->set==FILE_INFO_SET && e->offset>0)
    __add_int64_attribute("offset", e->offset, true); // just offset for now
  __add_uint64hex_attribute("flags", e->flags, true);
}

char* used_to_spade_json(struct relation_struct* e) {
  RELATION_START("Used");
  __relation_to_spade_json(e);
  RELATION_END();
  return buffer;
}

char* generated_to_spade_json(struct relation_struct* e) {
  RELATION_START("WasGeneratedBy");
  __relation_to_spade_json(e);
  RELATION_END();
  return buffer;
}

char* informed_to_spade_json(struct relation_struct* e) {
  RELATION_START("WasInformedBy");
  __relation_to_spade_json(e);
  RELATION_END();
  return buffer;
}

char* derived_to_spade_json(struct relation_struct* e) {
  RELATION_START("WasDerivedFrom");
  __relation_to_spade_json(e);
  RELATION_END();
  return buffer;
}

char* disc_to_spade_json(struct disc_node_struct* n) {
  buffer['0'];
  return buffer;
}

static __thread  char secctx[PATH_MAX];

char* proc_to_spade_json(struct proc_prov_struct* n) {
  NODE_START("Entity");
  __add_uint32_attribute("uid", n->uid, true);
  __add_uint32_attribute("gid", n->gid, true);
  __add_uint32_attribute("tgid", n->tgid, true);
  __add_uint32_attribute("utsns", n->utsns, true);
  __add_uint32_attribute("ipcns", n->ipcns, true);
  __add_uint32_attribute("mntns", n->mntns, true);
  __add_uint32_attribute("pidns", n->pidns, true);
  __add_uint32_attribute("netns", n->netns, true);
  __add_uint32_attribute("cgroupns", n->cgroupns, true);
  provenance_secid_to_secctx(n->secid, secctx, PATH_MAX);
  __add_string_attribute("secctx", secctx, true);
  __add_uint64_attribute("utime", n->utime, true);
  __add_uint64_attribute("stime", n->stime, true);
  __add_uint64_attribute("vm", n->vm, true);
  __add_uint64_attribute("rss", n->rss, true);
  __add_uint64_attribute("hw_vm", n->hw_vm, true);
  __add_uint64_attribute("hw_rss", n->hw_rss, true);
  __add_uint64_attribute("rbytes", n->rbytes, true);
  __add_uint64_attribute("wbytes", n->wbytes, true);
  __add_uint64_attribute("cancel_wbytes", n->cancel_wbytes, true);
  NODE_END();
  return buffer;
}

char* task_to_spade_json(struct task_prov_struct* n) {
  NODE_START("Activity");
  __add_uint32_attribute("pid", n->pid, true);
  __add_uint32_attribute("vpid", n->vpid, true);
  NODE_END();
  return buffer;
}

static __thread char uuid[UUID_STR_SIZE];

char* inode_to_spade_json(struct inode_prov_struct* n) {
  NODE_START("Entity");
  __add_uint32_attribute("uid", n->uid, true);
  __add_uint32_attribute("gid", n->gid, true);
  __add_uint32hex_attribute("mode", n->mode, true);
  __add_string_attribute("secctx", secctx, true);
  __add_uint32_attribute("ino", n->ino, true);
  __add_string_attribute("uuid", uuid_to_str(n->sb_uuid, uuid, UUID_STR_SIZE), true);
  NODE_END();
  return buffer;
}

char* sb_to_spade_json(struct sb_struct* n) {
  NODE_START("Entity");
  __add_string_attribute("cf:uuid", uuid_to_str(n->uuid, uuid, UUID_STR_SIZE), true);
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
  __add_uint32hex_attribute("cf:mode", n->mode, true);
  NODE_END();
  return buffer;
}

char* packet_to_spade_json(struct pck_struct* n) {
  ID_ENCODE(n->identifier.buffer, PROV_IDENTIFIER_BUFFER_LENGTH, id, PROV_ID_STR_LEN);
  buffer[0]='\0';
  update_time();
  strncat(buffer, "\n{\n", BUFFER_LENGTH);
  __add_string_attribute("type", "Entity", false);
  __add_string_attribute("id", id, true);
  strncat(buffer, ",\n\"annotations\": {\n", BUFFER_LENGTH);
  __add_string_attribute("node_type", "packet", false);
  __add_date_attribute(true);
  __add_uint32_attribute("packet_id", n->identifier.packet_id.id, true);
  __add_uint32_attribute("seq", n->identifier.packet_id.seq, true);
  __add_ipv4_attribute("sender", n->identifier.packet_id.snd_ip, n->identifier.packet_id.snd_port, true);
  __add_ipv4_attribute("receiver", n->identifier.packet_id.rcv_ip, n->identifier.packet_id.rcv_port, true);
  __add_uint64_attribute("jiffies", n->jiffies, true);
  NODE_END();
  return buffer;
}

char* str_msg_to_spade_json(struct str_struct* n) {
  NODE_START("Entity");
  __add_string_attribute("log", n->str, true);
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
  __add_string_attribute("pathname", n->name, true);
  NODE_END();
  return buffer;
}

char* iattr_to_spade_json(struct iattr_prov_struct* n) {
  NODE_START("Entity");
  __add_uint32hex_attribute("valid", n->valid, true);
  __add_uint32hex_attribute("mode", n->mode, true);
  __add_uint32_attribute("uid", n->uid, true);
  __add_uint32_attribute("gid", n->gid, true);
  __add_int64_attribute("size", n->size, true);
  __add_int64_attribute("atime", n->atime, true);
  __add_int64_attribute("ctime", n->ctime, true);
  __add_int64_attribute("mtime", n->mtime, true);
  NODE_END();
  return buffer;
}

char* xattr_to_spade_json(struct xattr_prov_struct* n) {
  NODE_START("Entity");
  __add_string_attribute("name", n->name, true);
  if(n->size>0){
    __add_uint32_attribute("size", n->size, true);
    // TODO record value when present
  }
  NODE_END();
  return buffer;
}

char* pckcnt_to_spade_json(struct pckcnt_struct* n) {
  char* cntenc;
  NODE_START("Entity");
  cntenc = malloc( encode64Bound(n->length) );
  base64encode(n->content, n->length, cntenc, encode64Bound(n->length));
  __add_string_attribute("content", cntenc, true);
  free(cntenc);
  __add_uint32_attribute("length", n->length, true);
  if(n->truncated==PROV_TRUNCATED)
    __add_string_attribute("truncated", "true", true);
  else
    __add_string_attribute("truncated", "false", true);
  NODE_END();
  return buffer;
}

char* arg_to_spade_json(struct arg_struct* n) {
  int i;
  char* tmp;
  NODE_START("Entity");
  for(i=0; i<n->length; i++){
    if(n->value[i]=='\\')
      n->value[i]='/';
    if(n->value[i]=='\n')
      n->value[i]=' ';
    if(n->value[i]=='\t')
      n->value[i]=' ';
  }
  tmp = repl_str(n->value, "\"", "\\\"");
  if(tmp==NULL)
    tmp = n->value;
  __add_string_attribute("value", tmp, true);
  if(n->truncated==PROV_TRUNCATED)
    __add_string_attribute("truncated", "true", true);
  else
    __add_string_attribute("truncated", "false", true);
  NODE_END();
  if(tmp != n->value)
    free(tmp);
  return buffer;
}

char* machine_description_spade_json( void ){
  char tmp[64];
  uint32_t machine_id;
  struct utsname machine_info;
  int lsm_fd;
  char lsm_list[2048];

  memset(lsm_list, 0, 2048);

  provenance_get_machine_id(&machine_id);
  uname(&machine_info);

  lsm_fd = open(LSM_LIST, O_RDONLY);
  read(lsm_fd, lsm_list, 2048);

  buffer[0]='\0';
  strncat(buffer, "[{", BUFFER_LENGTH);
  __add_string_attribute("type", "Entity", false);
  __add_string_attribute("id", utoa(machine_id, tmp, DECIMAL), true);
  strncat(buffer, ",\n\"annotations\": {\n", BUFFER_LENGTH);
  provenance_version(tmp, 64);
  __add_string_attribute("camflow", tmp, false);
  provenance_lib_version(tmp, 64);
  __add_string_attribute("libprovenance", tmp, true);
  __add_string_attribute("sysname", machine_info.sysname, true);
  __add_string_attribute("nodename", machine_info.nodename, true);
  __add_string_attribute("release", machine_info.release, true);
  __add_string_attribute("version", machine_info.version, true);
  __add_string_attribute("machine", machine_info.machine, true);
  __add_string_attribute("lsm_list", lsm_list, true);
  update_time();
  pthread_rwlock_rdlock(&date_lock);
  __add_string_attribute("date", date, true);
  pthread_rwlock_unlock(&date_lock);
  strncat(buffer, "}}]", BUFFER_LENGTH);
  return buffer;
}

static char* json;
static inline void init_buffer( void ){
  json = (char*)malloc(MAX_JSON_BUFFER_LENGTH);
  memset(buffer, 0, MAX_JSON_BUFFER_LENGTH);
  memset(json, 0, MAX_JSON_BUFFER_LENGTH);
}

static void (*print_json)(char* json);

void set_SPADEJSON_callback( void (*fcn)(char* json) ){
  init_buffer();
  print_json = fcn;
}

static inline bool __append(char* buff){
  if (strlen(buff) + 2 > MAX_JSON_BUFFER_LENGTH - strlen(json) - 1){ // not enough space
    return false;
  }
  strncat(json, buff, MAX_JSON_BUFFER_LENGTH - strlen(json) - 1); // copy up to free space
  return true;
}

static pthread_mutex_t l_json =  PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
void spade_json_append(char* buff){
  pthread_mutex_lock(&l_json);
  // we cannot append buffer is full, need to print json out
  if(!__append(buff)){
    flush_spade_json();
    pthread_mutex_unlock(&l_json);
    spade_json_append(buff);
    return;
  }
  pthread_mutex_unlock(&l_json);
}

static inline char* ready_to_print(){
  char *tmp;
  char *out;

  if (json[0]==0)
    return NULL;
  out = (char*)malloc(MAX_JSON_BUFFER_LENGTH+1);
  pthread_mutex_lock(&l_json);
  out[0] = '[';
  memcpy(&out[1], json, MAX_JSON_BUFFER_LENGTH);
  tmp = out + strlen(json);
  *tmp = ']';
  memset(json, 0, MAX_JSON_BUFFER_LENGTH);
  pthread_mutex_unlock(&l_json);
  return out;
}

static bool writing_out = false;
static pthread_mutex_t l_flush =  PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

void flush_spade_json(){
  bool should_flush=false;
  char* tmp;

  pthread_mutex_lock(&l_flush);
  if(!writing_out){
    writing_out = true;
    should_flush = true;
    update_time(); // we update the time
  }
  pthread_mutex_unlock(&l_flush);

  if(should_flush){
    tmp = ready_to_print();
    if(tmp!=NULL){
      print_json(tmp);
      free(tmp);
    }
    pthread_mutex_lock(&l_flush);
    writing_out = false;
    pthread_mutex_unlock(&l_flush);
  }
}
