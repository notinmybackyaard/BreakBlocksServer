#pragma once
#pragma comment(lib, "ws2_32")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <tchar.h>
#include <string>
#include <ctime>
#include <vector>
#include <winsock2.h>
#include <stdlib.h>
#include <iostream>
#include <Commctrl.h>
#include <string>
#include <list>
#include <fstream>

#pragma comment (lib, "winmm.lib")

using namespace std;

#include "Enum.h"
#include "Struct.h"
#include "Macro.h"


const int WINCX = 500;
const int WINCY = 500;

extern SOCKET sock;

class Object;

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000
#define MAX_OBJECTS_COUNT 200
#define WINDOWSIZEX 500
#define WINDOWSIZEY 500
#define PLAYERSPEED 200
#define BULLETSPEED 800
#define BLOCKSPEED -300
#define BLOCKPERLINE 9
#define BLOCKTYPENUM 5
