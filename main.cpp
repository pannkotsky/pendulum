#include <iostream>
#include <cmath>
#include <string>
#include <deque>
#include <SDL.h>
#include <SDL_ttf.h>

using namespace std;

#define PI 3.14159265

const int SCREEN_WIDTH  = 960;
const int SCREEN_HEIGHT = 720;
const int PADDING = SCREEN_WIDTH / 24;

const int L = SCREEN_WIDTH / 4 - PADDING * 2; // length of the pendulum

const double MASS_MIN = 10.0; // min mass of the body
const double MASS_MAX = 30.0; // max mass of the body

const double DEV_MIN = - PI / 2; // min deviation in radians
const double DEV_MAX = PI / 2; // max deviation in radians

const double STIFFNESS_MIN = 1.0;
const double STIFFNESS_MAX = 5.0;

const int HANG_POINT[] = {SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2};


void logSDLError(const string &msg);
void drawCircle(SDL_Renderer *renderer, double cx, double cy, double radius);
void drawPendulum(SDL_Renderer *renderer, double dev, double mass, double stiff);
void drawText(SDL_Renderer *renderer, TTF_Font *font, char *text, SDL_Color textColor, int x, int y, string alignX);


int main() {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        logSDLError("SDL_Init");
        return 1;
    }

    if (TTF_Init() < 0) {
        logSDLError("TTF_Init");
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Маятник", 100, 100, SCREEN_WIDTH,
                                          SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr){
        logSDLError("CreateWindow");
        SDL_Quit();
        return 1;
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
                                                SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr){
        logSDLError("CreateRenderer");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_bool done = SDL_FALSE;
    SDL_Event event;

    Uint32 interval = 10;
    TTF_Font* font = TTF_OpenFont("../fonts/OpenSans.ttf", 32);
    TTF_Font* helpFont = TTF_OpenFont("../fonts/OpenSans.ttf", 20);
    if (font == nullptr || helpFont == nullptr) {
        cerr << "Error: font not found" << endl;
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    SDL_Color textColor = {255, 255, 255};
    SDL_Color activeTextColor = {255, 255, 0};
    SDL_Color currentColor;

    int time = 0;
    double devInit = -1.0; // initial deviation
    double dev;
    double mass = (MASS_MAX + MASS_MIN) / 2;
    double stiff = (STIFFNESS_MAX + STIFFNESS_MIN) / 2;
    deque<double> lastPoints;
    int activeParam = 0;

    while (!done) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

        // Draw parameters
        char text[50];
        sprintf(text, "Маса: %.1f", mass);
        currentColor = activeParam == 0 ? activeTextColor : textColor;
        drawText(renderer, font, text, currentColor, SCREEN_WIDTH / 6, PADDING, "center");

        sprintf(text, "Жорсткість: %.1f", stiff);
        currentColor = activeParam == 1 ? activeTextColor : textColor;
        drawText(renderer, font, text, currentColor, SCREEN_WIDTH / 2, PADDING, "center");

        sprintf(text, "Поч. зсув: %.1f", devInit);
        currentColor = activeParam == 2 ? activeTextColor : textColor;
        drawText(renderer, font, text, currentColor, SCREEN_WIDTH * 5 / 6, PADDING, "center");

        char *helpText = "Використовуйте клавіші стрілок для зміни параметрів";
        drawText(renderer, helpFont, helpText, textColor, SCREEN_WIDTH / 2, 100, "center");

        sprintf(text, "Час: %d", time / 1000);
        drawText(renderer, font, text, textColor, SCREEN_WIDTH / 2, SCREEN_HEIGHT - PADDING - 80, "center");

        // Draw pendulum
        drawPendulum(renderer, dev, mass, stiff);

        int x0 = SCREEN_WIDTH / 2 + PADDING;
        // Draw the time axis for graph
        SDL_RenderDrawLine(
                renderer,
                x0, SCREEN_HEIGHT / 2,
                SCREEN_WIDTH - PADDING, SCREEN_HEIGHT / 2
        );
        int lastSeconds;
        int lastSecondsPosition;
        if (time / interval < L * 2) {
            lastSeconds = 3;
            lastSecondsPosition = x0 + lastSeconds * 100;
        } else {
            lastSeconds = time / 1000;
            lastSecondsPosition = x0 + L * 2 + (time % 2) / 10 - (time % 1000) / 10;
        }

        if (lastSecondsPosition + 100 <= SCREEN_WIDTH - PADDING) {
            drawCircle(renderer, lastSecondsPosition + 100, SCREEN_HEIGHT / 2, 3);
            sprintf(text, "%d", lastSeconds + 1);
            drawText(renderer, helpFont, text, textColor, lastSecondsPosition + 100, SCREEN_HEIGHT / 2 + 5, "center");
        }

        drawCircle(renderer, lastSecondsPosition, SCREEN_HEIGHT / 2, 3);
        sprintf(text, "%d", lastSeconds);
        drawText(renderer, helpFont, text, textColor, lastSecondsPosition, SCREEN_HEIGHT / 2 + 5, "center");

        drawCircle(renderer, lastSecondsPosition - 100, SCREEN_HEIGHT / 2, 3);
        sprintf(text, "%d", lastSeconds - 1);
        drawText(renderer, helpFont, text, textColor, lastSecondsPosition - 100, SCREEN_HEIGHT / 2 + 5, "center");

        drawCircle(renderer, lastSecondsPosition - 200, SCREEN_HEIGHT / 2, 3);
        sprintf(text, "%d", lastSeconds - 2);
        drawText(renderer, helpFont, text, textColor, lastSecondsPosition - 200, SCREEN_HEIGHT / 2 + 5, "center");

        drawText(renderer, helpFont, "Час", textColor, SCREEN_WIDTH - PADDING, SCREEN_HEIGHT / 2 - 40, "center");

        // Draw the deviation axis for graph
        SDL_RenderDrawLine(
                renderer,
                SCREEN_WIDTH / 2 + PADDING, SCREEN_HEIGHT / 2 - L - PADDING,
                SCREEN_WIDTH / 2 + PADDING, SCREEN_HEIGHT / 2 + L + PADDING
        );

        drawText(renderer, helpFont, "Зсув", textColor, x0 - 10, SCREEN_HEIGHT / 2 - L - PADDING, "right");
        drawCircle(renderer, x0, SCREEN_HEIGHT / 2, 3);
        drawText(renderer, helpFont, "0", textColor, x0 - 10, SCREEN_HEIGHT / 2 + 5, "right");
        drawCircle(renderer, x0, SCREEN_HEIGHT / 2 - L, 3);
        drawText(renderer, helpFont, "π/2", textColor, x0 - 10, SCREEN_HEIGHT / 2 - L + 5, "right");
        drawCircle(renderer, x0, SCREEN_HEIGHT / 2 + L, 3);
        drawText(renderer, helpFont, "-π/2", textColor, x0 - 10, SCREEN_HEIGHT / 2 + L + 5, "right");

        // Calculate deviation
        dev = devInit * cos(pow(stiff / mass, -0.5) * (time * 0.01 / interval));
        lastPoints.push_back(dev);
        if (lastPoints.size() > L * 2) {
            lastPoints.pop_front();
        }

        // Draw the graph curve
        double graphStartX = SCREEN_WIDTH / 2 + PADDING;
        double graphstartY = SCREEN_HEIGHT / 2;
        double graphX = graphStartX;
        deque<double>::iterator it = lastPoints.begin();
        while (it != lastPoints.end()) {
            SDL_RenderDrawPoint(renderer, graphX++, graphstartY - L * sin(*it++));
        }

        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&event)) {
            int key = event.key.keysym.scancode;
            if (event.type == SDL_QUIT || key == SDL_SCANCODE_ESCAPE) {
                done = SDL_TRUE;
            } else if (event.type == SDL_KEYUP) {
                if (key == SDL_SCANCODE_LEFT) {
                    activeParam = (activeParam + 2) % 3;
                } else if (key == SDL_SCANCODE_RIGHT) {
                    activeParam = (activeParam + 1) % 3;
                } else if (key == SDL_SCANCODE_UP) {
                    if (activeParam == 0) {
                        if (mass < MASS_MAX) {
                            mass++;
                            time = 0;
                            lastPoints.clear();
                        }
                    } else if (activeParam == 1) {
                        if (stiff < STIFFNESS_MAX) {
                            stiff += 0.5;
                            time = 0;
                            lastPoints.clear();
                        }
                    } else if (activeParam == 2) {
                        if (devInit + 0.1 < DEV_MAX) {
                            devInit += 0.1;
                            time = 0;
                            lastPoints.clear();
                        }
                    }
                } else if (key == SDL_SCANCODE_DOWN) {
                    if (activeParam == 0) {
                        if (mass > MASS_MIN) {
                            mass--;
                            time = 0;
                            lastPoints.clear();
                        }
                    } else if (activeParam == 1) {
                        if (stiff > STIFFNESS_MIN) {
                            stiff -= 0.5;
                            time = 0;
                            lastPoints.clear();
                        }
                    } else if (activeParam == 2) {
                        if (devInit - 0.1 > DEV_MIN) {
                            devInit -= 0.1;
                            time = 0;
                            lastPoints.clear();
                        }
                    }
                }
            }
        }

        time += interval;
        SDL_Delay(interval);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}


/**
* Log an SDL error with some error message to the output stream of our choice
* @param os The output stream to write the message to
* @param msg The error message to write, format will be msg error: SDL_GetError()
*/
void logSDLError(const string &msg){
    cerr << msg << " error: " << SDL_GetError() << endl;
}


void drawCircle(SDL_Renderer *renderer, double cx, double cy, double radius) {
    double xpos, ypos;
    double angle_inc = 1.0f / radius;
    for (double angle = 0.0f; angle <= PI * 2; angle += angle_inc) {
        xpos = cx + radius * cos(angle);
        ypos = cy + radius * sin(angle);
        SDL_RenderDrawPoint(renderer, xpos, ypos);
    }
}


void drawPendulum(SDL_Renderer *renderer, double dev, double mass, double stiff) {
    SDL_RenderDrawLine(
            renderer,
            SCREEN_WIDTH / 4 - 50, SCREEN_HEIGHT / 2,
            SCREEN_WIDTH / 4 - stiff * 2, SCREEN_HEIGHT / 2
    );
    SDL_RenderDrawLine(
            renderer,
            SCREEN_WIDTH / 4 + stiff * 2, SCREEN_HEIGHT / 2,
            SCREEN_WIDTH / 4 + 50, SCREEN_HEIGHT / 2
    );
    double x = HANG_POINT[0] + L * sin(dev);
    double y = HANG_POINT[1] + L * cos(dev);
    double x1 = HANG_POINT[0] + (L - mass) * sin(dev);
    double y1 = HANG_POINT[1] + (L - mass) * cos(dev);
    double x0 = HANG_POINT[0] + stiff * 2 * sin(dev);
    double y0 = HANG_POINT[1] + stiff * 2 * cos(dev);
    SDL_RenderDrawLine(renderer, x0, y0, x1, y1);
    drawCircle(renderer, HANG_POINT[0], HANG_POINT[1], stiff * 2);
    drawCircle(renderer, x, y, mass);
}


void drawText(SDL_Renderer *renderer, TTF_Font *font, char *text, SDL_Color textColor, int x, int y, string alignX) {
    SDL_Surface* surfaceMessage = TTF_RenderUTF8_Blended(font, text, textColor);
    SDL_Texture* Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
    SDL_Rect Message_rect;
    int textWidth, textHeigth;
    TTF_SizeUTF8(font, text, &textWidth, &textHeigth);
    int rectX = alignX == "center" ? x - textWidth / 2 : alignX == "right" ? x - textWidth : x;
    Message_rect.x = rectX;
    Message_rect.y = y;
    Message_rect.w = textWidth;
    Message_rect.h = textHeigth;
    SDL_RenderCopy(renderer, Message, NULL, &Message_rect);
    SDL_DestroyTexture(Message);
    SDL_FreeSurface(surfaceMessage);
}
