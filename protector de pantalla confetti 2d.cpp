// Copyright (c) 2025 Anibal Zanutti // Licensed under the MIT License. See LICENSE file for details.

#include <windows.h>
#include <GL/gl.h>
#include <ctime>
#include <vector>
#include <cmath>

#define NUM_CONFETTI 200

struct Particle {
    float x, y;
    float vx, vy;
    float r, g, b;
    float size;
};

std::vector<Particle> confetti;
int width = 800, height = 600;

int lastMouseX = -1, lastMouseY = -1;

void InitConfetti() {
    srand((unsigned)time(nullptr));
    confetti.resize(NUM_CONFETTI);
    for (auto &p : confetti) {
        p.x = rand() % width;
        p.y = rand() % height;
        p.vx = (rand() % 100 - 50) / 50.0f;
        p.vy = (rand() % 50 + 50) / 50.0f;
        p.r = rand() / (float)RAND_MAX;
        p.g = rand() / (float)RAND_MAX;
        p.b = rand() / (float)RAND_MAX;
        p.size = 3.0f + rand() % 5;
    }
}

void UpdateConfetti(float dt) {
    for (auto &p : confetti) {
        p.y -= p.vy * dt;
        p.x += p.vx * dt;
        if (p.y < 0) {
            p.y = height;
            p.x = rand() % width;
        }
        if (p.x < 0) p.x = width;
        if (p.x > width) p.x = 0;
    }
}

void DrawConfetti() {
    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_QUADS);
    for (auto &p : confetti) {
        glColor3f(p.r, p.g, p.b);
        float s = p.size;
        glVertex2f(p.x - s, p.y - s);
        glVertex2f(p.x + s, p.y - s);
        glVertex2f(p.x + s, p.y + s);
        glVertex2f(p.x - s, p.y + s);
    }
    glEnd();
    glFlush();
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HDC hDC;
    static HGLRC hRC;
    static LARGE_INTEGER freq, last;
    switch (msg) {
    case WM_CREATE: {
        PIXELFORMATDESCRIPTOR pfd = { sizeof(PIXELFORMATDESCRIPTOR), 1 };
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 24;
        hDC = GetDC(hwnd);
        int pf = ChoosePixelFormat(hDC, &pfd);
        SetPixelFormat(hDC, pf, &pfd);
        hRC = wglCreateContext(hDC);
        wglMakeCurrent(hDC, hRC);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, width, 0, height, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glClearColor(1, 1, 1, 1);

        InitConfetti();
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&last);
        SetTimer(hwnd, 1, 16, nullptr); // ~60 FPS
        break;
    }
    case WM_TIMER: {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        float dt = float(now.QuadPart - last.QuadPart) / freq.QuadPart * 60.0f;
        last = now;

        UpdateConfetti(dt);
        DrawConfetti();
        SwapBuffers(hDC);
        break;
    }
    case WM_DESTROY:
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(hRC);
        ReleaseDC(hwnd, hDC);
        PostQuitMessage(0);
        break;
    case WM_KEYDOWN:
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
        PostQuitMessage(0);
        break;
    case WM_MOUSEMOVE:
        if (lastMouseX >= 0 && lastMouseY >= 0) {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            if (abs(x - lastMouseX) > 3 || abs(y - lastMouseY) > 3)
                PostQuitMessage(0);
        }
        lastMouseX = LOWORD(lParam);
        lastMouseY = HIWORD(lParam);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    const char CLASS_NAME[] = "ConfettiGL";

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowExA(0, CLASS_NAME, "Confetti OpenGL",
        WS_POPUP, 0, 0, screenWidth, screenHeight,
        nullptr, nullptr, hInstance, nullptr);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, screenWidth, screenHeight, SWP_SHOWWINDOW);
    ShowCursor(FALSE);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}


