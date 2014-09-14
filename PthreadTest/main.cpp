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

pthread_mutex_t             s_fsMutex;
pthread_cond_t              s_fsCondition;

extern std::vector<fileinfo>        s_addFiles;
extern std::vector<std::string>     s_rmFiles;

void sendJsonConsoleCMD(Json::Value& json)
{
    Json::FastWriter jswriter;
    std::string sd_cmd = jswriter.write(json);
    sd_cmd.insert(0, "sendrequest ");
    sendConsoleCmd(sd_cmd.c_str());
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

    pthread_mutex_init(&s_mainMutex, NULL);
    pthread_cond_init(&s_mainCondition, NULL);
    
    pthread_mutex_init(&s_fsMutex, NULL);
    pthread_cond_init(&s_fsCondition, NULL);
    
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
            
            sendJsonConsoleCMD(request_args);
            s_rmFiles.clear();
        }
        
        //insert file
        if (s_addFiles.size() > 0)
        {
            std::vector<std::string> jsfiles;
            for (std::vector<fileinfo>::iterator i = s_addFiles.begin();
                 i != s_addFiles.end();
                 i++)
            {
                unsigned long extpos = i->first.rfind(".js");
                if (extpos == i->first.length() - 3)
                {
                    jsfiles.push_back(i->first);
                }
                sendFile(i->first.c_str(), i->second);
            }
            
            //pause main thread
            pthread_cond_wait(&s_fsCondition, &s_fsMutex);
            if (s_addFiles.size() <= 0)
            {
                if (jsfiles.size() > 0)
                {
                    Json::Value reloads(Json::arrayValue);
                    for (std::vector<std::string>::iterator j = jsfiles.begin(); j != jsfiles.end(); j++)
                    {
                        int idx = (int)(j - jsfiles.begin());
                        reloads[idx] = j->c_str();
                    }
                    reloads.append("main.js");
                    //precomfile
                    Json::Value precom;
                    precom["cmd"] = "precompile"; precom["modulefiles"] = reloads;
                    sendJsonConsoleCMD(precom);
                    //reload
                    Json::Value reload;
                    reload["cmd"] = "reload"; reload["modulefiles"] = reloads;
                    sendJsonConsoleCMD(reload);
                    printf("reload\n");
                }
            }
            else
            {
                printf("ERROR: send file failed.\n");
                s_addFiles.clear();
            }
        }
        
        //comman process
        char cmdin[256], tmp[256];
        std::cin.getline(tmp, 256);
        sprintf(cmdin, "%s\n", tmp);
        
        //file server command
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
        
        pthread_cond_wait(&s_mainCondition, &s_mainMutex);
    }
    
    return 0;
}

