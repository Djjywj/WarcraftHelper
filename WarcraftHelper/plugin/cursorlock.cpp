// NOTE: this file is intentionally ASCII-only.
// MSVC compiles sources without a UTF-8 BOM using the system ANSI codepage,
// so non-ASCII comments can swallow the next line on Chinese Windows.
#include "cursorlock.hpp"
#include "config/config.hpp"

// Only release a clip region this plugin installed, so we never clobber
// a lock owned by the game or the OS. Both flags are written only by the
// worker thread and read by Stop(), which runs after the thread is done.
static bool c_locked = false;
static bool c_closed = false;

static bool IsOurProcessWindow(HWND wnd) {
    DWORD pid = 0;
    GetWindowThreadProcessId(wnd, &pid);
    return pid == GetCurrentProcessId();
}

// Prefer the window cached by the framework; fall back to the foreground
// window of this process when it goes stale. Game::GetGameWindow only
// resolves once, so it stays dead after the game recreates its window.
static HWND ResolveGameWindow() {
    HWND game = GetGameInstance()->GetGameWindow();
    if (game != NULL && IsWindow(game)) {
        return game;
    }

    HWND foreground = GetForegroundWindow();
    if (foreground != NULL && IsWindow(foreground) && IsOurProcessWindow(foreground)) {
        return foreground;
    }
    return NULL;
}

// Clip the cursor to the client area, which excludes title bar and borders.
static bool ClipToGameWindow(HWND game) {
    RECT client;
    POINT origin = {0, 0};
    RECT clip;

    if (!GetClientRect(game, &client) || !ClientToScreen(game, &origin)) {
        return false;
    }

    clip.left = origin.x;
    clip.top = origin.y;
    clip.right = origin.x + client.right;
    clip.bottom = origin.y + client.bottom;

    if (clip.right <= clip.left || clip.bottom <= clip.top) {
        return false;
    }

    return ClipCursor(&clip) != 0;
}

DWORD __stdcall CursorLockThread(LPVOID lpThreadParameter) {
    while (1) {
        if (c_closed) {
            ExitThread(0);
        }

        HWND game = ResolveGameWindow();
        // Never lock while a dialog is up or another app is focused,
        // otherwise the cursor gets trapped inside a background window.
        bool should_lock = GetConfig()->m_cursorLock
                && game != NULL
                && IsWindowVisible(game)
                && !IsIconic(game)
                && GetForegroundWindow() == game
                && ClipToGameWindow(game);

        if (!should_lock && c_locked) {
            ClipCursor(NULL);
        }
        c_locked = should_lock;

        Sleep(50);
    }
    return 0;
}

void CursorLock::Start() {
    // The thread always runs; the config flag decides whether it locks,
    // so the feature can be toggled while the game is running.
    this->thread = CreateThread(NULL, NULL, CursorLockThread, NULL, NULL, NULL);
}

void CursorLock::Stop() {
    c_closed = true;
    ClipCursor(NULL);
    c_locked = false;
    TerminateThread(this->thread, 0);
}
