// Offline harness: compiles the real autowindowrefresh.cpp against a stub
// Win32 layer and asserts the refresh state machine (timing, focus gating,
// move/move-back, config changes).
#include "stub_win.h"
#include "autowindowrefresh.hpp"
#include "config/config.hpp"
#include <cstdio>
#include <cstring>

jmp_buf g_out;
int g_iteration = 0;
int g_maxIter = 0;

HWND g_game = (HWND)0x1234;
HWND g_foreground = (HWND)0x1234;
int g_visible = 1;
int g_iconic = 0;
int g_isWindow = 1;
int g_lbuttonDown = 0;

RECT g_rect = {100, 50, 900, 650};
DWORD g_tick = 0;

int g_moveCalls = 0;
RECT g_moveRects[64];
int g_moveFlags[64];
int g_moveOk[64];
int g_moveFailAt = 0;
int g_moveSilentFailAt = 0;
int g_getGameWindowCalls = 0;

WorldFn g_applyWorld = 0;

HWND GetForegroundWindow() { return g_foreground; }
BOOL IsWindow(HWND h) { return (h == g_game) ? g_isWindow : 1; }
BOOL IsWindowVisible(HWND) { return g_visible; }
BOOL IsIconic(HWND) { return g_iconic; }
BOOL GetWindowRect(HWND, RECT* rect) { *rect = g_rect; return 1; }
BOOL MoveWindow(HWND, int x, int y, int w, int h, BOOL repaint) {
    int idx = g_moveCalls;
    if (idx >= 64) { return 0; }
    g_moveRects[idx] = RECT{x, y, x + w, y + h};
    g_moveFlags[idx] = repaint;
    g_moveCalls++;
    if (g_moveFailAt == idx + 1) {
        g_moveOk[idx] = 0;
        return 0;
    }
    g_moveOk[idx] = 1;
    if (g_moveSilentFailAt == idx + 1) {
        // Reports success but leaves the geometry alone.
        return 1;
    }
    g_rect = RECT{x, y, x + w, y + h};
    return 1;
}
DWORD GetTickCount() { return g_tick; }
short GetAsyncKeyState(int key) {
    if (key == VK_LBUTTON) { return g_lbuttonDown ? (short)0x8000 : 0; }
    return 0;
}
HANDLE CreateThread(LPVOID, SIZE_T, DWORD(__stdcall* fn)(LPVOID), LPVOID, DWORD, DWORD*) {
    fn(0);
    return (HANDLE)1;
}
BOOL TerminateThread(HANDLE, DWORD) { return 1; }
void ExitThread(DWORD) { longjmp(g_out, 1); }
void Sleep(DWORD ms) {
    g_tick += ms;
    g_iteration++;
    if (g_applyWorld) { g_applyWorld(g_iteration); }
    if (g_iteration >= g_maxIter) { longjmp(g_out, 1); }
}

static int g_failures = 0;
static void check(int cond, const char* name) {
    printf("[%s] %s\n", cond ? "PASS" : "FAIL", name);
    if (!cond) { g_failures++; }
}

static void reset() {
    g_iteration = 0;
    g_tick = 0;
    g_moveCalls = 0;
    g_moveFailAt = 0;
    g_moveSilentFailAt = 0;
    g_getGameWindowCalls = 0;
    memset(g_moveRects, 0, sizeof(g_moveRects));
    memset(g_moveFlags, 0, sizeof(g_moveFlags));
    memset(g_moveOk, 0, sizeof(g_moveOk));
    g_applyWorld = 0;
    g_game = (HWND)0x1234;
    g_foreground = (HWND)0x1234;
    g_visible = 1;
    g_iconic = 0;
    g_isWindow = 1;
    g_lbuttonDown = 0;
    g_rect = RECT{100, 50, 900, 650};
    config.m_autoRefreshInterval = 15;
}

static void run(int iters) {
    g_maxIter = iters;
    AutoWindowRefresh plugin;
    if (setjmp(g_out) == 0) {
        plugin.Start();
    }
}

int main() {
    // 间隔没到：一动不动
    reset();
    config.m_autoRefreshInterval = 15;
    run(20);  // 20 * 50ms = 1000ms
    check(g_moveCalls == 0, "before interval elapses -> window untouched");

    // 到点：宽 +1 再还原，正好两次移动，且都不是强制重绘
    reset();
    config.m_autoRefreshInterval = 1;
    run(30);  // 1000ms 触发一次，1450ms 前不会触发第二次
    check(g_moveCalls == 2, "interval reached -> exactly one +1 / restore pair");
    check(g_moveCalls == 2 && g_moveRects[0].right == 901,
          "first move grows width by exactly 1px");
    check(g_moveCalls == 2 && g_moveRects[1].right == 900,
          "second move restores original width");
    check(g_rect.right - g_rect.left == 800 && g_rect.left == 100 && g_rect.top == 50,
          "window geometry restored (position + size)");
    check(g_moveCalls == 2 && g_moveFlags[0] == 0 && g_moveFlags[1] == 0,
          "moves use repaint=FALSE to avoid flicker");

    // 之后按间隔重复：45 轮 -> 1000ms 和 2000ms 各一次 = 4 次移动
    reset();
    config.m_autoRefreshInterval = 1;
    run(45);
    check(g_moveCalls == 4, "refresh repeats every interval (2 refreshes in 2.25s)");

    // 游戏不在前台：不抖
    reset();
    config.m_autoRefreshInterval = 1;
    g_foreground = (HWND)0x9999;
    run(30);
    check(g_moveCalls == 0, "other window focused -> no refresh");

    // 最小化：不抖
    reset();
    config.m_autoRefreshInterval = 1;
    g_iconic = 1;
    run(30);
    check(g_moveCalls == 0, "minimized -> no refresh");

    // 不可见：不抖
    reset();
    config.m_autoRefreshInterval = 1;
    g_visible = 0;
    run(30);
    check(g_moveCalls == 0, "invisible window -> no refresh");

    // 按住鼠标左键：不抖
    reset();
    config.m_autoRefreshInterval = 1;
    g_lbuttonDown = 1;
    run(30);
    check(g_moveCalls == 0, "left mouse button held -> no refresh");

    // 正掐着时间按下左键：那一拍跳过，松手后补上
    reset();
    config.m_autoRefreshInterval = 1;
    g_applyWorld = [](int i) { g_lbuttonDown = (i < 25) ? 1 : 0; };
    run(40);
    check(g_moveCalls > 0, "click during due time -> refresh happens after release");

    // 间隔为 0：关闭
    reset();
    config.m_autoRefreshInterval = 0;
    run(60);
    check(g_moveCalls == 0, "interval 0 -> feature disabled");

    // 运行中改小间隔：立刻生效
    reset();
    config.m_autoRefreshInterval = 0;
    g_applyWorld = [](int i) { if (i >= 10) { config.m_autoRefreshInterval = 1; } };
    run(40);
    check(g_moveCalls > 0, "interval turned on at runtime -> starts refreshing");

    // 运行中改成 0：立刻停
    reset();
    config.m_autoRefreshInterval = 1;
    g_applyWorld = [](int i) { if (i >= 21) { config.m_autoRefreshInterval = 0; } };
    run(60);
    check(g_moveCalls == 2, "interval disabled at runtime -> stops after current one");

    // 超大间隔：不溢出、不抖、不崩
    reset();
    config.m_autoRefreshInterval = 999999999;
    run(10);
    check(g_moveCalls == 0, "huge interval -> no overflow, no refresh");

    // 窗口尺寸非法：不调用 MoveWindow
    reset();
    config.m_autoRefreshInterval = 1;
    g_rect = RECT{0, 0, 0, 0};
    run(30);
    check(g_moveCalls == 0, "zero-sized window -> no refresh");

    // 句柄失效：不应崩溃，且每轮都重新去认窗口（句柄会变）
    reset();
    config.m_autoRefreshInterval = 1;
    g_isWindow = 0;
    run(5);
    check(g_moveCalls == 0, "stale handle -> no refresh, no crash");
    check(g_getGameWindowCalls == g_iteration,
          "game window handle is re-resolved every tick");

    // 句柄先失效后恢复：恢复后正常抖
    reset();
    config.m_autoRefreshInterval = 1;
    g_isWindow = 0;
    g_applyWorld = [](int i) { if (i >= 15) { g_isWindow = 1; } };
    run(45);
    check(g_moveCalls > 0, "handle valid again -> refreshes resume");

    // 还原失败一次：要重试，最终窗口没有变宽
    reset();
    config.m_autoRefreshInterval = 1;
    g_moveFailAt = 2;
    run(30);
    check(g_moveCalls == 3, "restore failed once -> retried");
    check(g_rect.right - g_rect.left == 800, "window width restored after retry");

    // 还原"假成功"：MoveWindow 返回成功但尺寸没变，必须靠复查几何重试
    reset();
    config.m_autoRefreshInterval = 1;
    g_moveSilentFailAt = 2;
    run(30);
    check(g_moveCalls == 3, "restore silently ignored -> retried after verifying geometry");
    check(g_rect.right - g_rect.left == 800, "window width restored after silent failure");

    printf("\n%s (%d failures)\n", g_failures ? "SOME TESTS FAILED" : "ALL TESTS PASSED", g_failures);
    return g_failures ? 1 : 0;
}
