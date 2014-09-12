//
//  ConsoleHandler.cpp
//  PthreadTest
//
//  Created by VincentChou on 14-8-21.
//  Copyright (c) 2014年 VincentChou. All rights reserved.
//

#include "ConsoleHandler.h"
#include "ODSocket.h"
#include "Utils.h"
#include "json.h"
#include <iostream>

extern pthread_mutex_t             s_mainMutex;
extern pthread_cond_t              s_mainCondition;

//console socket
static pthread_t            s_consoleRevThread;
static ODSocket             s_consoleSocket;

std::vector<fileinfo>       s_addFiles;
std::vector<std::string>    s_rmFiles;

void compareUpdateResources(fileinfoList& inapp)
{
    //compare diff
    fileinfoList local; traverseFolder("", local);
    //handle new local
    for (fileinfoList::iterator i = local.begin(); i != local.end(); i++)
    {
        //lookup inapp files
        fileinfoList::iterator find = inapp.find(i->first);
        if (find == inapp.end()) //can't find
        {
            //write to app.
            printf("add  %s\n", i->first.c_str());
            s_addFiles.push_back(fileinfo(i->first, i->second));
        }
        else if (i->second > find->second)
        {
            //write to app
            printf("modify  %s\n", i->first.c_str());
            s_addFiles.push_back(fileinfo(i->first, i->second));
        }
    }
    
    //handle rm files
    for (fileinfoList::iterator i = inapp.begin(); i != inapp.end(); i++)
    {
        fileinfoList::iterator find = local.find(i->first);
        if (find == local.end())
        {
            printf("remove %s\n", i->first.c_str());
            s_rmFiles.push_back(i->first);
        }
    }
    
    if (s_rmFiles.size() <= 0 && s_addFiles.size() <= 0)
    {
        printf("No files need reload.\n");
    }
}

void* revConsoleLoop(void * p)
{
    while (true)
    {
        if (s_consoleSocket.Select() != 0) continue;
        
        bool resumeMain = true;
        char check[2] = {'\0'};
        int ret = s_consoleSocket.Recv(check, 1, 0);
        if (ret <= 0) continue;
        
        if (check[0] == 0x01)
        {
            int last = 0;
            char lenbuf[64] = {'\0'};
            do
            {
                int ret = s_consoleSocket.Recv(lenbuf + last, 1, 0);
                if (ret <= 0) break;
                if (lenbuf[last] == ' ' || lenbuf[last] == '>' || lenbuf[last] == '\n'
                    || lenbuf[last] == '\0' || lenbuf[last] == 0x01)
                {
                    continue;
                }
                last++;
            } while (lenbuf[last - 1] != ':');
            
            std::string str(lenbuf, last - 1);
            int len = atoi(str.c_str());
            
            char* buf = new char[len + 1];
            memset(buf, '\0', len);
            int revlen = s_consoleSocket.Recv(buf, len, 0);
            if (revlen <= 0)
            {
                printf("ERROR: console socket rev error.");
                pthread_exit(NULL);
                return 0;
            }
            
            Json::Reader reader; Json::Value value;
            reader.parse(buf, value);
            if (value["cmd"].asString().compare("getfileinfo") == 0)
            {
                /*
                 cmd:
                 body:{fn:mdtime, fn1:mdtime1, ....}
                 */
                fileinfoList inapp;
                Json::Value& fbody = value["body"];
                Json::Value::Members mem = fbody.getMemberNames();
                for (int i = 0; i < mem.size(); i++)
                {
                    long mdtime = atol(fbody[mem.at(i)].asString().c_str()) / 1000;
                    inapp.insert(fileinfo(mem.at(i), mdtime));
                }
                compareUpdateResources(inapp);
            }
            else
            {
                printf("%s\n", buf);
            }
            
            delete [] buf;
        }
        else
        {
            /*
             如果只收到3个字符，为：
             `> `   62,32,0        62: >, 32:空格/, 0
             */
            const int plus = 256;
            int len = plus;
            char* buf = new char[len];
            memset(buf, '\0', len);
            buf[0] = check[0];
            int last = 1;
            do
            {
                int ret = s_consoleSocket.Recv(buf + last, 1, 0);
                if (ret <= 0) break;
                last++;
            } while (buf[last - 1] != '\n' && buf[last - 1] != 0);
            
//            printf("char value = \n");
//            for (int i = 0; i < last; i++)
//            {
//                printf("%c ", buf[i]);
//            }
//            printf("\n");
//            printf("int value = \n");
//            for (int i = 0; i < last; i++)
//            {
//                printf("%d ", buf[i]);
//            }
//            printf("\n");
            
            if (last == 3 && buf[0] == '>' && buf[1] == ' ' && buf[2] == 0)
                resumeMain = false;
            
            std::cout << buf;
            
            delete [] buf;
        }

        if (resumeMain)
        {
            printf("resume main thread.\n");
            pthread_cond_signal(&s_mainCondition);
        }
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


