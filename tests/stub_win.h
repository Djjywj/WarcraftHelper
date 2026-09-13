// Stub Win32 layer for the offline harness (Linux g++).
#pragma once

#include <cstddef>
#include <csetjmp>

#ifndef _WIN32
#define __stdcall
#endif

typedef unsigned long DWORD;
typedef int BOOL;
typedef void* HANDLE;
typedef void* LPVOID;
typedef void* HWND;
typedef unsigned long long SIZE_T;

#ifndef NULL
#define NULL 0
#endif

#define VK_LBUTTON 0x01

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

struct RECT { long left, top, right, bottom; };

extern jmp_buf g_out;
extern int g_iteration;
extern int g_maxIter;

extern HWND g_game;
extern HWND g_foreground;
extern int g_visible;
extern int g_iconic;
extern int g_isWindow;
extern int g_lbuttonDown;

extern RECT g_rect;
extern DWORD g_tick;

extern int g_moveCalls;
extern RECT g_moveRects[64];
extern int g_moveFlags[64];
extern int g_getGameWindowCalls;

typedef void (*WorldFn)(int iteration);
extern WorldFn g_applyWorld;

HWND GetForegroundWindow();
BOOL IsWindow(HWND hwnd);
BOOL IsWindowVisible(HWND hwnd);
BOOL IsIconic(HWND hwnd);
BOOL GetWindowRect(HWND hwnd, RECT* rect);
BOOL MoveWindow(HWND hwnd, int x, int y, int w, int h, BOOL repaint);
DWORD GetTickCount();
short GetAsyncKeyState(int key);
HANDLE CreateThread(LPVOID a, SIZE_T b, DWORD(__stdcall* fn)(LPVOID), LPVOID c, DWORD d, DWORD* e);
BOOL TerminateThread(HANDLE h, DWORD code);
void ExitThread(DWORD code);
void Sleep(DWORD ms);
