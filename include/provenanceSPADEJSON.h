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
#ifndef __PROVENANCESPADEJSON_H
#define __PROVENANCESPADEJSON_H

/* struct to spade functions */
/* TODO provide clean implementation? right now probably highly inneficient */
char* relation_to_spade(struct relation_struct* e);
char* used_to_spade(struct relation_struct* e);
char* generated_to_spade(struct relation_struct* e);
char* informed_to_spade(struct relation_struct* e);
char* derived_to_spade(struct relation_struct* e);
char* disc_to_spade(struct disc_node_struct* n);
char* proc_to_spade(struct proc_prov_struct* n);
char* task_to_spade(struct task_prov_struct* n);
char* inode_to_spade(struct inode_prov_struct* n);
char* sb_to_spade(struct sb_struct* n);
char* msg_to_spade(struct msg_msg_struct* n);
char* shm_to_spade(struct shm_struct* n);
char* packet_to_spade(struct pck_struct* n);
char* str_msg_to_spade(struct str_struct* n);
char* addr_to_spade(struct address_struct* n);
char* pathname_to_spade(struct file_name_struct* n);
const char* prefix_spade();
char* machine_description_spade(char* buffer);
char* iattr_to_spade(struct iattr_prov_struct* n);
char* xattr_to_spade(struct xattr_prov_struct* n);
char* pckcnt_to_spade(struct pckcnt_struct* n);
char* arg_to_spade(struct arg_struct* n);

#endif
