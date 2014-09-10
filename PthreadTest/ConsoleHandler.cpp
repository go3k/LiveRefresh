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

extern pthread_mutex_t             s_mainMutex;
extern pthread_cond_t              s_mainCondition;

//console socket
static pthread_t            s_consoleRevThread;
static ODSocket             s_consoleSocket;

std::vector<fileinfo>       s_addFiles;
std::vector<std::string>    s_rmFiles;

static bool                 s_isSendrequestCMD = false;

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
}

void* revConsoleLoop(void * p)
{
    while (true)
    {
        if (s_consoleSocket.Select() != 0) continue;
        
        if (s_isSendrequestCMD)
        {
            int last = 0;
            char lenbuf[64] = {'\0'};
            do
            {
                s_consoleSocket.Recv(lenbuf + last, 1, 0);
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
            const int len = 256;
            char* buf = new char[len];
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
                printf("%s\n", buf + 1);
            else
                printf("%s", buf);
            
            delete [] buf;
        }
        
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

const char* kSendRequestCMD = "sendrequest";

int sendConsoleCmd(const char* cmd)
{
    s_isSendrequestCMD = false;
    if (strlen(cmd) >= strlen(kSendRequestCMD)) {
        
        /* 
         if current is sendrequest cmd.
         sendrequest must have correct args or app will crash, sendrequest can only send by
         program, so we can ensure its args.
        */
        std::string str(cmd, strlen(kSendRequestCMD));
        if (str.compare(kSendRequestCMD) == 0)
        {
//            printf("Current cmd is sendrequest.\n");
            s_isSendrequestCMD = true;
        }
    }
    return s_consoleSocket.Send(cmd, (int)strlen(cmd));
}


