#include "raylib.h"
#include <iostream>

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

    // color array implementation :

    Game(int _SW, int _SH, unsigned int _maxIter)
    {
        SW = _SW;
        SH = _SH;
        maxIter = _maxIter;
        rTexture = LoadRenderTexture(SW, SH);
        isZoomSelected = false;
        zoomRatio = (Rect){-2, -2, 2, 2};
        scaledRatio.x = (zoomRatio.x - zoomRatio.width) / SW;
        scaledRatio.y = (zoomRatio.y - zoomRatio.height) / SH;
    }

    ~Game()
    {
        UnloadRenderTexture(rTexture);
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
            (9 * (1 - t) * t * t * t * 255),
            (15 * (1 - t) * (1 - t) * t * t * 255),
            (8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255),
            255};
    }

    Color juliaSet(long double x, long double y)
    {
        long double a = 4.0 * GetMousePosition().x / SW - 2;
        long double b = 4.0 * GetMousePosition().y / SH - 2;
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
            (3 * (1 - t) * t * t * t * 255),
            (16 * (1 - t) * (1 - t) * t * t * 255),
            (4 * (1 - t) * (1 - t) * (1 - t) * t * 255),
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

    void DrawOnRenderTexture()
    {
        BeginTextureMode(rTexture);
        for (int x = 0; x < SW; x++)
        {
            for (int y = 0; y < SH; y++)
            {
                // DrawPixel(x, y, juliaSet(4.0 * x / SW - 2, 4.0 * y / SH - 2));
                // DrawPixel(x, y, juliaSet(scaleRange(x, SW, 0, zoomRatio.x, zoomRatio.width), scaleRange(y, SH, 0, zoomRatio.y, zoomRatio.height)));
                DrawPixel(x, y, mandelbrot(x * scaledRatio.x + zoomRatio.width, y * scaledRatio.y + zoomRatio.height));
            }
        }
        EndTextureMode();
    }

    void Draw()
    {
        BeginDrawing();
        DrawTextureRec(rTexture.texture, (Rectangle){0, 0, (float)rTexture.texture.width, (float)-rTexture.texture.height}, (Vector2){0, 0}, WHITE);
        if (isZoomSelected)
        {
            DrawRectangleLines(zoomRec.x, zoomRec.y, zoomRec.width, zoomRec.height, WHITE);
            // DrawText(zoomRec.x, );
        }
        EndDrawing();
    }

    void Controls()
    {
        if (IsKeyReleased(KEY_R))
        {
            zoomRatio = (Rect){-2, -2, 2, 2};
            scaledRatio.x = (zoomRatio.x - zoomRatio.width) / SW;
            scaledRatio.y = (zoomRatio.y - zoomRatio.height) / SH;
            DrawOnRenderTexture();
        }
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            Vector2 mousePos = GetMousePosition();
            bool widthNegCalc = false, heightNegCalc = false;
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
                widthNegCalc = true;
            }
            if (mousePos.y < zoomRecOrigin.y)
            {
                zoomRec.height = zoomRecOrigin.y - mousePos.y;
                zoomRec.y = mousePos.y;
                heightNegCalc = true;
            }
            if (!widthNegCalc)
            {
                zoomRec.width = mousePos.x - zoomRecOrigin.x;
            }
            if (!heightNegCalc)
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

            DrawOnRenderTexture();
        }
    }
};

int main()
{
    int WIDTH = 1200;
    int HEIGHT = 800;
    unsigned int maxIter = 1000;

    InitWindow(WIDTH, HEIGHT, "Mandelbrotset");
    SetTargetFPS(60);
    Game game = {WIDTH, HEIGHT, maxIter};
    game.DrawOnRenderTexture();
    while (!WindowShouldClose())
    {

        game.Controls();
        game.Draw();
    }

    CloseWindow();

    return 0;
}