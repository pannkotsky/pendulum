#include <iostream>
#include <cmath>
#include <deque>
#include <SDL.h>

#define PI 3.14159265

const int SCREEN_WIDTH  = 960;
const int SCREEN_HEIGHT = 720;
const int PADDING = SCREEN_WIDTH / 24;

const int L = SCREEN_WIDTH / 4 - PADDING * 2; // length of the pendulum

const double MASS_MIN = 5.0; // min mass of the body
const double MASS_MAX = 30.0; // max mass of the body

const double DEV_MIN = - PI / 2; // min deviation in radians
const double DEV_MAX = PI / 2; // max deviation in radians

const double STIFFNESS_MIN = 1.0;
const double STIFFNESS_MAX = 5.0;

const int HANG_POINT[] = {SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2};


void logSDLError(std::ostream &os, const std::string &msg);
void drawCircle(SDL_Renderer *renderer, double cx, double cy, double radius);
void drawPendulum(SDL_Renderer *renderer, double dev, double mass);


int main() {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
        logSDLError(std::cout, "SDL_Init");
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Pendulum", 100, 100, SCREEN_WIDTH,
                                          SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr){
        logSDLError(std::cout, "CreateWindow");
        SDL_Quit();
        return 1;
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
                                                SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr){
        logSDLError(std::cout, "CreateRenderer");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_bool done = SDL_FALSE;
    SDL_Event event;

    Uint32 interval = 10;
    int time = 0;
    double devInit = -1.0; // initial deviation
    double dev;
    double mass = (MASS_MAX + MASS_MIN) / 2;
    double stiff = (STIFFNESS_MAX + STIFFNESS_MIN) / 2;
    std::deque<double> lastPoints;

    while (!done) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

        // Divide screen in two halves
        SDL_RenderDrawLine(
                renderer,
                SCREEN_WIDTH / 2, 0,
                SCREEN_WIDTH / 2, SCREEN_HEIGHT
        );

        // Draw the time axis for graph
        SDL_RenderDrawLine(
                renderer,
                SCREEN_WIDTH / 2 + PADDING, SCREEN_HEIGHT / 2,
                SCREEN_WIDTH - PADDING, SCREEN_HEIGHT / 2
        );

        // Draw the deviation axis for graph
        SDL_RenderDrawLine(
                renderer,
                SCREEN_WIDTH / 2 + PADDING, SCREEN_HEIGHT / 2 - L - PADDING,
                SCREEN_WIDTH / 2 + PADDING, SCREEN_HEIGHT / 2 + L + PADDING
        );

        // Calculate deviation
        dev = devInit * cos(pow(stiff / mass, -0.5) * (time * 0.01 / interval));
        lastPoints.push_back(dev);
        if (lastPoints.size() > L * 2) {
            lastPoints.pop_front();
        }

        // Draw pendulum
        drawPendulum(renderer, dev, mass);

        // Draw the graph curve
        double graphStartX = SCREEN_WIDTH / 2 + PADDING;
        double graphstartY = SCREEN_HEIGHT / 2;
        double graphX = graphStartX;
        std::deque<double>::iterator it = lastPoints.begin();
        while (it != lastPoints.end()) {
            SDL_RenderDrawPoint(renderer, graphX++, graphstartY - L * sin(*it++));
        }

        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                done = SDL_TRUE;
            }
        }

        time += interval;
        SDL_Delay(interval);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}


/**
* Log an SDL error with some error message to the output stream of our choice
* @param os The output stream to write the message to
* @param msg The error message to write, format will be msg error: SDL_GetError()
*/
void logSDLError(std::ostream &os, const std::string &msg){
    os << msg << " error: " << SDL_GetError() << std::endl;
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


void drawPendulum(SDL_Renderer *renderer, double dev, double mass) {
    drawCircle(renderer, HANG_POINT[0], HANG_POINT[1], 2);
    double x = HANG_POINT[0] + L * sin(dev);
    double y = HANG_POINT[1] + L * cos(dev);
    double x1 = HANG_POINT[0] + (L - mass) * sin(dev);
    double y1 = HANG_POINT[1] + (L - mass) * cos(dev);
    SDL_RenderDrawLine(renderer, HANG_POINT[0], HANG_POINT[1], x1, y1);
    drawCircle(renderer, x, y, mass);
}
