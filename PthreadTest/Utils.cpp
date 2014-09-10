//
//  Utils.cpp
//  PthreadTest
//
//  Created by VincentChou on 14-9-9.
//  Copyright (c) 2014年 VincentChou. All rights reserved.
//

#include "Utils.h"
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>

#define TRAVERSE_DEBUG 0

const char* projectFolder = "/Users/Vincent/Documents/cocos/CocosJSGame";

long getFileModifyTime(const char* file)
{
    struct stat attrib;
    if (stat(file, &attrib) == -1)
    {
        perror(file);
        exit(1);
    }
    return attrib.st_mtime;
}

void traverseFolder(const char* path, int deep, fileinfoList& list)
{
    std::string pullpath = projectFolder;
    pullpath.append(path);
#if TRAVERSE_DEBUG
    std::string outpath = pullpath;
    for (int i = 0; i < deep; i++) outpath.insert(0, "  ");
    printf("%s\n", outpath.c_str());
#endif
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (pullpath.c_str())) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            
            //ignore .xxx
            if (ent->d_name[0] == '.') continue;
            //ignore frameworks, runtime, tools, when deep == 0
            if (deep == 0)
            {
                if (strcmp(ent->d_name, "frameworks") == 0 ||
                    strcmp(ent->d_name, "runtime") == 0 ||
                    strcmp(ent->d_name, "tools") == 0)
                    continue;
            }
            
#if TRAVERSE_DEBUG
            std::string outpath = ent->d_name;
            for (int i = 0; i < deep + 1; i++) outpath.insert(0, "  ");
            printf ("%s\n", outpath.c_str());
#endif
            if (ent->d_type == DT_DIR)
            {
                std::string newpath = path;
                newpath.append("/");
                newpath.append(ent->d_name);
                traverseFolder(newpath.c_str(), deep + 1, list);
            }
            else if (ent->d_type == DT_REG)
            {
                //get modify date
                std::string fullfile = pullpath;
                fullfile.append("/");
                fullfile.append(ent->d_name);
                long mdtime = getFileModifyTime(fullfile.c_str());
                
                //record related file path and modify date
                std::string relatepath = path;
                relatepath.append("/");
                relatepath.append(ent->d_name);
                if (relatepath.size() > 0 && relatepath.at(0) == '/')
                    relatepath.erase(relatepath.begin());
                list.insert(fileinfo(relatepath, mdtime));
            }
        }
        closedir (dir);
    } else {
        /* could not open directory */
        perror ("");
        //        return EXIT_FAILURE;
    }
}

void traverseFolder(const char* path, fileinfoList& list)
{
    traverseFolder(path, 0, list);
}

void traverseTest()
{
    fileinfoList list;
    traverseFolder("", list);
    
    for (fileinfoList::iterator i = list.begin(); i != list.end(); i++)
    {
        printf("%s   %ld\n", i->first.c_str(), i->second);
    }
}

