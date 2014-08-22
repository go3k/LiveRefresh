//
//  main.cpp
//  PthreadTest
//
//  Created by VincentChou on 14-8-13.
//  Copyright (c) 2014年 VincentChou. All rights reserved.
//

#include <iostream>
#include <string>
#include "ConsoleHandler.h"
#include "FileServerHandler.h"

static bool                 s_mainloop = true;
pthread_mutex_t             s_mainMutex;
pthread_cond_t              s_mainCondition;

void traverseTest();

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
    
    traverseTest();
    return 0;
    
    const char* deviceip = "127.0.0.1";

    pthread_mutex_init(&s_mainMutex, NULL);
    pthread_cond_init(&s_mainCondition, NULL);
    
    //====console socket====
    int consolePort = 6050;
    bool suc = consoleSocketConnect(deviceip, consolePort);
    if (!suc) return 0;
    consoleRevThreadBegin();
    
    //====file server socket====
    int fsPort = 6060;
    suc = fsSocketConnect(deviceip, fsPort);
    if (!suc) return 0;
    fsRevThreadBegin();
    
    // insert code here...
    while (s_mainloop)
    {
        char cmdin[256], tmp[256];
        std::cin.getline(tmp, 256);
        sprintf(cmdin, "%s\n", tmp);
        
        //file server command
        bool fsCmd = true;
        if (fsCmd)
        {
            int ret = sendFile();
            if (ret <= 0)
            {
                printf("ERROR: FileServer file send fail.\n");
            }
        }
        else
        {
            int ret = sendConsoleCmd(cmdin);
            if (ret <= 0)
            {
                printf("ERROR: console command send fail.\n");
            }
        }
        
        pthread_cond_wait(&s_mainCondition, &s_mainMutex);
    }
    
    return 0;
}

