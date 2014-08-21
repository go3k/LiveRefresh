//
//  ConsoleHandler.h
//  PthreadTest
//
//  Created by VincentChou on 14-8-21.
//  Copyright (c) 2014年 VincentChou. All rights reserved.
//

#ifndef __PthreadTest__ConsoleHandler__
#define __PthreadTest__ConsoleHandler__

#include <iostream>

bool consoleSocketConnect(const char* ip, int port);

void consoleRevThreadBegin();

int sendConsoleCmd(const char* cmd);

#endif /* defined(__PthreadTest__ConsoleHandler__) */
