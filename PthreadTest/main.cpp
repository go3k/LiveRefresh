//
//  main.cpp
//  PthreadTest
//
//  Created by VincentChou on 14-8-13.
//  Copyright (c) 2014年 VincentChou. All rights reserved.
//

#include <iostream>
#include <string>
#include <vector>
#include "ConsoleHandler.h"
#include "FileServerHandler.h"
#include "json.h"
#include "Utils.h"

static bool                 s_mainloop = true;
pthread_mutex_t             s_mainMutex;
pthread_cond_t              s_mainCondition;

extern std::vector<fileinfo>        s_addFiles;
extern std::vector<std::string>     s_rmFiles;

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
    
#warning "需要检查代码，运行后CPU狂转。"
    
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
        /*
         Handle file sync.
         */
        //insert file
        for (std::vector<fileinfo>::iterator i = s_addFiles.begin();
             i != s_addFiles.end();
             i++)
        {
            sendFile(i->first.c_str(), i->second);
        }
        s_addFiles.clear();
        
        //rm file
        if (s_rmFiles.size() > 0)
        {
            Json::Value request_args;
            request_args["cmd"] = "remove";
            Json::Value rmfiles(Json::arrayValue);
            for (std::vector<std::string>::iterator i = s_rmFiles.begin();
                 i != s_rmFiles.end();
                 i++)
            {
                int idx = (int)(i - s_rmFiles.begin());
                rmfiles[idx] = i->c_str();
            }
            request_args["files"] = rmfiles;
//            printf("%s\n", request_args.toStyledString().c_str());
            
            Json::FastWriter jswriter;
            std::string rm_cmd = jswriter.write(request_args);
            rm_cmd.insert(0, "sendrequest ");
            rm_cmd.append("\n");
//            printf("%s", rm_cmd.c_str());
            sendConsoleCmd(rm_cmd.c_str());
            s_rmFiles.clear();
        }
        
        //comman process
        char cmdin[256], tmp[256];
        std::cin.getline(tmp, 256);
        sprintf(cmdin, "%s\n", tmp);
        
        //file server command
        bool fsCmd = false;
        if (fsCmd)
        {
            int ret = sendFile("res/CloseNormal.png", 1410360830);
            if (ret <= 0)
            {
                printf("ERROR: FileServer file send fail.\n");
            }
        }
        else
        {
            int ret = 0;
            if (strcmp(tmp, "refresh") == 0)
            {
                const char* fileinfo_cmd = "sendrequest {\"cmd\":\"getfileinfo\"}\n";
                ret = sendConsoleCmd(fileinfo_cmd);
            }
            else
            {
                ret = sendConsoleCmd(cmdin);
            }
            if (ret <= 0)
            {
                printf("ERROR: console command send fail.\n");
            }
        }
        
        pthread_cond_wait(&s_mainCondition, &s_mainMutex);
    }
    
    return 0;
}

