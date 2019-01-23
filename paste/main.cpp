//
//  main.cpp
//  paste
//
//  Created by zzl on 2019/1/19.
//  Copyright © 2019年 zzl. All rights reserved.
//

#include "clip.h"
#include <iostream>
#include <cstdio>
#include <syslog.h>
#include <dlfcn.h>
#include <stdarg.h>
#include "mach_override.h"
#include "mach-o/dyld.h"
#include <CoreServices/CoreServices.h>
//#include <Foundation/Foundation.h>


int debug(const char *format)
{
    FILE *file = fopen("/tmp/log.txt", "a+");
    if(file)
    {
        fprintf(file, format);
        fclose(file);
    }
    return 0;
}


// Used to track the pointer to victim code's function
long *victim_func_ptr;
long *victim_func_ptr_setdata;
long *victim_func_ptr_copydata;
long *victim_func_ptr_open;

// A function prototype so we can call the victim function from our override
// function.
//int (*victim_func_setdata)(int, int, int, int, int, int) = 0;
int32_t (*victim_func_copydata)(int, int, int, int, int) = 0;
// Our override function
int (*victim_func_open)(const char*, int) = 0;
/*int my_CFPasteboardSetdata(int argv0, int argv1, int argv2, int argv3, int argv4, int argv5)
{
    printf("in my_CFPasteboardSetdata\n");
    return (*victim_func_setdata)(argv0, argv1, argv2, argv3, argv4, argv5);
}*/
int32_t my_CFPasteboardCopydata(int argv0, int argv1, int argv2, int argv3, int argv4)
{
    printf("in my_CFPasteboardCopydata %llx\n", victim_func_copydata);
    printf("%d %x %x %x %x %x %x\n", argv0, argv1, argv2, argv3, argv4);
    int i =  (*victim_func_copydata)(argv0, argv1, argv2, argv3, argv4);
    return i;
}

int my_open(const char* path, int flag)
{
    printf("in my_open\n");
    return (*victim_func_open)(path, flag);
}

void install()
{
    printf("testlib: install\n");
  
    mach_error_t me;
    
    _dyld_lookup_and_bind(
                          "_CFPasteboardCopyData",
                          (void**) &victim_func_ptr_copydata,
                          NULL);
    
    //victim_func_ptr_copydata == dlsym(RTLD_DEFAULT, "_CFPasteboardCopyData");
    
    //TODO check for bad victim_func_ptr
     // Assign our long pointer to our function prototype
    victim_func_copydata = (int32_t (*)(int, int, int, int, int ))victim_func_ptr_copydata;
    
    // Do the override
    
    me = mach_override_ptr(
                           victim_func_ptr_copydata,
                           (void*)&my_CFPasteboardCopydata,
                           (void**)&victim_func_copydata);
    printf("testlib: victim_func_ptr_copydata   = %llx\n",(long)victim_func_ptr_copydata);
    printf("testlib: victim_func_copydata       = %llx\n", (long)victim_func_copydata);
    printf("testlib: my_cfpasteboardcopydata    = %llx\n", my_CFPasteboardCopydata);
    //victim_func_copydata = (int (*)(int, int, int, int, int))victim_func_ptr_copydata;
    printf("hook ret %d\n", me);
}

void install_open()
{
    printf("testlib: install\n");
    
    mach_error_t me;
    
    _dyld_lookup_and_bind(
                          "_open",
                          (void**) &victim_func_ptr_open,
                          NULL);
    
    //TODO check for bad victim_func_ptr
    printf("testlib: victim_func_ptr_open   = %llx\n", (long)victim_func_ptr_open);
    printf("testlib: victim_func_open       = %llx\n", (long)victim_func_open);
    // Assign our long pointer to our function prototype
    victim_func_open = (int (*)(const char*, int))victim_func_ptr_open;
    
    printf("testlib: victim_func_ptr_open   = %llx\n", (long)victim_func_ptr_open);
    printf("testlib: victim_func_open       = %llx\n", (long)victim_func_open);
    
    // Do the override
    
    me = mach_override_ptr(
                           victim_func_ptr_open,
                           (void*)&my_open,
                           (void**)&victim_func_open);
    printf("testlib: victim_func_ptr_open   = %llx\n", (long)victim_func_ptr_open);
    printf("testlib: victim_func_open       = %llx\n", (long)victim_func_open);
    printf("testlib: victim_func_open       = %llx\n", my_open);
    printf("hook ret %d\n", me);
}



int main() {
#if 0
    install_open();
    int fd = open("/Users/zzl/2.txt", O_RDONLY);
    printf("fd %d\n", fd);
    if(-1 != fd)
    {
        close(fd);
    }
#else
    install();
    if (clip::has(clip::text_format())) {
        std::string value;
        clip::get_text(value);
        
        std::cout << "Clipboard content is '" << value << "'\n";
    }
    else {
        std::cout << "Clipboard doesn't contain text\n";
    }
#endif
}
