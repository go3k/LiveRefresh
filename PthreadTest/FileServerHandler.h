//
//  FileServerHandler.h
//  PthreadTest
//
//  Created by VincentChou on 14-8-21.
//  Copyright (c) 2014年 VincentChou. All rights reserved.
//

#ifndef __PthreadTest__FileServerHandler__
#define __PthreadTest__FileServerHandler__

#include <iostream>

bool fsSocketConnect(const char* ip, int port);

void fsRevThreadBegin();

int sendFile(const char* file, long time);

#endif /* defined(__PthreadTest__FileServerHandler__) */
