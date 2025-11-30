/*
 Simple Tetris in C++ using graphics.h (WinBGIm)
 Controls:
  - Left arrow : move left
  - Right arrow: move right
  - Up arrow   : rotate
  - Down arrow : soft drop
  - Space      : hard drop
  - P          : pause

*/

#include <graphics.h>
#include <conio.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <vector>
#include <string>

using namespace std;

const int COLS = 10;
const int ROWS = 20;
const int BLOCK = 24; 
const int OFFSET_X = 50;
const int OFFSET_Y = 50;

int grid[ROWS][COLS];

struct Point { int r, c; };


int shapes[7][4][4][2] = {
    // I
    {
        {{0,0},{0,1},{0,2},{0,3}},
        {{0,1},{1,1},{2,1},{3,1}},
        {{1,0},{1,1},{1,2},{1,3}},
        {{0,2},{1,2},{2,2},{3,2}}
    },
    // J
    {
        {{0,0},{1,0},{1,1},{1,2}},
        {{0,1},{0,2},{1,1},{2,1}},
        {{0,0},{0,1},{0,2},{1,2}},
        {{0,1},{1,1},{2,1},{2,0}}
    },
    // L
    {
        {{0,2},{1,0},{1,1},{1,2}},
        {{0,1},{1,1},{2,1},{2,2}},
        {{0,0},{0,1},{0,2},{1,0}},
        {{0,0},{0,1},{1,1},{2,1}}
    },
    // O
    {
        {{0,0},{0,1},{1,0},{1,1}},
        {{0,0},{0,1},{1,0},{1,1}},
        {{0,0},{0,1},{1,0},{1,1}},
        {{0,0},{0,1},{1,0},{1,1}}
    },
    // S
    {
        {{0,1},{0,2},{1,0},{1,1}},
        {{0,1},{1,1},{1,2},{2,2}},
        {{1,1},{1,2},{2,0},{2,1}},
        {{0,0},{1,0},{1,1},{2,1}}
    },
    // T
    {
        {{0,1},{1,0},{1,1},{1,2}},
        {{0,1},{1,1},{1,2},{2,1}},
        {{1,0},{1,1},{1,2},{2,1}},
        {{0,1},{1,0},{1,1},{2,1}}
    },
    // Z
    {
        {{0,0},{0,1},{1,1},{1,2}},
        {{0,2},{1,1},{1,2},{2,1}},
        {{1,0},{1,1},{2,1},{2,2}},
        {{0,1},{1,0},{1,1},{2,0}}
    }
};

int colorsTet[8] = {0, LIGHTCYAN, LIGHTBLUE, LIGHTGREEN, YELLOW, LIGHTMAGENTA, LIGHTRED, LIGHTGRAY};

struct Tetromino {
    int type; // 0..6
    int rot;  // 0..3
    int r, c; // top-left anchor (row,col) - relative adjustments used when drawing
};

Tetromino cur;

void resetGrid() {
    for (int i=0;i<ROWS;i++) for (int j=0;j<COLS;j++) grid[i][j]=0;
}

void drawBlock(int r, int c, int color) {
    int x1 = OFFSET_X + c * BLOCK;
    int y1 = OFFSET_Y + r * BLOCK;
    int x2 = x1 + BLOCK - 1;
    int y2 = y1 + BLOCK - 1;
    setfillstyle(SOLID_FILL, color);
    bar(x1,y1,x2,y2);
    setcolor(BLACK);
    rectangle(x1,y1,x2,y2);
}

void drawField() {
    // background
    setcolor(WHITE);
    rectangle(OFFSET_X-1, OFFSET_Y-1, OFFSET_X + COLS*BLOCK, OFFSET_Y + ROWS*BLOCK);
    for (int r=0;r<ROWS;r++){
        for (int c=0;c<COLS;c++){
            if (grid[r][c]) drawBlock(r,c, grid[r][c]);
            else {
                // empty cell
                int x1 = OFFSET_X + c * BLOCK;
                int y1 = OFFSET_Y + r * BLOCK;
                int x2 = x1 + BLOCK - 1;
                int y2 = y1 + BLOCK - 1;
                setfillstyle(SOLID_FILL, BLACK);
                bar(x1,y1,x2,y2);
                setcolor(DARKGRAY);
                rectangle(x1,y1,x2,y2);
            }
        }
    }
}

void drawCurrent(const Tetromino &t, bool erase=false) {
    int col = t.type + 1; // use color index
    for (int k=0;k<4;k++){
        int pr = shapes[t.type][t.rot][k][0] + t.r;
        int pc = shapes[t.type][t.rot][k][1] + t.c;
        if (pr>=0 && pr<ROWS && pc>=0 && pc<COLS) {
            if (erase) {
                // draw empty
                int x1 = OFFSET_X + pc * BLOCK;
                int y1 = OFFSET_Y + pr * BLOCK;
                int x2 = x1 + BLOCK - 1;
                int y2 = y1 + BLOCK - 1;
                setfillstyle(SOLID_FILL, BLACK);
                bar(x1,y1,x2,y2);
                setcolor(DARKGRAY);
                rectangle(x1,y1,x2,y2);
            } else {
                drawBlock(pr,pc, colorsTet[col]);
            }
        }
    }
}

bool canPlace(const Tetromino &t) {
    for (int k=0;k<4;k++){
        int pr = shapes[t.type][t.rot][k][0] + t.r;
        int pc = shapes[t.type][t.rot][k][1] + t.c;
        if (pc<0 || pc>=COLS || pr>=ROWS) return false;
        if (pr>=0 && grid[pr][pc]) return false;
    }
    return true;
}

void placePiece(const Tetromino &t) {
    int col = t.type + 1;
    for (int k=0;k<4;k++){
        int pr = shapes[t.type][t.rot][k][0] + t.r;
        int pc = shapes[t.type][t.rot][k][1] + t.c;
        if (pr>=0 && pr<ROWS && pc>=0 && pc<COLS) grid[pr][pc] = colorsTet[col];
    }
}

int clearLines() {
    int cleared = 0;
    for (int r=ROWS-1;r>=0;r--) {
        bool full = true;
        for (int c=0;c<COLS;c++) if (!grid[r][c]) { full=false; break; }
        if (full) {
            cleared++;
            // move everything above down
            for (int rr=r; rr>0; rr--) for (int cc=0;cc<COLS;cc++) grid[rr][cc] = grid[rr-1][cc];
            for (int cc=0;cc<COLS;cc++) grid[0][cc]=0;
            r++; // recheck this row (now contains previous row)
        }
    }
    return cleared;
}

Tetromino nextPiece() {
    Tetromino t;
    t.type = rand()%7;
    t.rot = 0;
    t.r = -1; // spawn above
    t.c = COLS/2 - 2;
    return t;
}

void drawInfo(int score, int level) {
    int x = OFFSET_X + COLS*BLOCK + 20;
    int y = OFFSET_Y;
    setcolor(WHITE);
    char buf[100];
    sprintf(buf, "Score: %d", score);
    outtextxy(x,y,buf);
    sprintf(buf, "Level: %d", level);
    outtextxy(x,y+30,buf);
    outtextxy(x,y+60, "Controls:");
    outtextxy(x,y+80, "Arrows: Move/Rotate");
    outtextxy(x,y+100, "Space: Hard drop");
    outtextxy(x,y+120, "P: Pause");
}

int main(){
    srand((unsigned)time(NULL));
    int width = OFFSET_X*2 + COLS*BLOCK + 200;
    int height = OFFSET_Y*2 + ROWS*BLOCK;
    initwindow(width, height, "Tetris - graphics.h");

    resetGrid();
    Tetromino next = nextPiece();
    cur = nextPiece();
    if (!canPlace(cur)) {
        // game over immediately
        outtextxy(OFFSET_X, OFFSET_Y/2, "Game Over - cannot spawn piece");
        getch();
        closegraph();
        return 0;
    }

    int score = 0;
    int level = 1;
    int speed = 500; // ms
    bool running = true;
    bool paused = false;

    drawField();
    drawCurrent(cur);
    drawInfo(score, level);

    DWORD lastTick = GetTickCount();

    while (running) {
        if (!paused) {
            DWORD now = GetTickCount();
            if (now - lastTick >= (DWORD)speed) {
                // try move down
                Tetromino moved = cur;
                moved.r++;
                if (canPlace(moved)) {
                    drawCurrent(cur, true);
                    cur = moved;
                    drawCurrent(cur);
                } else {
                    // lock
                    placePiece(cur);
                    int lines = clearLines();
                    if (lines>0) {
                        score += (lines*100);
                        level = score/500 + 1;
                        speed = max(50, 500 - (level-1)*30);
                    }
                    // spawn next
                    cur = next;
                    next = nextPiece();
                    if (!canPlace(cur)) {
                        // game over
                        drawField();
                        outtextxy(OFFSET_X+20, OFFSET_Y+BLOCK*ROWS/2, "GAME OVER");
                        char buf[50]; sprintf(buf, "Final Score: %d", score);
                        outtextxy(OFFSET_X+20, OFFSET_Y+BLOCK*ROWS/2 + 20, buf);
                        running = false;
                    }
                }
                drawField();
                drawCurrent(cur);
                drawInfo(score, level);
                lastTick = now;
            }
        }

        // input handling
        if (kbhit()){
            int ch = getch();
            if (ch==0 || ch==224) {
                // arrow keys
                int code = getch();
                if (!paused && running) {
                    if (code==75) { // left
                        Tetromino moved = cur; moved.c--;
                        if (canPlace(moved)) {
                            drawCurrent(cur, true); cur = moved; drawCurrent(cur);
                        }
                    } else if (code==77) { // right
                        Tetromino moved = cur; moved.c++;
                        if (canPlace(moved)) {
                            drawCurrent(cur, true); cur = moved; drawCurrent(cur);
                        }
                    } else if (code==80) { // down
                        Tetromino moved = cur; moved.r++;
                        if (canPlace(moved)) {
                            drawCurrent(cur, true); cur = moved; drawCurrent(cur);
                        }
                    } else if (code==72) { // up - rotate
                        Tetromino moved = cur; moved.rot = (moved.rot+1)%4;
                        if (canPlace(moved)) {
                            drawCurrent(cur, true); cur = moved; drawCurrent(cur);
                        }
                    }
                    drawInfo(score, level);
                }
            } else {
                // normal keys
                if (ch=='p' || ch=='P') {
                    paused = !paused;
                    if (paused) outtextxy(OFFSET_X+20, OFFSET_Y+20, "PAUSED");
                    else drawField();
                } else if (ch==' ') {
                    // hard drop
                    if (!paused && running) {
                        Tetromino moved = cur;
                        while (true) {
                            Tetromino down = moved; down.r++;
                            if (canPlace(down)) moved = down; else break;
                        }
                        drawCurrent(cur, true);
                        cur = moved;
                        drawCurrent(cur);
                    }
                } else if (ch==27) { // ESC
                    running = false;
                }
            }
        }

        Sleep(10);
    }

   
    outtextxy(OFFSET_X+20, OFFSET_Y+BLOCK*ROWS/2 + 60, "Press any key to exit...");
    getch();
    closegraph();
    return 0;
}








