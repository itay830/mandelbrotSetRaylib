#include "raylib.h"
#include <iostream>
#include <thread>
#include <chrono>

using std::thread;

struct Rect
{
    long double x;
    long double y;
    long double width;
    long double height;
};
struct Vec2
{
    long double x;
    long double y;
};
// TODO: implement SW*SH*4 color array to reduce function call stack amounts (one draw funtion);
// TODO: use all CPU cores
class Game
{
public:
    unsigned int SW;
    unsigned int SH;
    unsigned int maxIter;
    Rect zoomRec;
    Rect zoomRatio;
    Vec2 zoomRecOrigin;
    Vec2 scaledRatio;
    bool isZoomSelected;
    RenderTexture2D rTexture;

    int numOfThreads;

    // smooth zooming
    Vec2 smoothVec;

    // 2d color array implementation :
    Color **colorArr;

    Game(int _SW, int _SH, unsigned int _maxIter, int _numOfThreads)
    {
        SW = _SW;
        SH = _SH;
        maxIter = _maxIter;
        numOfThreads = _numOfThreads;

        rTexture = LoadRenderTexture(SW, SH);
        isZoomSelected = false;
        zoomRatio = (Rect){-2, -2, 2, 2};
        scaledRatio.x = (zoomRatio.x - zoomRatio.width) / SW;
        scaledRatio.y = (zoomRatio.y - zoomRatio.height) / SH;

        smoothVec = (Vec2){0.9, 0.9};

        // 2d color array implementation :
        colorArr = new Color *[SH];
        for (int i = 0; i < SH; i++)
        {
            colorArr[i] = new Color[SW];
            for (int j = 0; j < SW; j++)
            {
                colorArr[i][j] = Color{255, 255, 255, 255};
            }
        }
    }

    ~Game()
    {
        UnloadRenderTexture(rTexture);

        // 2d color array implementation :
        for (int i = 0; i < SH; i++)
        {
            delete[] colorArr[i];
        }
        delete[] colorArr;
    }

    Color mandelbrot(long double a, long double b)
    {
        long double x0 = a, y0 = b, tempX0;
        unsigned int iterCount = 0;
        while (iterCount < maxIter && x0 * x0 + y0 * y0 <= 4)
        {
            tempX0 = x0;
            x0 = x0 * x0 - y0 * y0 + a;
            y0 = 2 * tempX0 * y0 + b;
            iterCount += 1;
        }
        // return (Color){iterCount * 255 / maxIter, iterCount * 255 / maxIter, iterCount * 255 / maxIter, 255};
        long double t = (double)iterCount / maxIter;
        // std::cout << 9 * (1 - t) * t * t * t << "\n";
        return (Color){
            (30 * (1 - t) * (1 - t) * t * t * t * 255),
            (16 * (1 - t) * t * t * 255),
            (40 * (1 - t) * (1 - t) * t * t * t * 255),
            255};
    }

    Color juliaSet(long double x, long double y)
    {
        long double a = GetMousePosition().x * scaledRatio.x + zoomRatio.width;
        long double b = GetMousePosition().y * scaledRatio.y + zoomRatio.height;
        long double tempX;
        unsigned int iterCount = 0;
        while (x * x + y * y <= 4 && iterCount < maxIter)
        {
            tempX = x;
            x = x * x - y * y + a;
            y = 2 * tempX * y + b;
            iterCount += 1;
        }
        // return (Color){iterCount * 255 / maxIter, iterCount * 255 / maxIter, iterCount * 255 / maxIter, 255};
        long double t = (long double)iterCount / maxIter;
        return (Color){
            (12 * (1 - t) * t * t * t * 255),
            (16 * (1 - t) * (1 - t) * t * t * 255),
            (8 * (1 - t) * (1 - t) * (1 - t) * t * 255),
            255};
    }

    Color getColor(unsigned int iterCount)
    {
        return (Color){iterCount * 255 / maxIter, iterCount * 255 / maxIter, iterCount * 255 / maxIter, 255};
    }

    long double scaleRange(long double old_value, long double old_max, long double old_min, long double new_max, long double new_min)
    {
        return (((old_value - old_min) * (new_max - new_min)) / (old_max - old_min)) + new_min;
    }

    void setColorMap(int startX, int endX, int startY, int endY)
    {

        for (int x = startX; x < endX; x++)
        {
            for (int y = startY; y < endY; y++)
            {
                // DrawPixel(x, y, juliaSet(4.0 * x / SW - 2, 4.0 * y / SH - 2));
                // DrawPixel(x, y, juliaSet(scaleRange(x, SW, 0, zoomRatio.x, zoomRatio.width), scaleRange(y, SH, 0, zoomRatio.y, zoomRatio.height)));
                // DrawPixel(x, y, mandelbrot(x * scaledRatio.x + zoomRatio.width, y * scaledRatio.y + zoomRatio.height));
                colorArr[y][x] = mandelbrot(x * scaledRatio.x + zoomRatio.width, y * scaledRatio.y + zoomRatio.height);
            }
        }
    }

    void colorMapToTexture()
    {
        BeginTextureMode(rTexture);
        for (int y = 0; y < SH; y++)
        {
            for (int x = 0; x < SW; x++)
            {
                DrawPixel(x, y, colorArr[y][x]);
            }
        }
        EndTextureMode();
    }

    void Draw()
    {

        BeginDrawing();
        ClearBackground(BLACK);
        DrawTextureRec(rTexture.texture, (Rectangle){0, 0, (float)rTexture.texture.width, (float)-rTexture.texture.height}, (Vector2){0, 0}, WHITE);
        if (isZoomSelected)
        {
            DrawRectangleLines(zoomRec.x, zoomRec.y, zoomRec.width, zoomRec.height, WHITE);
            // DrawText(zoomRec.x, );
        }
        EndDrawing();
    }

    bool Controls()
    {
        bool draw = false;
        if (IsKeyReleased(KEY_R))
        {
            zoomRatio = (Rect){-2, -2, 2, 2};
            scaledRatio.x = (zoomRatio.x - zoomRatio.width) / SW;
            scaledRatio.y = (zoomRatio.y - zoomRatio.height) / SH;
            // DrawOnRenderTexture(0, SW, 0, SH);
            draw = true;
        }
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            Vector2 mousePos = GetMousePosition();
            if (!isZoomSelected)
            {
                zoomRecOrigin = (Vec2){mousePos.x, mousePos.y};
                zoomRec.x = mousePos.x;
                zoomRec.y = mousePos.y;
                isZoomSelected = true;
            }
            if (mousePos.x < zoomRecOrigin.x)
            {
                zoomRec.width = zoomRecOrigin.x - mousePos.x;
                zoomRec.x = mousePos.x;
            }
            else
            {
                zoomRec.width = mousePos.x - zoomRecOrigin.x;
            }
            if (mousePos.y < zoomRecOrigin.y)
            {
                zoomRec.height = zoomRecOrigin.y - mousePos.y;
                zoomRec.y = mousePos.y;
            }
            else
            {
                zoomRec.height = mousePos.y - zoomRecOrigin.y;
            }
        }
        else if (isZoomSelected)
        {
            isZoomSelected = false;
            Rect zoomRatioTemp = (Rect){zoomRatio.x, zoomRatio.y, zoomRatio.width, zoomRatio.height};
            zoomRatio = (Rect){
                scaleRange(zoomRec.x + zoomRec.width, SW, 0, zoomRatioTemp.x, zoomRatioTemp.width),   // zoomRec.x
                scaleRange(zoomRec.y + zoomRec.height, SH, 0, zoomRatioTemp.y, zoomRatioTemp.height), // zoomRec.y
                scaleRange(zoomRec.x, SW, 0, zoomRatioTemp.x, zoomRatioTemp.width),                   // zoomRec.x + zoomRec.width
                scaleRange(zoomRec.y, SH, 0, zoomRatioTemp.y, zoomRatioTemp.height)};                 // zoomRec.y + zoomRec.height

            scaledRatio.x = (zoomRatio.x - zoomRatio.width) / SW;
            scaledRatio.y = (zoomRatio.y - zoomRatio.height) / SH;
            // maxIter *= 1.25;
            // DrawOnRenderTexture(0, SW, 0, SH);
            draw = true;
        }
        return draw;
    }

    void SmoothZoom()
    {
        zoomRatio.x *= smoothVec.x;
        zoomRatio.y *= smoothVec.y;
        zoomRatio.width *= smoothVec.x;
        zoomRatio.height *= smoothVec.y;

        scaledRatio.x = (zoomRatio.x - zoomRatio.width) / SW;
        scaledRatio.y = (zoomRatio.y - zoomRatio.height) / SH;

        // DrawOnRenderTexture(0, SW, 0, SH);
    }
};

void threadDraw(Game &game, int numThread)
{
    // auto startTime = std::chrono::high_resolution_clock::now();
    thread threads[numThread];
    for (int i = 1; i <= numThread; i++)
    {
        threads[i - 1] = std::thread(&Game::setColorMap, &game, 0, game.SW, game.SH * (i - 1) / numThread, game.SH * i / numThread);
    }

    for (int i = 0; i < numThread; i++)
    {
        threads[i].join();
    }
    game.colorMapToTexture();
    // auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime);
    // std::cout << dur.count() << "\n";
}

int main()
{
    int WIDTH = 1200;
    int HEIGHT = 900;
    unsigned int maxIter = 2500;

    InitWindow(WIDTH, HEIGHT, "Mandelbrotset");
    SetTargetFPS(60);
    Game game = {WIDTH, HEIGHT, maxIter, 8};
    threadDraw(game, game.numOfThreads);

    while (!WindowShouldClose())
    {

        if (game.Controls())
        {
            threadDraw(game, game.numOfThreads);
        }

        // game.SmoothZoom();
        game.Draw();
    }

    CloseWindow();
    return 0;
}