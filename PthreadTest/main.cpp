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

//console socket
static pthread_t            s_consoleRevThread;
static ODSocket             s_consoleSocket;

void* revConsoleLoop(void * p)
{
    while (true)
    {
        if (s_consoleSocket.Select() != 0) continue;
        
        const int len = 256;
        char* buf = new char[256];
        memset(buf, '\0', len);
        int revlen = s_consoleSocket.Recv(buf, len, 0);
        
        /* 
         如果只收到3个字符，为：
         `> `   62,32,0        62: >, 32:空格/, 0
         */
        
        if (revlen <= 0)
        {
            printf("ERROR: console socket rev error.");
            continue;
        }
        if (buf[0] == 0x01)
        {
            printf("%s\n", buf + 1);
        }
        else
            printf("%s", buf);
        
        delete [] buf;
        
        pthread_cond_signal(&s_mainCondition);
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
    
    const char* deviceip = "127.0.0.1";
    int consolePort = 6050;

    pthread_mutex_init(&s_mainMutex, NULL);
    pthread_cond_init(&s_mainCondition, NULL);
    
    //====console socket====
    //console socket connect
    s_consoleSocket.Create(AF_INET, SOCK_STREAM);
    bool suc = s_consoleSocket.Connect(deviceip, consolePort);
    
    if (!suc)
    {
        printf("ERROR: Can't connect %s:%d\n", deviceip, consolePort);
        return 0;
    }
    
    //receive console response thread
    pthread_create(&s_consoleRevThread, NULL, revConsoleLoop, NULL);
    
    // insert code here...
    while (s_mainloop)
    {
        char cmdin[256], tmp[256];
        std::cin.getline(tmp, 256);
        sprintf(cmdin, "%s\n", tmp);
        int ret = s_consoleSocket.Send(cmdin, (int)strlen(cmdin));
        if (ret <= 0)
        {
            printf("ERROR: console command send fail.\n");
        }
        
        pthread_cond_wait(&s_mainCondition, &s_mainMutex);
    }
    
    return 0;
}

