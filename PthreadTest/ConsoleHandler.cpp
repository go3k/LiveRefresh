//
//  ConsoleHandler.cpp
//  PthreadTest
//
//  Created by VincentChou on 14-8-21.
//  Copyright (c) 2014年 VincentChou. All rights reserved.
//

#include "ConsoleHandler.h"
#include "ODSocket.h"

extern pthread_mutex_t             s_mainMutex;
extern pthread_cond_t              s_mainCondition;

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
            pthread_exit(NULL);
            return 0;
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

bool consoleSocketConnect(const char* ip, int port)
{
    s_consoleSocket.Create(AF_INET, SOCK_STREAM);
    bool suc = s_consoleSocket.Connect(ip, port);
    if (!suc)
    {
        printf("ERROR: Can't connect %s:%d\n", ip, port);
        return false;
    }
    return true;
}

void consoleRevThreadBegin()
{
    //receive console response thread
    pthread_create(&s_consoleRevThread, NULL, revConsoleLoop, NULL);
}

int sendConsoleCmd(const char* cmd)
{
    return s_consoleSocket.Send(cmd, (int)strlen(cmd));
}


