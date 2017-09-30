#include <iostream>
#include <unistd.h>
#include <string>
#include <termios.h>
#include <array>
#include <thread>
#include <mutex>
#include "mydef.hpp"

#define FIRST_KEYCODE 27
#define SECOND_KEYCODE 91
#define UP 65
#define DOWN 66
#define RIGHT 67
#define LEFT 68


std::mutex mtx;
int get_key_arrow_code();
void drawpoint(int x, int y);
void stream();
void movePoint();
void moveEnemy();
void collisionJudge();
void myTimer();
int x = 1;
int y = 1;
int enemy_x = WIDTH / 2;
int enemy_y = HEIGHT / 2;
int time_count = 0;
bool collision_wall = false;
int gameover = 0;

int main()
{
    try {
        std::thread thStream(stream);
        std::thread thMovePoint(movePoint);

        std::thread thMoveEnemy(moveEnemy);
        std::thread thCollosionJudge(collisionJudge);
        std::thread thMyTimer(myTimer);
        thStream.join();
        thMovePoint.join();
        thMoveEnemy.join();
        thCollosionJudge.join();
        thMyTimer.join();
        usleep(1000000);
    } catch (const std::exception& e) {
        std::cout << "Exception occured." << e.what() << std::endl;
    }
    return 0;
}

void stream()
{
    while (gameover == 0) {
        drawpoint(x, y);
        usleep(50000);
        std::cout << "\033[2J" << std::endl;
        if (gameover == 1) {
            std::cout << "\033[2J" << std::endl;
            std::cout << "\033[31m" << std::endl;
            std::cout << "Gameover." << std::endl;
            std::cout << "You couldn't escape from the Enemy." << std::endl;
            std::cout << "\033[39m" << std::endl;
        } else if (gameover == 2) {
            std::cout << "\033[2J" << std::endl;
            std::cout << "\033[36m" << std::endl;
            std::cout << "Congraturations!!!" << std::endl;
            std::cout << "You escaped from the Enemy!" << std::endl;
            std::cout << "game clear!" << std::endl;
            std::cout << "\033[39m" << std::endl;
        }
    }
}
void movePoint()
{
    std::array<int, 3> key_array = {0};
    while (gameover == 0) {
        int key = get_key_arrow_code();
        if (DEBUG) {
            std::cout << key << std::endl;
        }
        collision_wall = false;  //壁接触判定
        if (key == FIRST_KEYCODE) {
            key_array[0] = FIRST_KEYCODE;
            if (DEBUG) {
                std::cout << "1st step OK";
            }
        } else if (key == SECOND_KEYCODE) {
            key_array[1] = SECOND_KEYCODE;
            if (DEBUG) {
                std::cout << "2nd step OK";
            }
        } else if (key_array[0] == FIRST_KEYCODE && key_array[1] == SECOND_KEYCODE) {
            switch (key) {
            case UP:
                if (DEBUG) {
                    std::cout << "Up   ";
                }
                if (y + 1 <= HEIGHT) {
                    y++;
                } else {
                    collision_wall = true;
                }
                break;
            case DOWN:
                if (DEBUG) {
                    std::cout << "Down ";
                }
                if (y - 1 >= 1) {
                    y--;
                } else {
                    collision_wall = true;
                }
                break;
            case LEFT:
                if (DEBUG) {
                    std::cout << "Left ";
                }
                if (x - 1 >= 1) {
                    x--;
                } else {
                    collision_wall = true;
                }
                break;
            case RIGHT:
                if (DEBUG) {
                    std::cout << "Right";
                }
                if (x + 1 <= WIDTH) {
                    std::lock_guard<std::mutex> mutex_mtx(mtx);
                    x++;
                } else {
                    collision_wall = true;
                }
                break;
            default:
                if (DEBUG) {
                    std::cout << "Unknown keycode. " << FIRST_KEYCODE << " " << SECOND_KEYCODE << " " << key << std::endl;
                }
                break;
            }
            key_array[0] = 0;
            key_array[1] = 0;
        } else {
            key_array[0] = 0;
            key_array[1] = 0;
        }
    }
}

int get_key_arrow_code()
{
    int ch = 0;
    static struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    std::lock_guard<std::mutex> gameover_mtx(mtx);
    if (gameover == 0) {
        ch = getchar();
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}
void drawpoint(int x, int y)
{
    if (!DEBUG) {
        /* タイトル */
        std::cout << "\x1b[43m";
        std::cout << "\x1b[31m";
        std::cout << "Escape from Enemy!!";
        std::cout << "\x1b[49m";
        std::cout << "\x1b[39m";
        for (int i = 0; i < 3; i++) {
            std::cout << std::endl;
        }

        /* 上の辺の描画 */
        for (int i = 0; i < WIDTH + 1; i++) {
            std::cout << "#";
            for (int j = 0; j < WIDTH_INTERVAL - 1; j++) {
                std::cout << " ";
            }
        }
        // 最後の一個を出力して次の行へ
        std::cout << "#" << std::endl;
        for (int j = 0; j < HEIGHT_INTERVAL - 1; j++) {
            std::cout << std::endl;
        }

        /* 上から表示行までの行 */
        for (int i = 0; i < HEIGHT; i++) {
            std::cout << "#";
            for (int j = 0; j < (WIDTH + 1) * WIDTH_INTERVAL - 1; j++) {
                std::cout << " ";
            }
            std::cout << "#" << std::endl;
            for (int j = 0; j < HEIGHT_INTERVAL - 1; j++) {
                std::cout << std::endl;
            }
        }
        /* 下の辺の描画 */
        for (int i = 0; i < WIDTH + 1; i++) {
            std::cout << "#";
            for (int j = 0; j < WIDTH_INTERVAL - 1; j++) {
                std::cout << " ";
            }
        }
        std::cout << "#" << std::endl;
        for (int j = 0; j < HEIGHT_INTERVAL - 1; j++) {
            std::cout << std::endl;
        }
        // 状態の表示
        std::cout << "x: " << x << ", y: " << y << ", enemy_x: " << enemy_x << ", enemy_y: " << enemy_y << ", gameover: " << gameover << ", time_count" << CLEARTIME - time_count << std::endl;

        std::cout << "\e[" << (y + 1) * HEIGHT_INTERVAL + 1 << "A";
        std::cout << "\e[" << x * WIDTH_INTERVAL << "C";
        std::string str(". <- Me");
        if (collision_wall) {
            str += "  Ouch!! There's a wall against me!";
        }
        std::cout << str;
        std::cout << "\e[" << (y + 1) * HEIGHT_INTERVAL + 1 << "B";
        std::cout << "\e[" << x * WIDTH_INTERVAL + static_cast<int>(str.size()) << "D";
        std::cout << "\e[" << (enemy_y + 1) * HEIGHT_INTERVAL + 1 << "A";
        std::cout << "\e[" << enemy_x * WIDTH_INTERVAL << "C";
        std::string str_enemy(". <- Enemy");
        std::cout << str_enemy;
        std::cout << "\e[" << (enemy_y + 1) * HEIGHT_INTERVAL + 1 << "B";
        std::cout << "\e[" << enemy_x * WIDTH_INTERVAL << "D";
        std::cout << std::endl;
        if (x == enemy_x && y == enemy_y) {
            gameover = 1;
        }
        if (time_count >= CLEARTIME) {
            gameover = 2;
        }
    }
}

void moveEnemy()
{
    while (gameover == 0) {
        if (abs(enemy_x - x) >= abs(enemy_y - y)) {
            if (enemy_x > x) {
                enemy_x--;
            } else {
                enemy_x++;
            }
        } else {
            if (enemy_y > y) {
                enemy_y--;
            } else {
                enemy_y++;
            }
        }
        usleep(10000000 / SPEED);
    }
}
void collisionJudge()
{
    while (gameover == 0) {
        if (x == enemy_x && y == enemy_y) {
            std::lock_guard<std::mutex> gameover_mtx(mtx);
            gameover = 1;
        }
    }
}
void myTimer()
{
    while (gameover == 0) {
        usleep(1000000);
        time_count++;
        if (time_count >= CLEARTIME) {
            std::lock_guard<std::mutex> gameover_mtx(mtx);
            gameover = 2;
        }
    }
}
