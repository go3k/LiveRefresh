//
//  Utils.h
//  PthreadTest
//
//  Created by VincentChou on 14-9-9.
//  Copyright (c) 2014年 VincentChou. All rights reserved.
//

#ifndef __PthreadTest__Utils__
#define __PthreadTest__Utils__

#include <iostream>
//#include <vector>
#include <map>

#define LRLOG_ENABLE 0
#if LRLOG_ENABLE
#define LRLog(format,...) printf(format,##__VA_ARGS__)
#else
#define LRLog(format,...)
#endif

extern const char* projectFolder;

typedef std::pair<std::string, long> fileinfo;
typedef std::map<std::string, long> fileinfoList;

void traverseFolder(const char* path, fileinfoList& list);
void traverseTest();

#endif /* defined(__PthreadTest__Utils__) */
