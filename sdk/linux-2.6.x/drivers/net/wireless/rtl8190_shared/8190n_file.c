/*
 *  Routines for file-open
 *
 *  $Id: 8190n_file.c,v 1.1.1.1 2010/05/05 09:00:39 jiunming Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8190N_FILE_C_

#ifdef __KERNEL__
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>

#endif


//Joshua
//sys_open can not be called by module, use this function instead.
/*
int rtl8190n_fileopen(const char *filename, int flags, int mode){
        char *tmp = getname(filename);
        int fd = get_unused_fd();
        struct file *f = filp_open(tmp, flags, mode);
        fd_install(fd, f);
	printk("fd install \n");
        putname(tmp);
        return fd;

}
*/


//chris
// sys_open/sys_read is not allowed, rewrite the functions
struct file* file_open(const char* path, int flags, int rights) {
    struct file* filp = NULL;
    mm_segment_t oldfs;
    int err = 0;

    oldfs = get_fs();
    set_fs(get_ds());
    filp = filp_open(path, flags, rights);
    set_fs(oldfs);
    if(IS_ERR(filp)) {
        err = PTR_ERR(filp);
        return NULL;
    }
    return filp;
}

void file_close(struct file* file) {
    filp_close(file, NULL);
}


int file_read(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size) {
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_read(file, data, size, &offset);

    set_fs(oldfs);
    return ret;
}


//EXPORT_SYMBOL(my_open);

