#include <iostream>
#include <cmath>
#include <string>
#include <deque>
#include <SDL.h>
#include <SDL_ttf.h>

using namespace std;

// визначення констант
const int SCREEN_WIDTH  = 960; // ширина екрану
const int SCREEN_HEIGHT = 720; // висота екрану
const int PADDING = SCREEN_WIDTH / 24; // відступ від краю екрану
const double MASS_MIN = 5.0; // мінімальна маса тіла
const double MASS_MAX = 10.0; // максимальна маса тіла
const double DEV_MIN = - 7.0; // мінімальне відхилення
const double DEV_MAX = 7.0; // максимальне відхилення
const double STIFFNESS_MIN = 20.0; // мінімальне значення коефіцієнту жорсткості
const double STIFFNESS_MAX = 80.0; // максимальне значення коефіцієнту жорсткості
const int L = SCREEN_HEIGHT / 4 - PADDING; // максимальне відхилення для цілей виведення на екран
const int HANG_POINT[] = {SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4 - 60}; // координати точки підвісу
const int GRAPH_WIDTH = SCREEN_WIDTH / 2 - PADDING * 4; // область графіка, де будується синусоїда


SDL_Window *initGraphics();
void tearDownGraphics(SDL_Window *window, SDL_Renderer *renderer);

double calcDeviation(double devInit, double mass, double stiff, double time);
double calcPeriod(double mass, double stiff);
double calcFrequency(double period);

void drawCircle(SDL_Renderer *renderer, double cx, double cy, double radius);
void drawPendulum(SDL_Renderer *renderer, double dev, double mass, double stiff);
void drawText(SDL_Renderer *renderer, TTF_Font *font, const char *text, SDL_Color textColor, int x, int y, const string alignX);


int main() {
    // ініціалізація графічної оболонки
    SDL_Window *window = initGraphics();
    if (window == nullptr) return EXIT_FAILURE;
    SDL_Renderer *renderer = SDL_GetRenderer(window);
    TTF_Font* font = TTF_OpenFont("../fonts/OpenSans.ttf", 32);
    TTF_Font* helpFont = TTF_OpenFont("../fonts/OpenSans.ttf", 20);
    if (font == nullptr || helpFont == nullptr) {
        cerr << "Error: font not found" << endl;
        TTF_Quit();
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_bool done = SDL_FALSE;
    SDL_Event event;
    Uint32 interval = 10;
    SDL_Color textColor = {255, 255, 255};
    SDL_Color activeTextColor = {255, 255, 0};
    SDL_Color currentColor;
    int time = 0;
    double devInit = -5.0; // початкове відхилення
    double dev = devInit;
    double mass = (MASS_MAX + MASS_MIN) / 2; // маса за замовчуванням
    double stiff = (STIFFNESS_MAX + STIFFNESS_MIN) / 2; // жорсткість за замовчуванням
    deque<double> lastPoints; // масив точок для побудови графіка
    int activeParam = 0; // індекс пункту меню

    double period = calcPeriod(mass, stiff);
    double frequency = calcFrequency(period);

    while (!done) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

        // Draw parameters
        char text[50];

        const char *helpText = "Використовуйте клавіші стрілок для зміни параметрів";
        drawText(renderer, helpFont, helpText, textColor, SCREEN_WIDTH / 2, 10, "center");

        sprintf(text, "Маса: %.1f кг", mass);
        currentColor = activeParam == 0 ? activeTextColor : textColor;
        drawText(renderer, font, text, currentColor, SCREEN_WIDTH / 6, PADDING, "center");

        sprintf(text, "Жорст.: %.1f Н/м", stiff);
        currentColor = activeParam == 1 ? activeTextColor : textColor;
        drawText(renderer, font, text, currentColor, SCREEN_WIDTH / 2, PADDING, "center");

        sprintf(text, "Поч. зсув: %.1f м", devInit);
        currentColor = activeParam == 2 ? activeTextColor : textColor;
        drawText(renderer, font, text, currentColor, SCREEN_WIDTH * 5 / 6, PADDING, "center");

        sprintf(text, "Час: %d с", time / 1000);
        drawText(renderer, font, text, textColor, SCREEN_WIDTH - PADDING, SCREEN_HEIGHT - PADDING - 80, "right");

        sprintf(text, "Період коливань: %.2f с", period);
        drawText(renderer, font, text, textColor, PADDING, SCREEN_HEIGHT - PADDING - 120, "left");

        sprintf(text, "Частота коливань: %.2f rad/с", frequency);
        drawText(renderer, font, text, textColor, PADDING, SCREEN_HEIGHT - PADDING - 80, "left");

        // малюємо маятник
        drawPendulum(renderer, dev, mass, stiff);

        int x0 = SCREEN_WIDTH / 2 + PADDING;
        // малюємо вісь часу для графіку
        SDL_RenderDrawLine(
                renderer,
                x0, SCREEN_HEIGHT / 2,
                SCREEN_WIDTH - PADDING, SCREEN_HEIGHT / 2
        );
        int lastSeconds;
        int lastSecondsPosition;
        if (time / interval < GRAPH_WIDTH) {
            lastSeconds = 3;
            lastSecondsPosition = x0 + lastSeconds * 100;
        } else {
            lastSeconds = time / 1000;
            lastSecondsPosition = x0 + GRAPH_WIDTH + (time % 2) / 10 - (time % 1000) / 10;
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

        drawText(renderer, helpFont, "Час, с", textColor, SCREEN_WIDTH - PADDING, SCREEN_HEIGHT / 2 - 40, "center");

        // малюємо вісь відхилення
        SDL_RenderDrawLine(
                renderer,
                SCREEN_WIDTH / 2 + PADDING, SCREEN_HEIGHT / 2 - L - PADDING,
                SCREEN_WIDTH / 2 + PADDING, SCREEN_HEIGHT / 2 + L + PADDING
        );

        drawText(renderer, helpFont, "Зсув, м", textColor, x0 - 10, SCREEN_HEIGHT / 2 - L - PADDING, "right");
        drawCircle(renderer, x0, SCREEN_HEIGHT / 2, 3);
        drawText(renderer, helpFont, "0", textColor, x0 - 10, SCREEN_HEIGHT / 2 + 5, "right");
        drawCircle(renderer, x0, SCREEN_HEIGHT / 2 - L, 3);
        drawText(renderer, helpFont, "7", textColor, x0 - 10, SCREEN_HEIGHT / 2 - L + 5, "right");
        drawCircle(renderer, x0, SCREEN_HEIGHT / 2 + L, 3);
        drawText(renderer, helpFont, "-7", textColor, x0 - 10, SCREEN_HEIGHT / 2 + L + 5, "right");

        // обчислюємо відхилення
        dev = calcDeviation(devInit, mass, stiff, time * 0.01 / interval);
        lastPoints.push_back(dev);
        if (lastPoints.size() > GRAPH_WIDTH) {
            lastPoints.pop_front();
        }

        // малюємо синусоїду коливань
        double graphStartX = SCREEN_WIDTH / 2 + PADDING;
        double graphstartY = SCREEN_HEIGHT / 2;
        double graphX = graphStartX;
        deque<double>::iterator it = lastPoints.begin();
        while (it != lastPoints.end()) {
            SDL_RenderDrawPoint(renderer, graphX++, graphstartY - L * (*it++) / DEV_MAX);
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
                            mass += 0.5;
                            time = 0;
                            period = calcPeriod(mass, stiff);
                            frequency = calcFrequency(period);
                            lastPoints.clear();
                        }
                    } else if (activeParam == 1) {
                        if (stiff < STIFFNESS_MAX) {
                            stiff += 5;
                            time = 0;
                            period = calcPeriod(mass, stiff);
                            frequency = calcFrequency(period);
                            lastPoints.clear();
                        }
                    } else if (activeParam == 2) {
                        if (devInit < DEV_MAX) {
                            devInit += 0.5;
                            time = 0;
                            lastPoints.clear();
                        }
                    }
                } else if (key == SDL_SCANCODE_DOWN) {
                    if (activeParam == 0) {
                        if (mass > MASS_MIN) {
                            mass -= 0.5;
                            time = 0;
                            period = calcPeriod(mass, stiff);
                            frequency = calcFrequency(period);
                            lastPoints.clear();
                        }
                    } else if (activeParam == 1) {
                        if (stiff > STIFFNESS_MIN) {
                            stiff -= 5;
                            time = 0;
                            period = calcPeriod(mass, stiff);
                            frequency = calcFrequency(period);
                            lastPoints.clear();
                        }
                    } else if (activeParam == 2) {
                        if (devInit > DEV_MIN) {
                            devInit -= 0.5;
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

    tearDownGraphics(window, renderer);
    return EXIT_SUCCESS;
}


void logSDLError(const string &msg){
    cerr << msg << " error: " << SDL_GetError() << endl;
}


SDL_Window *initGraphics() {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        logSDLError("SDL_Init");
        return nullptr;
    }

    if (TTF_Init() < 0) {
        logSDLError("TTF_Init");
        return nullptr;
    }

    SDL_Window *window = SDL_CreateWindow("Маятник", 100, 100, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr){
        logSDLError("CreateWindow");
        SDL_Quit();
        return nullptr;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr){
        logSDLError("CreateRenderer");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return nullptr;
    }

    return window;
}


void tearDownGraphics(SDL_Window *window, SDL_Renderer *renderer) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}


double calcDeviation(double devInit, double mass, double stiff, double time) {
    return devInit * cos(sqrt(stiff / mass) * time);
}


double calcPeriod(double mass, double stiff) {
    return 2 * M_PI * sqrt(mass / stiff);
}


double calcFrequency(double period) {
    return 2 * M_PI / period;
}


void drawCircle(SDL_Renderer *renderer, double cx, double cy, double radius) {
    double xpos, ypos;
    double angle_inc = 1.0f / radius;
    for (double angle = 0.0f; angle <= M_PI * 2; angle += angle_inc) {
        xpos = cx + radius * cos(angle);
        ypos = cy + radius * sin(angle);
        SDL_RenderDrawPoint(renderer, xpos, ypos);
    }
}


void drawPendulum(SDL_Renderer *renderer, double dev, double mass, double stiff) {
    int hangPointRadius = 5;
    // величина відхилення для виведення на екран
    double devDisplay = L * dev / DEV_MAX;
    double bodyCenter[] = {SCREEN_WIDTH / 4 , SCREEN_HEIGHT / 2 - devDisplay};
    double bodyRadius = mass * 3;
    // кінці пружини
    SDL_RenderDrawLine(
            renderer,
            HANG_POINT[0], HANG_POINT[1] + hangPointRadius,
            HANG_POINT[0], HANG_POINT[1] + hangPointRadius + 10
    );
    SDL_RenderDrawLine(
            renderer,
            bodyCenter[0], bodyCenter[1] - bodyRadius - 10,
            bodyCenter[0], bodyCenter[1] - bodyRadius
    );
    // пружна частина пружини
    // мінімальна довжина пружини у найбільш стиснутому стані
    double minSpringLength = SCREEN_HEIGHT / 2 - L - HANG_POINT[1] - hangPointRadius - bodyRadius - 20;
    int numSpirals = stiff * 0.33;
    double springLength = minSpringLength + (-devDisplay + L);
    double spiralLength = springLength / numSpirals;
    for (int i = 0; i < numSpirals; i++) {
        SDL_RenderDrawLine(
                renderer,
                HANG_POINT[0], HANG_POINT[1] + hangPointRadius + 10 + spiralLength * i,
                HANG_POINT[0] + 20, HANG_POINT[1] + hangPointRadius + 10 + spiralLength * (i + 0.25)
        );
        SDL_RenderDrawLine(
                renderer,
                HANG_POINT[0] + 20, HANG_POINT[1] + hangPointRadius + 10 + spiralLength * (i + 0.25),
                HANG_POINT[0], HANG_POINT[1] + hangPointRadius + 10 + spiralLength * (i + 0.5)
        );
        SDL_RenderDrawLine(
                renderer,
                HANG_POINT[0], HANG_POINT[1] + hangPointRadius + 10 + spiralLength * (i + 0.5),
                HANG_POINT[0] - 20, HANG_POINT[1] + hangPointRadius + 10 + spiralLength * (i + 0.75)
        );
        SDL_RenderDrawLine(
                renderer,
                HANG_POINT[0] - 20, HANG_POINT[1] + hangPointRadius + 10 + spiralLength * (i + 0.75),
                HANG_POINT[0], HANG_POINT[1] + hangPointRadius + 10 + spiralLength * (i + 1)
        );
    }
    // дві лінії стелі
    SDL_RenderDrawLine(
            renderer,
            HANG_POINT[0] - 50, HANG_POINT[1],
            HANG_POINT[0] - hangPointRadius, HANG_POINT[1]
    );
    SDL_RenderDrawLine(
            renderer,
            HANG_POINT[0] + hangPointRadius, HANG_POINT[1],
            HANG_POINT[0] + 50, HANG_POINT[1]
    );
    // точка підвісу
    drawCircle(renderer, HANG_POINT[0], HANG_POINT[1], hangPointRadius);
    // тіло
    drawCircle(renderer, bodyCenter[0], bodyCenter[1], bodyRadius);
}


void drawText(SDL_Renderer *renderer, TTF_Font *font, const char *text, SDL_Color textColor, int x, int y, const string alignX) {
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
    SDL_RenderCopy(renderer, Message, nullptr, &Message_rect);
    SDL_DestroyTexture(Message);
    SDL_FreeSurface(surfaceMessage);
}
