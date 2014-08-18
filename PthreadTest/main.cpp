//
//  main.cpp
//  PthreadTest
//
//  Created by VincentChou on 14-8-13.
//  Copyright (c) 2014年 VincentChou. All rights reserved.
//

#include <iostream>
#include <string>
#include "pthread.h"
#include "ODSocket.h"

static bool                 s_mainloop = true;
static pthread_mutex_t		s_mainMutex;
static pthread_cond_t		s_mainCondition;
//
static ODSocket             s_consoleSocket;

void* threadPrint(void * p)
{
    int* param = (int*)p;
    
    int i = 0;
    while (true) {
        printf("thread print %d.\n", i);
        i++;
        if (i > 10)
        {
            pthread_exit(NULL);
        }
    }
    
    return 0;
}

/*
 1. waiting for use command
 2. connect to fileserver and console
 3. two kind of command:
    * send to console
    * send to fileserver rewrite js files.
 */
int main(int argc, const char * argv[])
{
    printf("argc = %d \n", argc);
    
    for (int i = 0; i < argc; i++)
    {
        const char * arg = argv[i];
        printf("arg = %s\n", arg);
    }

    pthread_mutex_init(&s_mainMutex, NULL);
    pthread_cond_init(&s_mainCondition, NULL);
    
    //====console socket====
    //socket connect
    s_consoleSocket.Create(AF_INET, SOCK_STREAM);
    bool suc = s_consoleSocket.Connect("127.0.0.1", 6050);
    printf("socket connect result = %d\n", suc);
    
    if (suc)
    {
        const char* cmd = "fps off\n";
        int ret = s_consoleSocket.Send(cmd, (int)strlen(cmd));
        
        printf("socket send result = %d\n" ,ret);
        
        sleep(2);
        
        cmd = "fps on\n";
        ret = s_consoleSocket.Send(cmd, (int)strlen(cmd));
        
        printf("socket send result = %d\n" ,ret);
    }
    
    //receive console response thread
    
    
    //while loop that wait response from 6050
    while (true) {
        if (s_consoleSocket.Select() != 0) continue;
        
        printf("Socket receive data.\n");
        
        //        int retlen = 0;
    }
    
    // insert code here...
    printf("Hello, World!\n");
    while (s_mainloop)
    {
        char name[256];
        std::cin.getline(name, 256);
        printf("getline = %s\n", name);
        
        pthread_cond_wait(&s_mainCondition, &s_mainMutex);
        
        printf("main loop end.\n");
    }
    
    return 0;
    //===============
    
    printf("main end.\n");
    
    return 0;
}

