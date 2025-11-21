#include <windows.h>
#include <GL/gl.h>
#include <cmath>
#include <ctime>
#include <vector>

#define NUM_CONFETTI 400
#define TIMER_ID     1
#define FRAME_MS     16 // ~60 FPS

struct Particle {
    float x, y;       // posición
    float vx, vy;     // velocidad
    float angle;      // rotación
    float spin;       // velocidad angular
    float r, g, b;    // color
    float size;       // tamaño
};

std::vector<Particle> confetti;
int width = 800, height = 600;

int lastMouseX = -1, lastMouseY = -1;

// Inicializa partículas con valores aleatorios
void InitConfetti() {
    srand((unsigned)time(nullptr));
    confetti.resize(NUM_CONFETTI);
    for (auto &p : confetti) {
        p.x = rand() % width;
        p.y = rand() % height;
        p.vx = (rand() % 200 - 100) / 200.0f;
        p.vy = (rand() % 200 + 100) / 100.0f;  // velocidad hacia abajo
        p.angle = rand() % 360;
        p.spin = (rand() % 100 - 50) / 100.0f;
        p.r = rand() / (float)RAND_MAX;
        p.g = rand() / (float)RAND_MAX;
        p.b = rand() / (float)RAND_MAX;
        p.size = 3.0f + rand() % 6;
    }
}

void UpdateConfetti(float dt) {
    const float gravity = 0.3f;
    for (auto &p : confetti) {
        p.vy -= gravity * dt;     // gravedad
        p.x += p.vx * dt;
        p.y += p.vy * dt;
        p.angle += p.spin * dt * 10;

        // si sale de la pantalla, reaparece arriba
        if (p.y < -10) {
            p.y = height + 10;
            p.x = rand() % width;
            p.vy = (rand() % 200 + 100) / 100.0f;
            p.r = rand() / (float)RAND_MAX;
            p.g = rand() / (float)RAND_MAX;
            p.b = rand() / (float)RAND_MAX;
        }
        if (p.x < 0) p.x += width;
        if (p.x > width) p.x -= width;
    }
}

void DrawConfetti() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    for (auto &p : confetti) {
        glPushMatrix();
        glTranslatef(p.x, p.y, 0);
        glRotatef(p.angle, 0, 0, 1);
        glColor3f(p.r, p.g, p.b);

        float s = p.size;
        glBegin(GL_QUADS);
        glVertex2f(-s, -s);
        glVertex2f(s, -s);
        glVertex2f(s, s);
        glVertex2f(-s, s);
        glEnd();

        glPopMatrix();
    }
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

        // Configurar OpenGL 2D
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, width, 0, height, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        InitConfetti();

        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&last);
        SetTimer(hwnd, TIMER_ID, FRAME_MS, nullptr);
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
    const char CLASS_NAME[] = "ConfettiOpenGL";

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowExA(0, CLASS_NAME, "Confetti OpenGL avanzado",
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
