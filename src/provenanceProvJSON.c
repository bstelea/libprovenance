/*
*
* provenanceProvJSON.c
*
* Author: Thomas Pasquier <tfjmp@seas.harvard.edu>
*
* Copyright (C) 2016 Harvard University
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
#include <netdb.h>
#include <pthread.h>

#include "provenancelib.h"
#include "provenancePovJSON.h"
#include "provenanceutils.h"

#define MAX_PROVJSON_BUFFER_LENGTH PATH_MAX*2

static pthread_mutex_t l_flush =  PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
static pthread_mutex_t l_activity =  PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
static pthread_mutex_t l_agent =  PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
static pthread_mutex_t l_entity =  PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
static pthread_mutex_t l_edge =  PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
static pthread_mutex_t l_used =  PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
static pthread_mutex_t l_generated =  PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
static pthread_mutex_t l_informed =  PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
static pthread_mutex_t l_derived =  PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
static pthread_mutex_t l_message =  PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

static char activity[MAX_PROVJSON_BUFFER_LENGTH];
static char agent[MAX_PROVJSON_BUFFER_LENGTH];
static char entity[MAX_PROVJSON_BUFFER_LENGTH];
static char edge[MAX_PROVJSON_BUFFER_LENGTH];
static char used[MAX_PROVJSON_BUFFER_LENGTH];
static char generated[MAX_PROVJSON_BUFFER_LENGTH];
static char informed[MAX_PROVJSON_BUFFER_LENGTH];
static char derived[MAX_PROVJSON_BUFFER_LENGTH];
static char message[MAX_PROVJSON_BUFFER_LENGTH];

bool writing_out = false;

static void (*print_json)(char* json);

int disclose_node_ProvJSON(uint32_t type, const char* content, prov_identifier_t* identifier){
  int err;
  struct disc_node_struct node;

  strncpy(node.content, content, PATH_MAX);
  node.length=strnlen(content, PATH_MAX);
  node.identifier.node_id.type=type;

  if(err = provenance_disclose_node(&node)<0){
    return err;
  }
  memcpy(identifier, &node.identifier, sizeof(prov_identifier_t));
  return err;
}

int disclose_edge_ProvJSON(uint32_t type, prov_identifier_t* sender, prov_identifier_t* receiver){
  struct edge_struct edge;
  edge.type=type;
  edge.allowed=true;
  memcpy(&edge.snd, sender, sizeof(prov_identifier_t));
  memcpy(&edge.rcv, receiver, sizeof(prov_identifier_t));
  return provenance_disclose_edge(&edge);
}

void set_ProvJSON_callback( void (*fcn)(char* json) ){
  print_json = fcn;
}

static inline bool __append(char destination[MAX_PROVJSON_BUFFER_LENGTH], char* source){
  if (strlen(source) + 2 > MAX_PROVJSON_BUFFER_LENGTH - strlen(destination)){ // not enough space
    return false;
  }
  // add the comma
  if(destination[0]!='\0')
    strcat(destination, ",");
  strncat(destination, source, MAX_PROVJSON_BUFFER_LENGTH - strlen(destination) - 1); // copy up to free space
  return true;
}

#define JSON_START "{\"prefix\":{"
#define JSON_ACTIVITY "}, \"activity\":{"
#define JSON_AGENT "}, \"agent\":{"
#define JSON_ENTITY "}, \"entity\":{"
#define JSON_MESSAGE "}, \"message\":{"
#define JSON_EDGE "}, \"edge\":{"
#define JSON_USED "}, \"used\":{"
#define JSON_GENERATED "}, \"wasGeneratedBy\":{"
#define JSON_INFORMED "}, \"wasInformedBy\":{"
#define JSON_DERIVED "}, \"wasDerivedFrom\":{"
#define JSON_END "}}"

#define JSON_LENGTH (strlen(JSON_START)\
                      +strlen(JSON_ACTIVITY)\
                      +strlen(JSON_AGENT)\
                      +strlen(JSON_ENTITY)\
                      +strlen(JSON_MESSAGE)\
                      +strlen(JSON_EDGE)\
                      +strlen(JSON_USED)\
                      +strlen(JSON_GENERATED)\
                      +strlen(JSON_INFORMED)\
                      +strlen(JSON_DERIVED)\
                      +strlen(JSON_END)\
                      +strlen(prefix_json())\
                      +strlen(activity)\
                      +strlen(agent)\
                      +strlen(entity)\
                      +strlen(message)\
                      +strlen(edge)\
                      +strlen(used)\
                      +strlen(generated)\
                      +strlen(derived)\
                      +strlen(informed)\
                      +1)

#define str_is_empty(str) (str[0]=='\0')
// we create the JSON string to be sent to the call back
static inline char* ready_to_print(){
  pthread_mutex_lock(&l_derived);
  pthread_mutex_lock(&l_informed);
  pthread_mutex_lock(&l_generated);
  pthread_mutex_lock(&l_used);
  pthread_mutex_lock(&l_edge);
  pthread_mutex_lock(&l_message);
  pthread_mutex_lock(&l_entity);
  pthread_mutex_lock(&l_agent);
  pthread_mutex_lock(&l_activity);

  /* allocate memory */
  char* json = (char*)malloc(JSON_LENGTH * sizeof(char));
  json[0]='\0';

  strcat(json, JSON_START);
  strcat(json, prefix_json());

  /* recording activities */
  if(!str_is_empty(activity)){
    strcat(json, JSON_ACTIVITY);
    strcat(json, activity);
    memset(activity, '\0', MAX_PROVJSON_BUFFER_LENGTH);
  }
  pthread_mutex_unlock(&l_activity);

  /* recording agents */
  if(!str_is_empty(agent)){
    strcat(json, JSON_AGENT);
    strcat(json, agent);
    memset(agent, '\0', MAX_PROVJSON_BUFFER_LENGTH);
  }
  pthread_mutex_unlock(&l_agent);

  /* recording entities */
  if(!str_is_empty(entity)){
    strcat(json, JSON_ENTITY);
    strcat(json, entity);
    memset(entity, '\0', MAX_PROVJSON_BUFFER_LENGTH);
  }
  pthread_mutex_unlock(&l_entity);

  /* recording entities */
  if(!str_is_empty(message)){
    strcat(json, JSON_MESSAGE);
    strcat(json, message);
    memset(message, '\0', MAX_PROVJSON_BUFFER_LENGTH);
  }
  pthread_mutex_unlock(&l_message);

  /* recording edges */
  if(!str_is_empty(edge)){
    strcat(json, JSON_EDGE);
    strcat(json, edge);
    memset(edge, '\0', MAX_PROVJSON_BUFFER_LENGTH);
  }
  pthread_mutex_unlock(&l_edge);

  if(!str_is_empty(used)){
    strcat(json, JSON_USED);
    strcat(json, used);
    memset(used, '\0', MAX_PROVJSON_BUFFER_LENGTH);
  }
  pthread_mutex_unlock(&l_used);

  if(!str_is_empty(generated)){
    strcat(json, JSON_GENERATED);
    strcat(json, generated);
    memset(generated, '\0', MAX_PROVJSON_BUFFER_LENGTH);
  }
  pthread_mutex_unlock(&l_generated);

  if(!str_is_empty(informed)){
    strcat(json, JSON_INFORMED);
    strcat(json, informed);
    memset(informed, '\0', MAX_PROVJSON_BUFFER_LENGTH);
  }
  pthread_mutex_unlock(&l_informed);

  if(!str_is_empty(derived)){
    strcat(json, JSON_DERIVED);
    strcat(json, derived);
    memset(derived, '\0', MAX_PROVJSON_BUFFER_LENGTH);
  }
  pthread_mutex_unlock(&l_derived);

  strcat(json, JSON_END);
  return json;
}

void flush_json(){
  bool should_flush=false;
  char* json;

  pthread_mutex_lock(&l_flush);
  if(!writing_out){
    writing_out = true;
    should_flush = true;
  }
  pthread_mutex_unlock(&l_flush);

  if(should_flush){
    json = ready_to_print();
    print_json(json);
    free(json);
    pthread_mutex_lock(&l_flush);
    writing_out = false;
    pthread_mutex_unlock(&l_flush);
  }
}

static inline void json_append(pthread_mutex_t* l, char destination[MAX_PROVJSON_BUFFER_LENGTH], char* source){
  pthread_mutex_lock(l);
  // we cannot append buffer is full, need to print json out
  if(!__append(destination, source)){
    flush_json();
    pthread_mutex_unlock(l);
    json_append(l, destination, source);
    return;
  }
  pthread_mutex_unlock(l);
}

void append_activity(char* json_element){
  json_append(&l_activity, activity, json_element);
}

void append_agent(char* json_element){
  json_append(&l_agent, agent, json_element);
}

void append_entity(char* json_element){
  json_append(&l_entity, entity, json_element);
}

void append_message(char* json_element){
  json_append(&l_message, message, json_element);
}

void append_edge(char* json_element){
  json_append(&l_edge, edge, json_element);
}

void append_used(char* json_element){
  json_append(&l_used, used, json_element);
}

void append_generated(char* json_element){
  json_append(&l_generated, generated, json_element);
}

void append_informed(char* json_element){
  json_append(&l_informed, informed, json_element);
}

void append_derived(char* json_element){
  json_append(&l_derived, derived, json_element);
}

static __thread char buffer[MAX_PROVJSON_BUFFER_LENGTH];

#define PROV_ID_STR_LEN HEXIFY_OUTPUT_LENGTH(PROV_IDENTIFIER_BUFFER_LENGTH)
#define ID_ENCODE hexify

static __thread char id[PROV_ID_STR_LEN];
static __thread char sender[PROV_ID_STR_LEN];
static __thread char receiver[PROV_ID_STR_LEN];
static __thread char parent_id[PROV_ID_STR_LEN];

#define EDGE_PREP_IDs(e) ID_ENCODE(e->identifier.buffer, PROV_IDENTIFIER_BUFFER_LENGTH, id, PROV_ID_STR_LEN);\
                        ID_ENCODE(e->snd.buffer, PROV_IDENTIFIER_BUFFER_LENGTH, sender, PROV_ID_STR_LEN);\
                        ID_ENCODE(e->rcv.buffer, PROV_IDENTIFIER_BUFFER_LENGTH, receiver, PROV_ID_STR_LEN);

#define DISC_PREP_IDs(n) ID_ENCODE(n->identifier.buffer, PROV_IDENTIFIER_BUFFER_LENGTH, id, PROV_ID_STR_LEN);\
                        ID_ENCODE(n->parent.buffer, PROV_IDENTIFIER_BUFFER_LENGTH, parent_id, PROV_ID_STR_LEN);

#define NODE_PREP_IDs(n) hexify(n->identifier.buffer, PROV_IDENTIFIER_BUFFER_LENGTH, id, PROV_ID_STR_LEN);

char* node_info_to_json(char* buf, struct node_identifier* n){
  sprintf(buf, "\"cf:type\": %u, \"cf:id\":%llu, \"cf:boot_id\":%u, \"cf:machine_id\":%u, \"cf:version\":%u", n->type, n->id, n->boot_id, n->machine_id, n->version);
  return buf;
}

char* edge_info_to_json(char* buf, struct edge_identifier* e){
  sprintf(buf, "\"cf:id\":%llu, \"cf:boot_id\":%u, \"cf:machine_id\":%u", e->id, e->boot_id, e->machine_id);
  return buf;
}

static char* bool_str[] = {"false", "true"};



static const char ED_STR_UNKNOWN []               = "unknown";
static const char ED_STR_READ []                  = "read";
static const char ED_STR_WRITE []                 = "write";
static const char ED_STR_CREATE []                = "create";
static const char ED_STR_PASS []                  = "pass";
static const char ED_STR_CHANGE []                = "change";
static const char ED_STR_MMAP []                  = "mmap";
static const char ED_STR_ATTACH []                = "attach";
static const char ED_STR_ASSOCIATE []             = "associate";
static const char ED_STR_BIND []                  = "bind";
static const char ED_STR_CONNECT []               = "connect";
static const char ED_STR_LISTEN []                = "listen";
static const char ED_STR_ACCEPT []                = "accept";
static const char ED_STR_OPEN []                  = "open";
static const char ED_STR_PARENT []                = "parent";
static const char ED_STR_VERSION []               = "version";
static const char ED_STR_LINK []                  = "link";
static const char ED_STR_NAMED []                 = "named";
static const char ED_STR_IFC []                   = "ifc";
static const char ED_STR_EXEC []                  = "exec";
static const char ED_STR_FORK []                  = "fork";
static const char ED_STR_VERSION_PROCESS []       = "version";
static const char ED_STR_SEARCH []                = "search";

static inline const char* edge_str(uint32_t type){
  switch(type){
    case ED_READ:
      return ED_STR_READ;
    case ED_WRITE:
      return ED_STR_WRITE;
    case ED_CREATE:
      return ED_STR_CREATE;
    case ED_PASS:
      return ED_STR_PASS;
    case ED_CHANGE:
      return ED_STR_CHANGE;
    case ED_MMAP:
      return ED_STR_MMAP;
    case ED_ATTACH:
      return ED_STR_ATTACH;
    case ED_ASSOCIATE:
      return ED_STR_ASSOCIATE;
    case ED_BIND:
      return ED_STR_BIND;
    case ED_CONNECT:
      return ED_STR_CONNECT;
    case ED_LISTEN:
      return ED_STR_LISTEN;
    case ED_ACCEPT:
      return ED_STR_ACCEPT;
    case ED_OPEN:
      return ED_STR_OPEN;
    case ED_PARENT:
      return ED_STR_PARENT;
    case ED_VERSION:
      return ED_STR_VERSION;
    case ED_LINK:
      return ED_STR_LINK;
    case ED_NAMED:
      return ED_STR_NAMED;
    case ED_IFC:
      return ED_STR_IFC;
    case ED_EXEC:
      return ED_STR_EXEC;
    case ED_FORK:
      return ED_STR_FORK;
    case ED_VERSION_PROCESS:
      return ED_STR_VERSION_PROCESS;
    case ED_SEARCH:
      return ED_STR_SEARCH;
    default:
      return ED_STR_UNKNOWN;
  }
}

char* edge_to_json(struct edge_struct* e){
  char edge_info[1024];
  EDGE_PREP_IDs(e);
  sprintf(buffer, "\"cf:%s\":{%s, \"cf:type\":\"%s\", \"cf:allowed\":%s, \"cf:sender\":\"cf:%s\", \"cf:receiver\":\"cf:%s\"}",
    id,
    edge_info_to_json(edge_info, &e->identifier.edge_id),
    edge_str(e->type),
    bool_str[e->allowed],
    sender,
    receiver);
  return buffer;
}

char* used_to_json(struct edge_struct* e){
  char edge_info[1024];
  EDGE_PREP_IDs(e);
  sprintf(buffer, "\"cf:%s\":{%s, \"cf:type\":\"%s\", \"cf:allowed\":%s, \"prov:entity\":\"cf:%s\", \"prov:activity\":\"cf:%s\"}",
    id,
    edge_info_to_json(edge_info, &e->identifier.edge_id),
    edge_str(e->type),
    bool_str[e->allowed],
    sender,
    receiver);
  return buffer;
}

char* generated_to_json(struct edge_struct* e){
  char edge_info[1024];
  EDGE_PREP_IDs(e);
  sprintf(buffer, "\"cf:%s\":{%s, \"cf:type\":\"%s\", \"cf:allowed\":%s, \"prov:activity\":\"cf:%s\", \"prov:entity\":\"cf:%s\"}",
    id,
    edge_info_to_json(edge_info, &e->identifier.edge_id),
    edge_str(e->type),
    bool_str[e->allowed],
    sender,
    receiver);
  return buffer;
}

char* informed_to_json(struct edge_struct* e){
  char edge_info[1024];
  EDGE_PREP_IDs(e);
  sprintf(buffer, "\"cf:%s\":{%s, \"cf:type\":\"%s\", \"cf:allowed\":%s, \"prov:informant\":\"cf:%s\", \"prov:informed\":\"cf:%s\"}",
    id,
    edge_info_to_json(edge_info, &e->identifier.edge_id),
    edge_str(e->type),
    bool_str[e->allowed],
    sender,
    receiver);
  return buffer;
}

char* derived_to_json(struct edge_struct* e){
  char edge_info[1024];
  EDGE_PREP_IDs(e);
  sprintf(buffer, "\"cf:%s\":{%s, \"cf:type\":\"%s\", \"cf:allowed\":%s, \"prov:usedEntity\":\"cf:%s\", \"prov:generatedEntity\":\"cf:%s\"}",
    id,
    edge_info_to_json(edge_info, &e->identifier.edge_id),
    edge_str(e->type),
    bool_str[e->allowed],
    sender,
    receiver);
  return buffer;
}

char* disc_to_json(struct disc_node_struct* n){
  char node_info[1024];
  DISC_PREP_IDs(n);
  if(n->length > 0){
    sprintf(buffer, "\"cf:%s\" : {%s, \"cf:parent_id\":\"cf:%s\", %s}",
      id,
      node_info_to_json(node_info, &n->identifier.node_id),
      parent_id,
      n->content);
  }else{
    sprintf(buffer, "\"cf:%s\" : {%s}, \"cf:parent_id\":\"cf:%s\"",
      id,
      node_info_to_json(node_info, &n->identifier.node_id),
      parent_id);
  }
  return buffer;
}

char* task_to_json(struct task_prov_struct* n){
  char node_info[1024];
  NODE_PREP_IDs(n);
  sprintf(buffer, "\"cf:%s\" : {%s, \"cf:user_id\":%u, \"cf:group_id\":%u}",
    id,
    node_info_to_json(node_info, &n->identifier.node_id),
    n->uid,
    n->gid);
  return buffer;
}

#define UUID_STR_SIZE 37
char* uuid_to_str(uint8_t* uuid, char* str, size_t size){
  if(size<37){
    sprintf(str, "UUID-ERROR");
    return str;
  }
  sprintf(str, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
    uuid[0], uuid[1], uuid[2], uuid[3]
    , uuid[4], uuid[5]
    , uuid[6], uuid[7]
    , uuid[8], uuid[9]
    , uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
    return str;
}

static const char STR_UNKNOWN[]= "unknown";
static const char STR_BLOCK_SPECIAL[]= "block special";
static const char STR_CHAR_SPECIAL[]= "char special";
static const char STR_DIRECTORY[]= "directory";
static const char STR_FIFO[]= "fifo";
static const char STR_LINK[]= "link";
static const char STR_FILE[]= "file";
static const char STR_SOCKET[]= "socket";

static inline const char* get_inode_type(mode_t mode){
  if(S_ISBLK(mode))
    return STR_BLOCK_SPECIAL;
  else if(S_ISCHR(mode))
    return STR_CHAR_SPECIAL;
  else if(S_ISDIR(mode))
    return STR_DIRECTORY;
  else if(S_ISFIFO(mode))
    return STR_FIFO;
  else if(S_ISLNK(mode))
    return STR_LINK;
  else if(S_ISREG(mode))
    return STR_FILE;
  else if(S_ISSOCK(mode))
    return STR_SOCKET;
  return STR_UNKNOWN;
}

char* inode_to_json(struct inode_prov_struct* n){

  char msg_info[1024];
  char node_info[1024];
  char uuid[UUID_STR_SIZE];
  NODE_PREP_IDs(n)
  sprintf(buffer, "\"cf:%s\" : {%s, \"cf:user_id\":%u, \"cf:group_id\":%u, \"prov:type\":\"cf:%s\", \"cf:mode\":\"0X%04hhX\", \"cf:uuid\":\"%s\"}",
    id,
    node_info_to_json(node_info, &n->identifier.node_id),
    n->uid,
    n->gid,
    get_inode_type(n->mode),
    n->mode,
    uuid_to_str(n->sb_uuid, uuid, UUID_STR_SIZE));
  return buffer;
}

char* sb_to_json(struct sb_struct* n){
  char node_info[1024];
  char uuid[UUID_STR_SIZE];
  NODE_PREP_IDs(n)
  sprintf(buffer, "\"cf:%s\" : {%s, \"cf:uuid\":\"%s\"}",
    id,
    node_info_to_json(node_info, &n->identifier.node_id),
    uuid_to_str(n->uuid, uuid, UUID_STR_SIZE));
  return buffer;
}

char* msg_to_json(struct msg_msg_struct* n){
  char node_info[1024];
  NODE_PREP_IDs(n)
  sprintf(buffer, "\"cf:%s\" : {%s, \"cf:type\":%ld}",
    id,
    node_info_to_json(node_info, &n->identifier.node_id),
    n->type);
  return buffer;
}

char* shm_to_json(struct shm_struct* n){
  char node_info[1024];
  NODE_PREP_IDs(n)
  sprintf(buffer, "\"cf:%s\" : {%s, \"cf:mode\":\"0X%04hhX\"}",
    id,
    node_info_to_json(node_info, &n->identifier.node_id),
    n->mode);
  return buffer;
}

char* sock_to_json(struct sock_struct* n){
  char node_info[1024];
  NODE_PREP_IDs(n)
  sprintf(buffer, "\"cf:%s\" : {%s, \"cf:type\":%u, \"cf:family\":%u, \"cf:protocol\":%u}",
    id,
    node_info_to_json(node_info, &n->identifier.node_id),
    n->type,
    n->family,
    n->protocol);
  return buffer;
}

char* str_msg_to_json(struct str_struct* n){
  char edge_info[1024];
  NODE_PREP_IDs(n)
  sprintf(buffer, "\"cf:%s\" : {%s, \"cf:message\":\"%s\"}",
    id,
    edge_info_to_json(edge_info, &n->identifier.edge_id),
    n->str);
  return buffer;
}

char* sockaddr_to_json(char* buf, struct sockaddr* addr, size_t length){
  char host[NI_MAXHOST];
  char serv[NI_MAXSERV];

  if(addr->sa_family == AF_INET){
    getnameinfo(addr, length, host, NI_MAXHOST, serv, NI_MAXSERV, 0);
    sprintf(buf, "{\"type\":\"AF_INET\", \"host\":\"%s\", \"serv\":\"%s\"}", host, serv);
  }else if(addr->sa_family == AF_INET6){
    getnameinfo(addr, length, host, NI_MAXHOST, serv, NI_MAXSERV, 0);
    sprintf(buf, "{\"type\":\"AF_INET6\", \"host\":\"%s\", \"serv\":\"%s\"}", host, serv);
  }else if(addr->sa_family == AF_UNIX){
    sprintf(buf, "{\"type\":\"AF_UNIX\", \"path\":\"%s\"}", ((struct sockaddr_un*)addr)->sun_path);
  }else{
    sprintf(buf, "{\"type\":\"OTHER\"}");
  }

  return buf;
}

char* addr_to_json(struct address_struct* n){
  char node_info[1024];
  char addr_info[PATH_MAX+1024];
  NODE_PREP_IDs(n)
  sprintf(buffer, "\"cf:%s\" : {%s, \"cf:address\":%s}",
    id,
    node_info_to_json(node_info, &n->identifier.node_id),
    sockaddr_to_json(addr_info, &n->addr, n->length));
  return buffer;
}

char* pathname_to_json(struct file_name_struct* n){
  char node_info[1024];
  NODE_PREP_IDs(n)
  sprintf(buffer, "\"cf:%s\" : {%s, \"cf:pathname\":\"%s\"}",
    id,
    node_info_to_json(node_info, &n->identifier.node_id),
    n->name);
  return buffer;
}

char* ifc_to_json(struct ifc_context_struct* n){
  char node_info[1024];
  NODE_PREP_IDs(n)
  sprintf(buffer, "\"cf:%s\" : {%s, \"cf:ifc\":\"TODO\"}",
    id,
    node_info_to_json(node_info, &n->identifier.node_id));
  return buffer;
}

char* prefix_json(){
  return "\"prov\" : \"http://www.w3.org/ns/prov\", \"cf\":\"http://www.camflow.org\"";
}