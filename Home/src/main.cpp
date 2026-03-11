#include <graphics.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <vector>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

using namespace std;

class Mouse {
private:
    int mx, my, x, y;
public:

void sound(){
    PlaySound(TEXT("media/faah.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

    void update() {
		mx = mousex();
        my = mousey();

    }
    void chrosshair() {
        setcolor(WHITE);
        line(mx - 10, my, mx + 10, my);
        line(mx, my - 10, mx, my + 10);
    }

};

class Circle {
private:
    int x, y, radius, clickCount, speed, color, dx, dy;
    

public:
    
    Circle(const char* img ="") {
        radius = 25;
        clickCount = 0;
        speed = 1; 
		color = (rand() % 15) + 1; //ngerandom color
       

        x = rand() % (1280 - radius * 2) + radius;
        y = rand() % (720 - radius * 2) + radius;

        direction();
    }

    bool clicked(int mx, int my) {
        if (mx >= (x - radius) && mx <= (x + radius) &&
            my >= (y - radius) && my <= (y + radius)) {
            return true;
        }
        return false;
    }

    void draw() {
        setfillstyle(SOLID_FILL, color);
        setcolor(WHITE);
        fillellipse(x, y, radius, radius);
    }

    void direction() {
        do {
            dx = (rand() % 3) - 1;
            dy = (rand() % 3) - 1;
        } while (dx == 0 && dy == 0);
    }

    void move() {
        x = x + speed * dx;
        y = y + speed * dy;

        
        if (x > 1280) x = 0;
        if (x < 0) x = 1280;
        if (y > 720) y = 0;
        if (y < 0) y = 720;
    }

    void increaseStats() {
        clickCount++;
        speed++;
        x = rand() % (1280 - radius * 2) + radius;
        y = rand() % (720 - radius * 2) + radius;
		direction();
    }
};


int main() {
 
    initwindow(1280, 720, "ditotototototoototototototot");
    srand(time(0));

    vector<Circle> circles;
    circles.push_back(Circle());
	Mouse mouse;

    int totalClicks = 0;


    while (!kbhit()) {
        cleardevice();

        mouse.update();

        if (ismouseclick(WM_LBUTTONDOWN)) {
            int mx, my;
            getmouseclick(WM_LBUTTONDOWN, mx, my);

            bool hit = false;

            for (int i = 0; i < circles.size(); i++) {
                if (circles[i].clicked(mx, my)) {
                    circles[i].increaseStats();
                    totalClicks++;
                    mouse.sound();
                    hit = true;
                    break; 
                }
            }

            if (hit) {
                circles.push_back(Circle());
            }
        }

        
        for (int i = 0; i < circles.size(); i++) {
            circles[i].move();
            circles[i].draw();
        }

		mouse.chrosshair();

        setcolor(WHITE);
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
        char scoreText[50];
        sprintf(scoreText, "score: %d", totalClicks);
        outtextxy(10, 10, scoreText);

        delay(25);
    }

    getch();
    return 0;
}