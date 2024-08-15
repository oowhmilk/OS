#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/syscalls.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>
#include <asm/uaccess.h>
#include <linux/slab.h>

#define BUFFER_SIZE 1024

SYSCALL_DEFINE2(print_reverse,char *,from_user,char *,to_user){
    char reversed_str[BUFFER_SIZE];
    char result_str[BUFFER_SIZE] = "";

        copy_from_user(reversed_str, from_user, BUFFER_SIZE);

    int length;
        length = strlen(reversed_str);

        int i = 0;
    for (i = length - 1; i >= 0; i--) {
        strncat(result_str, &reversed_str[i], 1);
    }

    copy_to_user(to_user, result_str, BUFFER_SIZE);

        return 0;
}

