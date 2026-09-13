// NOTE: this file is intentionally ASCII-only.
// MSVC compiles sources without a UTF-8 BOM using the system ANSI codepage,
// so non-ASCII comments can swallow the next line on Chinese Windows.
#include "autowindowrefresh.hpp"
#include "config/config.hpp"

// Windowed Warcraft III occasionally paints text on top of the previous frame
// (overlapping glyphs). Resizing the window forces the game to redraw and the
// artifact disappears, so this plugin does exactly that on a timer: width +1
// and straight back. F7 (windowfixer) stays as the manual trigger.
static bool a_closed = false;

// Upper bound for the configured interval, in seconds. Keeps the
// milliseconds conversion (interval * 1000) inside a DWORD.
static const int kMaxIntervalSeconds = 86400;

// Game::GetGameWindow only resolves the handle once and caches it, so it can
// go stale if the game recreates its window.
static HWND ResolveGameWindow() {
    HWND game = GetGameInstance()->GetGameWindow();
    if (game != NULL && IsWindow(game)) {
        return game;
    }
    return NULL;
}

// Only nudge the window while the game itself is focused and on screen, and
// never while the user is holding the mouse button down, so a click or a
// drag-select is not interrupted.
static bool ShouldRefresh(HWND game) {
    if (game == NULL) {
        return false;
    }
    if (!IsWindowVisible(game) || IsIconic(game)) {
        return false;
    }
    if (GetForegroundWindow() != game) {
        return false;
    }
    if (System::IsKeyDown(VK_LBUTTON)) {
        return false;
    }
    return true;
}

// Both moves pass repaint = FALSE: the game repaints by itself once the size
// changes, which avoids a full white flicker of the whole window.
static bool RefreshOnce(HWND game) {
    RECT before;
    RECT now;
    int width = 0;
    int height = 0;
    int attempt = 0;

    if (!GetWindowRect(game, &before)) {
        return false;
    }

    width = before.right - before.left;
    height = before.bottom - before.top;
    if (width <= 0 || height <= 0) {
        return false;
    }

    if (!MoveWindow(game, before.left, before.top, width + 1, height, FALSE)) {
        return false;
    }

    // Restoring the size must not be skipped, otherwise the window stays one
    // pixel wider for the rest of the session. MoveWindow can report success
    // without the size actually sticking, so verify the real geometry and
    // retry instead of trusting the return value.
    for (attempt = 0; attempt < 3; attempt++) {
        MoveWindow(game, before.left, before.top, width, height, FALSE);
        if (GetWindowRect(game, &now)
                && (now.right - now.left) == width
                && (now.bottom - now.top) == height) {
            return true;
        }
    }
    return false;
}

DWORD __stdcall AutoWindowRefreshThread(LPVOID lpThreadParameter) {
    DWORD lastRefresh = GetTickCount();

    while (1) {
        int interval = GetConfig()->m_autoRefreshInterval;
        HWND game = NULL;
        DWORD now = 0;

        if (a_closed) {
            ExitThread(0);
        }

        if (interval > kMaxIntervalSeconds) {
            interval = kMaxIntervalSeconds;
        }

        now = GetTickCount();
        game = ResolveGameWindow();

        if (interval > 0
                && ShouldRefresh(game)
                && (DWORD)(now - lastRefresh) >= (DWORD)interval * 1000) {
            if (RefreshOnce(game)) {
                // Restart the countdown only after a real refresh, so a
                // refresh missed while the game was in the background happens
                // as soon as the player comes back.
                lastRefresh = now;
            }
        }

        Sleep(50);
    }
    return 0;
}

void AutoWindowRefresh::Start() {
    // The thread always runs; the config value decides whether it ever
    // touches the window, so the interval can be retuned while playing.
    this->thread = CreateThread(NULL, NULL, AutoWindowRefreshThread, NULL, NULL, NULL);
}

void AutoWindowRefresh::Stop() {
    a_closed = true;
    TerminateThread(this->thread, 0);
}
