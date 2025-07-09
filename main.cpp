#include <windows.h>
#include <cmath>
#include <string>
#include <vector>
#include <iostream>
using namespace std;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void DrawLineDDA(HDC hdc, int x1, int y1, int x2, int y2);
void DrawLineMidpoint(HDC hdc, int x1, int y1, int x2, int y2);
void DrawLineParametric(HDC hdc, int x1, int y1, int x2, int y2);
void DrawCircleDirect(HDC hdc, int xc, int yc, int R);
void DrawCirclePolar(HDC hdc, int xc, int yc, int R);
void DrawCircleIterativePolar(HDC hdc, int xc, int yc, int R);
void DrawCircleMidpoint(HDC hdc, int xc, int yc, int R);
void DrawCircleModifiedMidpoint(HDC hdc, int xc, int yc, int R);
void FillCircleWithLines(HDC hdc, int xc, int yc, int R, int quarter);
void FillCircleWithCircles(HDC hdc, int xc, int yc, int R, int quarter);
void FillSquareWithHermite(HDC hdc, int x, int y, int size);
void FillRectangleWithBezier(HDC hdc, int x, int y, int width, int height);
void FillConvexPolygon(HDC hdc, const std::vector<POINT>& points, COLORREF color);
void FillNonConvexPolygon(HDC hdc, const std::vector<POINT>& points, COLORREF color);
void FloodFillRecursive(HDC hdc, int x, int y, COLORREF fillColor, COLORREF borderColor);
void FloodFillNonRecursive(HDC hdc, int x, int y, COLORREF fillColor, COLORREF borderColor);
void DrawCardinalSpline(HDC hdc, const std::vector<POINT>& points, double tension);
void DrawEllipseDirect(HDC hdc, int xc, int yc, int A, int B);
void DrawEllipsePolar(HDC hdc, int xc, int yc, int A, int B);
void DrawEllipseMidpoint(HDC hdc, int xc, int yc, int A, int B);
void ClipPoint(HDC hdc, POINT p, RECT clip, COLORREF color);
void ClipLine(HDC hdc, POINT p1, POINT p2, RECT clip);
void ClipPolygon(HDC hdc, std::vector<POINT> poly, RECT clip);
void EnableConsole();
void SaveShapes(const char* filename);
void LoadShapes(HWND hwnd, const char* filename);

enum DrawMode {
    NONE, LINE_DDA, LINE_MIDPOINT, LINE_PARAMETRIC,
    CIRCLE_DIRECT, CIRCLE_POLAR, CIRCLE_ITERATIVE_POLAR,
    CIRCLE_MIDPOINT, CIRCLE_MODIFIED_MIDPOINT,FILL_CIRCLE_LINES,
    FILL_CIRCLE_CIRCLES, FILL_SQUARE_HERMITE, FILL_RECTANGLE_BEZIER,
    FILL_CONVEX, FILL_NONCONVEX, FLOOD_RECURSIVE, FLOOD_NONRECURSIVE, CARDINAL_SPLINE,
    ELLIPSE_DIRECT, ELLIPSE_POLAR, ELLIPSE_MIDPOINT,
    CLIP_RECT_POINT, CLIP_RECT_LINE, CLIP_RECT_POLYGON,
    CLIP_SQUARE_POINT, CLIP_SQUARE_LINE
};

struct Shape {
    DrawMode type;
    POINT p1, p2;
    vector<POINT> polygon;
    COLORREF color;
    int extraData = 0;
};



vector<Shape> shapes;
DrawMode currentMode = NONE;
COLORREF currentColor = RGB(0, 0, 0);
POINT startPoint;
bool isDrawing = false;

vector<POINT> polygonPoints;
bool collectingPolygon = false;


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    EnableConsole();

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "DrawingApp";
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.hCursor = LoadCursor(NULL, IDC_CROSS);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "DrawingApp", "2D Drawing Application", WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

void EnableConsole() {
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    std::cout << "Console initialized.\n";
}

int PromptQuarterSelection() {
    return 2;
}


void AddMenus(HWND hwnd) {
    HMENU hMenubar = CreateMenu();
    HMENU hMenu = CreateMenu();
    HMENU hColorMenu = CreateMenu();

    AppendMenu(hMenu, MF_STRING, 1, "Line DDA");
    AppendMenu(hMenu, MF_STRING, 2, "Line Midpoint");
    AppendMenu(hMenu, MF_STRING, 3, "Line Parametric");
    AppendMenu(hMenu, MF_STRING, 4, "Circle Direct");
    AppendMenu(hMenu, MF_STRING, 5, "Circle Polar");
    AppendMenu(hMenu, MF_STRING, 6, "Circle Iterative Polar");
    AppendMenu(hMenu, MF_STRING, 7, "Circle Midpoint");
    AppendMenu(hMenu, MF_STRING, 8, "Circle Modified Midpoint");
    AppendMenu(hMenu, MF_STRING, 16, "Fill Circle With Lines");
    AppendMenu(hMenu, MF_STRING, 17, "Fill Circle With Circles");
    AppendMenu(hMenu, MF_STRING, 18, "Fill Square With Hermite");
    AppendMenu(hMenu, MF_STRING, 19, "Fill Rectangle With Bezier");
    AppendMenu(hMenu, MF_STRING, 20, "Convex Polygon Fill");
    AppendMenu(hMenu, MF_STRING, 21, "Non-Convex Polygon Fill");
    AppendMenu(hMenu, MF_STRING, 22, "Recursive Flood Fill");
    AppendMenu(hMenu, MF_STRING, 23, "Non-Recursive Flood Fill");
    AppendMenu(hMenu, MF_STRING, 24, "Cardinal Spline Curve");
    AppendMenu(hMenu, MF_STRING, 25, "Ellipse Direct");
    AppendMenu(hMenu, MF_STRING, 26, "Ellipse Polar");
    AppendMenu(hMenu, MF_STRING, 27, "Ellipse Midpoint");
    AppendMenu(hMenu, MF_STRING, 28, "Clip Rect: Point");
    AppendMenu(hMenu, MF_STRING, 29, "Clip Rect: Line");
    AppendMenu(hMenu, MF_STRING, 30, "Clip Rect: Polygon");
    AppendMenu(hMenu, MF_STRING, 31, "Clip Square: Point");
    AppendMenu(hMenu, MF_STRING, 32, "Clip Square: Line");

    AppendMenu(hColorMenu, MF_STRING, 9, "Red");
    AppendMenu(hColorMenu, MF_STRING, 10, "Green");
    AppendMenu(hColorMenu, MF_STRING, 11, "Blue");
    AppendMenu(hColorMenu, MF_STRING, 12, "Black");

    AppendMenu(hMenubar, MF_POPUP, (UINT_PTR)hMenu, "Algorithms");
    AppendMenu(hMenubar, MF_POPUP, (UINT_PTR)hColorMenu, "Colors");
    AppendMenu(hMenubar, MF_STRING, 13, "Clear Screen");
    AppendMenu(hMenubar, MF_STRING, 14, "Save");
    AppendMenu(hMenubar, MF_STRING, 15, "Load");

    SetMenu(hwnd, hMenubar);
}
void DrawLineDDA(HDC hdc, int x1, int y1, int x2, int y2) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = max(abs(dx), abs(dy));
    float xInc = dx / (float)steps;
    float yInc = dy / (float)steps;
    float x = x1, y = y1;
    for (int i = 0; i <= steps; i++) {
        SetPixel(hdc, round(x), round(y), currentColor);
        x += xInc;
        y += yInc;
    }
}

void DrawLineMidpoint(HDC hdc, int x1, int y1, int x2, int y2) {
    int dx = x2 - x1, dy = y2 - y1;
    int d = dy - dx / 2;
    int x = x1, y = y1;
    SetPixel(hdc, x, y, currentColor);
    while (x < x2) {
        x++;
        if (d < 0) {
            d += dy;
        } else {
            d += dy - dx;
            y++;
        }
        SetPixel(hdc, x, y, currentColor);
    }
}

void DrawLineParametric(HDC hdc, int x1, int y1, int x2, int y2) {
    for (double t = 0; t <= 1; t += 0.001) {
        int x = round(x1 + t * (x2 - x1));
        int y = round(y1 + t * (y2 - y1));
        SetPixel(hdc, x, y, currentColor);
    }
}

void DrawCircleDirect(HDC hdc, int xc, int yc, int R) {
    for (int x = 0; x <= R / sqrt(2); x++) {
        int y = round(sqrt(R * R - x * x));
        SetPixel(hdc, xc + x, yc + y, currentColor);
        SetPixel(hdc, xc - x, yc + y, currentColor);
        SetPixel(hdc, xc + x, yc - y, currentColor);
        SetPixel(hdc, xc - x, yc - y, currentColor);
        SetPixel(hdc, xc + y, yc + x, currentColor);
        SetPixel(hdc, xc - y, yc + x, currentColor);
        SetPixel(hdc, xc + y, yc - x, currentColor);
        SetPixel(hdc, xc - y, yc - x, currentColor);
    }
}

void DrawCirclePolar(HDC hdc, int xc, int yc, int R) {
    for (double theta = 0; theta <= 2 * 3.14159; theta += 0.001) {
        int x = round(R * cos(theta));
        int y = round(R * sin(theta));
        SetPixel(hdc, xc + x, yc + y, currentColor);
    }
}

void DrawCircleIterativePolar(HDC hdc, int xc, int yc, int R) {
    double x = R, y = 0;
    double theta = 1.0 / R;
    double cosTheta = cos(theta), sinTheta = sin(theta);
    for (int i = 0; i <= 2 * 3.14159 * R; i++) {
        SetPixel(hdc, round(xc + x), round(yc + y), currentColor);
        double x1 = x * cosTheta - y * sinTheta;
        y = x * sinTheta + y * cosTheta;
        x = x1;
    }
}

void DrawCircleMidpoint(HDC hdc, int xc, int yc, int R) {
    int x = 0, y = R;
    int d = 1 - R;
    while (x <= y) {
        SetPixel(hdc, xc + x, yc + y, currentColor);
        SetPixel(hdc, xc - x, yc + y, currentColor);
        SetPixel(hdc, xc + x, yc - y, currentColor);
        SetPixel(hdc, xc - x, yc - y, currentColor);
        SetPixel(hdc, xc + y, yc + x, currentColor);
        SetPixel(hdc, xc - y, yc + x, currentColor);
        SetPixel(hdc, xc + y, yc - x, currentColor);
        SetPixel(hdc, xc - y, yc - x, currentColor);
        if (d < 0) d += 2 * x + 3;
        else {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
    }
}

void DrawCircleModifiedMidpoint(HDC hdc, int xc, int yc, int R) {
    int x = 0, y = R;
    int d = 1 - R;
    int c1 = 3, c2 = 5 - 2 * R;
    while (x <= y) {
        SetPixel(hdc, xc + x, yc + y, currentColor);
        SetPixel(hdc, xc - x, yc + y, currentColor);
        SetPixel(hdc, xc + x, yc - y, currentColor);
        SetPixel(hdc, xc - x, yc - y, currentColor);
        SetPixel(hdc, xc + y, yc + x, currentColor);
        SetPixel(hdc, xc - y, yc + x, currentColor);
        SetPixel(hdc, xc + y, yc - x, currentColor);
        SetPixel(hdc, xc - y, yc - x, currentColor);
        if (d < 0) {
            d += c1;
            c2 += 2;
        } else {
            d += c2;
            c2 += 4;
            y--;
        }
        c1 += 2;
        x++;
    }
}
void FillCircleWithLines(HDC hdc, int xc, int yc, int R, int quarter) {
    for (int r = 0; r <= R; r += 5) {
        int x = 0, y = r;
        int d = 1 - r;
        while (x <= y) {
            if (quarter == 1) {
                DrawLineDDA(hdc, xc, yc, xc + x, yc - y);
                DrawLineDDA(hdc, xc, yc, xc + y, yc - x);
            } else if (quarter == 2) {
                DrawLineDDA(hdc, xc, yc, xc - x, yc - y);
                DrawLineDDA(hdc, xc, yc, xc - y, yc - x);
            } else if (quarter == 3) {
                DrawLineDDA(hdc, xc, yc, xc - x, yc + y);
                DrawLineDDA(hdc, xc, yc, xc - y, yc + x);
            } else if (quarter == 4) {
                DrawLineDDA(hdc, xc, yc, xc + x, yc + y);
                DrawLineDDA(hdc, xc, yc, xc + y, yc + x);
            }
            if (d < 0) d += 2 * x + 3;
            else {
                d += 2 * (x - y) + 5;
                y--;
            }
            x++;
        }
    }
}

void FillCircleWithCircles(HDC hdc, int xc, int yc, int R, int quarter) {
    for (int r = 0; r <= R; r += 5) {
        switch (quarter) {
            case 1:
                DrawCirclePolar(hdc, xc, yc - R + r, r);
                break;
            case 2:
                DrawCirclePolar(hdc, xc - R + r, yc, r);
                break;
            case 3:
                DrawCirclePolar(hdc, xc, yc + R - r, r);
                break;
            case 4:
                DrawCirclePolar(hdc, xc + R - r, yc, r);
                break;
        }
    }
}

void HermiteCurve(HDC hdc, POINT p0, POINT p1, POINT t0, POINT t1) {
    for (double t = 0; t <= 1; t += 0.001) {
        double h1 = 2 * t * t * t - 3 * t * t + 1;
        double h2 = -2 * t * t * t + 3 * t * t;
        double h3 = t * t * t - 2 * t * t + t;
        double h4 = t * t * t - t * t;
        int x = round(h1 * p0.x + h2 * p1.x + h3 * t0.x + h4 * t1.x);
        int y = round(h1 * p0.y + h2 * p1.y + h3 * t0.y + h4 * t1.y);
        SetPixel(hdc, x, y, currentColor);
    }
}

void FillSquareWithHermite(HDC hdc, int x, int y, int size) {
    POINT p0 = { x, y };
    POINT p1 = { x, y + size };

    POINT t0 = { 50, 100 };
    POINT t1 = { 50, 100 };

    for (int i = 0; i <= size; i += 5) {
        POINT offsetP0 = { p0.x + i, p0.y };
        POINT offsetP1 = { p1.x + i, p1.y };
        HermiteCurve(hdc, offsetP0, offsetP1, t0, t1);
    }
}


void BezierCurve(HDC hdc, POINT p0, POINT p1, POINT p2, POINT p3) {
    for (double t = 0; t <= 1; t += 0.001) {
        double x = pow(1 - t, 3) * p0.x + 3 * pow(1 - t, 2) * t * p1.x + 3 * (1 - t) * t * t * p2.x + t * t * t * p3.x;
        double y = pow(1 - t, 3) * p0.y + 3 * pow(1 - t, 2) * t * p1.y + 3 * (1 - t) * t * t * p2.y + t * t * t * p3.y;
        SetPixel(hdc, round(x), round(y), currentColor);
    }
}

void FillRectangleWithBezier(HDC hdc, int x, int y, int width, int height) {
    for (int i = 0; i <= height; i += 5) {
        POINT p0 = { x, y + i };
        POINT p1 = { x + width / 3, y + i - 20 };
        POINT p2 = { x + 2 * width / 3, y + i + 20 };
        POINT p3 = { x + width, y + i };
        BezierCurve(hdc, p0, p1, p2, p3);
    }
}

void FillConvexPolygon(HDC hdc, const std::vector<POINT>& points, COLORREF color) {
    HBRUSH hBrush = CreateSolidBrush(color);
    SelectObject(hdc, hBrush);
    Polygon(hdc, points.data(), points.size());
    DeleteObject(hBrush);
}

void FillNonConvexPolygon(HDC hdc, const std::vector<POINT>& points, COLORREF color) {
    HRGN hRgn = CreatePolygonRgn(points.data(), points.size(), WINDING);
    HBRUSH hBrush = CreateSolidBrush(color);
    FillRgn(hdc, hRgn, hBrush);
    DeleteObject(hRgn);
    DeleteObject(hBrush);
}

void FloodFillRecursive(HDC hdc, int x, int y, COLORREF fillColor, COLORREF borderColor) {
    if (x < 0 || y < 0 || x >= 800 || y >= 600) return;
    COLORREF current = GetPixel(hdc, x, y);
    if (GetPixel(hdc, x, y) == CLR_INVALID) return;
    if (current != borderColor && current != fillColor) {
        SetPixel(hdc, x, y, fillColor);
        FloodFillRecursive(hdc, x + 1, y, fillColor, borderColor);
        FloodFillRecursive(hdc, x - 1, y, fillColor, borderColor);
        FloodFillRecursive(hdc, x, y + 1, fillColor, borderColor);
        FloodFillRecursive(hdc, x, y - 1, fillColor, borderColor);
    }
}


void FloodFillNonRecursive(HDC hdc, int x, int y, COLORREF fillColor, COLORREF borderColor) {
    if (x < 0 || y < 0 || x >= 800 || y >= 600) return;
    std::vector<POINT> stack;
    stack.push_back({x, y});

    while (!stack.empty()) {
        POINT p = stack.back();
        stack.pop_back();

        if (p.x < 0 || p.y < 0 || p.x >= 800 || p.y >= 600) continue;
        COLORREF current = GetPixel(hdc, p.x, p.y);
        if (current != borderColor && current != fillColor) {
            SetPixel(hdc, p.x, p.y, fillColor);
            stack.push_back({p.x + 1, p.y});
            stack.push_back({p.x - 1, p.y});
            stack.push_back({p.x, p.y + 1});
            stack.push_back({p.x, p.y - 1});
        }
    }
}


void DrawCardinalSpline(HDC hdc, const std::vector<POINT>& points, double tension) {
    if (points.size() < 4) return;

    for (size_t i = 1; i < points.size() - 2; ++i) {
        POINT p0 = points[i - 1];
        POINT p1 = points[i];
        POINT p2 = points[i + 1];
        POINT p3 = points[i + 2];

        for (double t = 0; t <= 1; t += 0.001) {
            double t2 = t * t;
            double t3 = t2 * t;

            double m0x = (p2.x - p0.x) * tension;
            double m0y = (p2.y - p0.y) * tension;
            double m1x = (p3.x - p1.x) * tension;
            double m1y = (p3.y - p1.y) * tension;

            double x = (2 * t3 - 3 * t2 + 1) * p1.x +
                       (t3 - 2 * t2 + t) * m0x +
                       (-2 * t3 + 3 * t2) * p2.x +
                       (t3 - t2) * m1x;

            double y = (2 * t3 - 3 * t2 + 1) * p1.y +
                       (t3 - 2 * t2 + t) * m0y +
                       (-2 * t3 + 3 * t2) * p2.y +
                       (t3 - t2) * m1y;

            SetPixel(hdc, round(x), round(y), currentColor);
        }
    }
}

void DrawEllipseDirect(HDC hdc, int xc, int yc, int A, int B) {
    int x = 0;
    for (x = -A; x <= A; ++x) {
        int y = round(B * sqrt(1 - (x * x) / (double)(A * A)));
        SetPixel(hdc, xc + x, yc + y, currentColor);
        SetPixel(hdc, xc + x, yc - y, currentColor);
    }
}

void DrawEllipsePolar(HDC hdc, int xc, int yc, int A, int B) {
    for (double theta = 0; theta <= 2 * 3.14159; theta += 0.001) {
        int x = round(A * cos(theta));
        int y = round(B * sin(theta));
        SetPixel(hdc, xc + x, yc + y, currentColor);
    }
}

void DrawEllipseMidpoint(HDC hdc, int xc, int yc, int A, int B) {
    int x = 0;
    int y = B;
    long A2 = A * A, B2 = B * B;
    long d = B2 - A2 * B + 0.25 * A2;
    long dx = 2 * B2 * x;
    long dy = 2 * A2 * y;

    while (dx < dy) {
        SetPixel(hdc, xc + x, yc + y, currentColor);
        SetPixel(hdc, xc - x, yc + y, currentColor);
        SetPixel(hdc, xc + x, yc - y, currentColor);
        SetPixel(hdc, xc - x, yc - y, currentColor);
        if (d < 0) {
            x++;
            dx += 2 * B2;
            d += dx + B2;
        } else {
            x++;
            y--;
            dx += 2 * B2;
            dy -= 2 * A2;
            d += dx - dy + B2;
        }
    }
    d = B2 * (x + 0.5) * (x + 0.5) + A2 * (y - 1) * (y - 1) - A2 * B2;
    while (y >= 0) {
        SetPixel(hdc, xc + x, yc + y, currentColor);
        SetPixel(hdc, xc - x, yc + y, currentColor);
        SetPixel(hdc, xc + x, yc - y, currentColor);
        SetPixel(hdc, xc - x, yc - y, currentColor);
        if (d > 0) {
            y--;
            dy -= 2 * A2;
            d += A2 - dy;
        } else {
            y--;
            x++;
            dx += 2 * B2;
            dy -= 2 * A2;
            d += dx - dy + A2;
        }
    }
}


bool IsInside(POINT p, RECT clip) {
    return (p.x >= clip.left && p.x <= clip.right && p.y >= clip.top && p.y <= clip.bottom);
}


void ClipPoint(HDC hdc, POINT p, RECT clip, COLORREF color) {
    if (IsInside(p, clip)) {
        SetPixel(hdc, p.x, p.y, color);
    }
}

const int INSIDE = 0;
const int LEFT = 1;
const int RIGHT = 2;
const int BOTTOM = 4;
const int TOP = 8;

int ComputeOutCode(POINT p, RECT clip) {
    int code = INSIDE;
    if (p.x < clip.left) code |= LEFT;
    else if (p.x > clip.right) code |= RIGHT;
    if (p.y < clip.top) code |= TOP;
    else if (p.y > clip.bottom) code |= BOTTOM;
    return code;
}

void ClipLine(HDC hdc, POINT p1, POINT p2, RECT clip) {
    int outcode1 = ComputeOutCode(p1, clip);
    int outcode2 = ComputeOutCode(p2, clip);
    bool accept = false;

    while (true) {
        if (!(outcode1 | outcode2)) {
            accept = true;
            break;
        } else if (outcode1 & outcode2) {
            break;
        } else {
            int outcodeOut = outcode1 ? outcode1 : outcode2;
            int x, y;

            if (outcodeOut & TOP) {
                x = p1.x + (p2.x - p1.x) * (clip.top - p1.y) / (p2.y - p1.y);
                y = clip.top;
            } else if (outcodeOut & BOTTOM) {
                x = p1.x + (p2.x - p1.x) * (clip.bottom - p1.y) / (p2.y - p1.y);
                y = clip.bottom;
            } else if (outcodeOut & RIGHT) {
                y = p1.y + (p2.y - p1.y) * (clip.right - p1.x) / (p2.x - p1.x);
                x = clip.right;
            } else {
                y = p1.y + (p2.y - p1.y) * (clip.left - p1.x) / (p2.x - p1.x);
                x = clip.left;
            }

            if (outcodeOut == outcode1) {
                p1 = {x, y};
                outcode1 = ComputeOutCode(p1, clip);
            } else {
                p2 = {x, y};
                outcode2 = ComputeOutCode(p2, clip);
            }
        }
    }

    if (accept) {
        HPEN hPen = CreatePen(PS_SOLID, 1, currentColor);
        HPEN oldPen = (HPEN)SelectObject(hdc, hPen);
        MoveToEx(hdc, p1.x, p1.y, NULL);
        LineTo(hdc, p2.x, p2.y);
        SelectObject(hdc, oldPen);
        DeleteObject(hPen);
    }

}


vector<POINT> ClipEdge(std::vector<POINT>& poly, int edge, RECT clip) {
    vector<POINT> out;
    for (size_t i = 0; i < poly.size(); ++i) {
        POINT curr = poly[i];
        POINT prev = poly[(i + poly.size() - 1) % poly.size()];

        bool inCurr = IsInside(curr, clip);
        bool inPrev = IsInside(prev, clip);

        if (inPrev && inCurr) {
            out.push_back(curr);
        } else if (inPrev && !inCurr) {
            // prev in, curr out
            // skip
        } else if (!inPrev && inCurr) {
            out.push_back(curr);
        }
    }
    return out;
}

void ClipPolygon(HDC hdc, vector<POINT> poly, RECT clip, COLORREF color) {
    vector<POINT> clipped = poly;

    auto SutherlandHodgman = [&](vector<POINT>& input, auto inside, auto intersect) {
        vector<POINT> output;
        for (size_t i = 0; i < input.size(); ++i) {
            POINT curr = input[i];
            POINT prev = input[(i + input.size() - 1) % input.size()];
            bool inCurr = inside(curr);
            bool inPrev = inside(prev);

            if (inPrev && inCurr) {
                output.push_back(curr);
            } else if (inPrev && !inCurr) {
                output.push_back(intersect(prev, curr));
            } else if (!inPrev && inCurr) {
                output.push_back(intersect(prev, curr));
                output.push_back(curr);
            }
        }
        return output;
    };

    clipped = SutherlandHodgman(clipped,
                                [&](POINT p){ return p.x >= clip.left; },
                                [&](POINT p1, POINT p2){
                                    POINT i;
                                    i.x = clip.left;
                                    i.y = p1.y + (clip.left - p1.x) * (p2.y - p1.y) / (p2.x - p1.x);
                                    return i;
                                });

    clipped = SutherlandHodgman(clipped,
                                [&](POINT p){ return p.x <= clip.right; },
                                [&](POINT p1, POINT p2){
                                    POINT i;
                                    i.x = clip.right;
                                    i.y = p1.y + (clip.right - p1.x) * (p2.y - p1.y) / (p2.x - p1.x);
                                    return i;
                                });

    clipped = SutherlandHodgman(clipped,
                                [&](POINT p){ return p.y >= clip.top; },
                                [&](POINT p1, POINT p2){
                                    POINT i;
                                    i.y = clip.top;
                                    i.x = p1.x + (clip.top - p1.y) * (p2.x - p1.x) / (p2.y - p1.y);
                                    return i;
                                });

    clipped = SutherlandHodgman(clipped,
                                [&](POINT p){ return p.y <= clip.bottom; },
                                [&](POINT p1, POINT p2){
                                    POINT i;
                                    i.y = clip.bottom;
                                    i.x = p1.x + (clip.bottom - p1.y) * (p2.x - p1.x) / (p2.y - p1.y);
                                    return i;
                                });

    if (clipped.size() >= 3) {
        HBRUSH brush = CreateSolidBrush(color);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
        Polygon(hdc, clipped.data(), clipped.size());
        SelectObject(hdc, oldBrush);
        DeleteObject(brush);
    }
}


void DrawClippingWindow(HDC hdc, RECT clip, COLORREF borderColor = RGB(0, 0, 0)) {
    HPEN hPen = CreatePen(PS_DASH, 1, borderColor);
    HPEN oldPen = (HPEN)SelectObject(hdc, hPen);
    MoveToEx(hdc, clip.left, clip.top, NULL);
    LineTo(hdc, clip.right, clip.top);
    LineTo(hdc, clip.right, clip.bottom);
    LineTo(hdc, clip.left, clip.bottom);
    LineTo(hdc, clip.left, clip.top);
    SelectObject(hdc, oldPen);
    DeleteObject(hPen);
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            AddMenus(hwnd);
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case 1: currentMode = LINE_DDA; break;
                case 2: currentMode = LINE_MIDPOINT; break;
                case 3: currentMode = LINE_PARAMETRIC; break;
                case 4: currentMode = CIRCLE_DIRECT; break;
                case 5: currentMode = CIRCLE_POLAR; break;
                case 6: currentMode = CIRCLE_ITERATIVE_POLAR; break;
                case 7: currentMode = CIRCLE_MIDPOINT; break;
                case 8: currentMode = CIRCLE_MODIFIED_MIDPOINT; break;
                case 9: currentColor = RGB(255, 0, 0); break;
                case 10: currentColor = RGB(0, 255, 0); break;
                case 11: currentColor = RGB(0, 0, 255); break;
                case 12: currentColor = RGB(0, 0, 0); break;
                case 13: shapes.clear(); InvalidateRect(hwnd, NULL, TRUE); break;
                case 14: SaveShapes("shapes.txt"); break;
                case 15: LoadShapes(hwnd, "shapes.txt"); break;
                case 16: currentMode = FILL_CIRCLE_LINES; break;
                case 17: currentMode = FILL_CIRCLE_CIRCLES; break;
                case 18: currentMode = FILL_SQUARE_HERMITE; break;
                case 19: currentMode = FILL_RECTANGLE_BEZIER; break;
                case 20: currentMode = FILL_CONVEX; break;
                case 21: currentMode = FILL_NONCONVEX; break;
                case 22: currentMode = FLOOD_RECURSIVE; break;
                case 23: currentMode = FLOOD_NONRECURSIVE; break;
                case 24: currentMode = CARDINAL_SPLINE; break;
                case 25: currentMode = ELLIPSE_DIRECT; break;
                case 26: currentMode = ELLIPSE_POLAR; break;
                case 27: currentMode = ELLIPSE_MIDPOINT; break;
                case 28: currentMode = CLIP_RECT_POINT; break;
                case 29: currentMode = CLIP_RECT_LINE; break;
                case 30: currentMode = CLIP_RECT_POLYGON; break;
                case 31: currentMode = CLIP_SQUARE_POINT; break;
                case 32: currentMode = CLIP_SQUARE_LINE; break;
            }
            break;

        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);

            if (currentMode == FILL_CONVEX || currentMode == FILL_NONCONVEX || currentMode == CARDINAL_SPLINE ||
                currentMode == CLIP_RECT_POLYGON || currentMode == CLIP_SQUARE_POINT) {
                polygonPoints.push_back({x, y});
                collectingPolygon = true;
            }
            else if (currentMode == FLOOD_RECURSIVE || currentMode == FLOOD_NONRECURSIVE) {
                HDC hdc = GetDC(hwnd);
                COLORREF borderColor = RGB(0, 0, 0);
                if (currentMode == FLOOD_RECURSIVE)
                    FloodFillRecursive(hdc, x, y, currentColor, borderColor);
                else
                    FloodFillNonRecursive(hdc, x, y, currentColor, borderColor);
                ReleaseDC(hwnd, hdc);
            }
            else {
                startPoint = {x, y};
                isDrawing = true;
            }
            break;
        }

        case WM_RBUTTONDOWN: {
            HDC hdc = GetDC(hwnd);

            if (collectingPolygon && polygonPoints.size() >= 3) {
                RECT clipWindow = {100, 100, 300, 300};  // Rectangle or Square as clipping window
                vector<POINT> clipped;

                if (currentMode == CLIP_RECT_POLYGON) {
                    ClipPolygon(hdc, polygonPoints, clipWindow, currentColor);
                }
                else if (currentMode == CLIP_SQUARE_POINT) {
                    for (const POINT& p : polygonPoints) {
                        ClipPoint(hdc, p, {200, 200, 300, 300}, currentColor);
                    }
                }
                else if (currentMode == FILL_CONVEX) {
                    FillConvexPolygon(hdc, polygonPoints, currentColor);
                }
                else if (currentMode == FILL_NONCONVEX) {
                    FillNonConvexPolygon(hdc, polygonPoints, currentColor);
                }
                else if (currentMode == CARDINAL_SPLINE) {
                    DrawCardinalSpline(hdc, polygonPoints, 0.25);
                }

                collectingPolygon = false;
                polygonPoints.clear();
            }

            ReleaseDC(hwnd, hdc);
            break;
        }

        case WM_LBUTTONUP:
            if (isDrawing) {
                HDC hdc = GetDC(hwnd);
                int x2 = LOWORD(lParam);
                int y2 = HIWORD(lParam);
                int R = static_cast<int>(hypot(x2 - startPoint.x, y2 - startPoint.y));

                switch (currentMode) {
                    case FILL_CIRCLE_LINES: {
                        int quarter = PromptQuarterSelection();
                        if (quarter >= 1 && quarter <= 4) {
                            FillCircleWithLines(hdc, startPoint.x, startPoint.y, R, quarter);
                            Shape shape;
                            shape.type = FILL_CIRCLE_LINES;
                            shape.p1 = startPoint;
                            shape.p2 = {x2, y2};
                            shape.color = currentColor;
                            shape.extraData = quarter;
                            shapes.push_back(shape);

                        } else {
                            MessageBox(hwnd, "Invalid quarter selected.", "Error", MB_OK | MB_ICONERROR);
                        }
                        break;
                    }
                    case FILL_CIRCLE_CIRCLES: {
                        int quarter = PromptQuarterSelection();
                        if (quarter >= 1 && quarter <= 4) {
                            FillCircleWithCircles(hdc, startPoint.x, startPoint.y, R, quarter);
                            Shape shape;
                            shape.type = FILL_CIRCLE_CIRCLES;
                            shape.p1 = startPoint;
                            shape.p2 = {x2, y2};
                            shape.color = currentColor;
                            shape.extraData = quarter;
                            shapes.push_back(shape);

                        } else {
                            MessageBox(hwnd, "Invalid quarter selected.", "Error", MB_OK | MB_ICONERROR);
                        }
                        break;
                    }
                    case FILL_SQUARE_HERMITE: {
                        FillSquareWithHermite(hdc, startPoint.x, startPoint.y, abs(x2 - startPoint.x));
                        Shape shape;
                        shape.type = FILL_SQUARE_HERMITE;
                        shape.p1 = startPoint;
                        shape.p2 = {x2, y2};
                        shape.color = currentColor;
                        shapes.push_back(shape);

                        break;
                    }
                    case FILL_RECTANGLE_BEZIER: {
                        FillRectangleWithBezier(hdc, startPoint.x, startPoint.y, abs(x2 - startPoint.x),
                                                abs(y2 - startPoint.y));
                        Shape shape;
                        shape.type = FILL_RECTANGLE_BEZIER;
                        shape.p1 = startPoint;
                        shape.p2 = {x2, y2};
                        shape.color = currentColor;
                        shapes.push_back(shape);

                        break;
                    }
                    default:
                        int x2 = LOWORD(lParam);
                        int y2 = HIWORD(lParam);

                        Shape s;
                        s.type = currentMode;
                        s.p1 = startPoint;
                        s.p2 = {x2, y2};
                        s.color = currentColor;
                        shapes.push_back(s);

                        shapes.push_back(s);

                        InvalidateRect(hwnd, NULL, FALSE);
                        ReleaseDC(hwnd, hdc);
                        isDrawing = false;
                        break;
                }

                ReleaseDC(hwnd, hdc);
                isDrawing = false;
            }
            break;


        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // Draw rectangle and square clipping windows
            DrawClippingWindow(hdc, {100, 100, 300, 300});
            DrawClippingWindow(hdc, {200, 200, 300, 300}, RGB(255, 0, 0));


            for (const Shape& s : shapes) {
                COLORREF prevColor = currentColor;
                currentColor = s.color;

                switch (s.type) {
                    case LINE_DDA:
                        DrawLineDDA(hdc, s.p1.x, s.p1.y, s.p2.x, s.p2.y);
                        break;

                    case LINE_MIDPOINT:
                        DrawLineMidpoint(hdc, s.p1.x, s.p1.y, s.p2.x, s.p2.y);
                        break;

                    case LINE_PARAMETRIC:
                        DrawLineParametric(hdc, s.p1.x, s.p1.y, s.p2.x, s.p2.y);
                        break;

                    case CIRCLE_DIRECT:
                    case CIRCLE_POLAR:
                    case CIRCLE_ITERATIVE_POLAR:
                    case CIRCLE_MIDPOINT:
                    case CIRCLE_MODIFIED_MIDPOINT: {
                        int R = static_cast<int>(hypot(s.p2.x - s.p1.x, s.p2.y - s.p1.y));
                        if (R == 0) break;
                        switch (s.type) {
                            case CIRCLE_DIRECT:
                                DrawCircleDirect(hdc, s.p1.x, s.p1.y, R);
                                break;
                            case CIRCLE_POLAR:
                                DrawCirclePolar(hdc, s.p1.x, s.p1.y, R);
                                break;
                            case CIRCLE_ITERATIVE_POLAR:
                                DrawCircleIterativePolar(hdc, s.p1.x, s.p1.y, R);
                                break;
                            case CIRCLE_MIDPOINT:
                                DrawCircleMidpoint(hdc, s.p1.x, s.p1.y, R);
                                break;
                            case CIRCLE_MODIFIED_MIDPOINT:
                                DrawCircleModifiedMidpoint(hdc, s.p1.x, s.p1.y, R);
                                break;
                        }
                        break;
                    }

                    case FILL_CIRCLE_LINES: {
                        int R = static_cast<int>(hypot(s.p2.x - s.p1.x, s.p2.y - s.p1.y));
                        if (R == 0) break;
                        FillCircleWithLines(hdc, s.p1.x, s.p1.y, R, s.extraData);
                        break;
                    }

                    case FILL_CIRCLE_CIRCLES: {
                        int R = static_cast<int>(hypot(s.p2.x - s.p1.x, s.p2.y - s.p1.y));
                        if (R == 0) break;
                        FillCircleWithCircles(hdc, s.p1.x, s.p1.y, R, s.extraData);
                        break;
                    }

                    case FILL_SQUARE_HERMITE:
                        FillSquareWithHermite(hdc, s.p1.x, s.p1.y, abs(s.p2.x - s.p1.x));
                        break;

                    case FILL_RECTANGLE_BEZIER:
                        FillRectangleWithBezier(hdc, s.p1.x, s.p1.y, abs(s.p2.x - s.p1.x), abs(s.p2.y - s.p1.y));
                        break;

                    case ELLIPSE_DIRECT:
                    case ELLIPSE_POLAR:
                    case ELLIPSE_MIDPOINT: {
                        int A = abs(s.p2.x - s.p1.x);
                        int B = abs(s.p2.y - s.p1.y);
                        if (A == 0 || B == 0) break;
                        switch (s.type) {
                            case ELLIPSE_DIRECT:
                                DrawEllipseDirect(hdc, s.p1.x, s.p1.y, A, B);
                                break;
                            case ELLIPSE_POLAR:
                                DrawEllipsePolar(hdc, s.p1.x, s.p1.y, A, B);
                                break;
                            case ELLIPSE_MIDPOINT:
                                DrawEllipseMidpoint(hdc, s.p1.x, s.p1.y, A, B);
                                break;
                        }
                        break;
                    }

                    case CLIP_RECT_POINT:
                        ClipPoint(hdc, s.p1, {100, 100, 300, 300}, s.color);  // Example rect
                        break;
                    case CLIP_RECT_LINE: {
                        ClipLine(hdc, s.p1, s.p2, {100, 100, 300, 300});
                        break;
                    }
                    case CLIP_RECT_POLYGON: {
                        if (!s.polygon.empty())
                            ClipPolygon(hdc, s.polygon, {100, 100, 300, 300}, s.color);
                        break;
                    }
                    case CLIP_SQUARE_POINT:
                        ClipPoint(hdc, s.p1, {200, 200, 300, 300}, s.color);  // Example square
                        break;
                    case CLIP_SQUARE_LINE:
                        ClipLine(hdc, s.p1, s.p2, {200, 200, 300, 300});
                        break;

                    default:
                        break;
                }

                currentColor = prevColor;
            }


            EndPaint(hwnd, &ps);
            break;
        }


        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

void SaveShapes(const char* filename) {
    FILE* f;
    fopen_s(&f, filename, "w");
    if (!f) return;


    for (const Shape& s : shapes) {
        fprintf(f, "S %d %d %d %d %d %d %d %d %d\n",
                s.type, s.p1.x, s.p1.y, s.p2.x, s.p2.y,
                GetRValue(s.color), GetGValue(s.color), GetBValue(s.color), s.extraData);
    }


    fprintf(f, "P %zu\n", polygonPoints.size());
    for (const POINT& pt : polygonPoints) {
        fprintf(f, "%d %d\n", pt.x, pt.y);
    }

    fclose(f);
}


void LoadShapes(HWND hwnd, const char* filename) {
    FILE* f;
    fopen_s(&f, filename, "r");
    if (!f) return;

    shapes.clear();
    polygonPoints.clear();

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == 'S') {
            int type, x1, y1, x2, y2, r, g, b, extra;
            if (sscanf(line + 1, "%d %d %d %d %d %d %d %d %d",
                       &type, &x1, &y1, &x2, &y2, &r, &g, &b, &extra) == 9) {
                Shape s;
                s.type = static_cast<DrawMode>(type);
                s.p1 = {x1, y1};
                s.p2 = {x2, y2};
                s.color = RGB(r, g, b);
                s.extraData = extra;
                shapes.push_back(s);
            }
        } else if (line[0] == 'P') {
            int count;
            if (sscanf(line + 1, "%d", &count) == 1) {
                for (int i = 0; i < count; ++i) {
                    if (fgets(line, sizeof(line), f)) {
                        int x, y;
                        if (sscanf(line, "%d %d", &x, &y) == 2) {
                            polygonPoints.push_back({x, y});
                        }
                    }
                }
            }
        }
    }

    fclose(f);
    InvalidateRect(hwnd, NULL, TRUE);
}
