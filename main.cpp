#include "chess.h"
#include <algorithm>

// 全局变量声明
extern Resource res;
extern PageType g_currentPage;
extern bool g_isWhiteUp;

int main() {
    // 1. 初始化EasyX图形窗口
    initgraph(WIN_WIDTH, WIN_HEIGHT);
    
    // 2. 初始化资源
    res_init(&res);
    
    // 2.1 初始化并播放启动动画
    initSplashAnimation();
    playSplashAnimation();
    
    // 动画播放完毕后重新绘制当前页面，防止界面空白
    drawBasePage();
    FlushBatchDraw();
    
    // 3. 注册所有按钮
    registerButton(385, 225, 200, 80, &res.startbutton, &res.startbuttonhighlight, PAGE_START);
    registerButton(350, 312, 125, 50, &res.choosesideWN, &res.choosesideWH, PAGE_CHOOSE);
    registerButton(525, 312, 125, 50, &res.choosesideBN, &res.choosesideBH, PAGE_CHOOSE);
    registerButton(600, 25, 135, 75, &res.buttondrawup, &res.buttondrawhighlightup, PAGE_CHESS);
    registerButton(815, 25, 135, 75, &res.buttonloseup, &res.buttonlosehighlightup, PAGE_CHESS);
    registerButton(600, 500, 135, 75, &res.buttondrawdown, &res.buttondrawhighlightdown, PAGE_CHESS);
    registerButton(815, 500, 135, 75, &res.buttonlosedown, &res.buttonlosehighlightdown, PAGE_CHESS);
    registerButton(325, 345, 150, 50, &res.drawacceptup, &res.drawaccepthighlightup, PAGE_TEMP1);
    registerButton(325, 345, 150, 50, &res.drawacceptdown, &res.drawaccepthighlightdown, PAGE_TEMP2);
    registerButton(525, 345, 150, 50, &res.drawrefuseup, &res.drawrefusehighlightup, PAGE_TEMP1);
    registerButton(525, 345, 150, 50, &res.drawrefusedown, &res.drawrefusehighlightdown, PAGE_TEMP2);
    registerButton(325, 345, 150, 50, &res.reset, &res.resethighlight, PAGE_TEMP3);
    registerButton(525, 345, 150, 50, &res.exit, &res.exithighlight, PAGE_TEMP3);
    
    // 4. 初始化页面
    drawBasePage();
    FlushBatchDraw();
    
    ExMessage msg;
    
    // 5. 主游戏循环
    while (true) {
        // 监听鼠标消息
        if (peekmessage(&msg, EX_MOUSE)) {
            switch (msg.message) {
                case WM_MOUSEMOVE:
                    // 处理按钮悬浮效果
                    for (int i = 0; i < g_buttonCount; i++) {
                        Button* btn = &g_buttons[i];
                        if (btn->belongPage != g_currentPage) continue;
                        
                        bool currentHover = isMouseOnImage(msg.x, msg.y, btn->x, btn->y, btn->w, btn->h);
                        if (currentHover != g_isHover[i]) {
                            if (currentHover) {
                                drawAlpha(btn->highlightImg, btn->x, btn->y);
                            } else {
                                drawAlpha(btn->normalImg, btn->x, btn->y);
                            }
                            g_isHover[i] = currentHover;
                            FlushBatchDraw(btn->x, btn->y, btn->w, btn->h);
                        }
                    }
                    break;
                    
                case WM_LBUTTONDOWN:
                    // 处理按钮点击
                    if (handleButtonClick(msg.x, msg.y)) {
                        FlushBatchDraw();
                        break;
                    }
                    
                    // 处理棋盘点击
                    if (g_currentPage == PAGE_CHESS) {
                        int boardX = screenToBoardX(msg.x);
                        int boardY;
                        if (g_isWhiteUp) {
                            boardY = screenToBoardYWhiteUp(msg.y);
                        } else {
                            boardY = screenToBoardY(msg.y);
                        }
                        handleBoardClick(boardX, boardY, g_isWhiteUp);
                        FlushBatchDraw();
                    }
                    break;
                    
                case WM_MOUSEWHEEL:
                    // 处理鼠标滚轮事件，用于记谱区滚动
                    if (g_currentPage == PAGE_CHESS) {
                        extern int scrollOffsetY;
                        extern int totalTextHeight;
                        extern bool needRedrawNotation;
                        
                        // 记谱区的位置和大小
                        const int MAIN_TXT_START_X = 600 + 20;
                        const int MAIN_TXT_START_Y = 125;
                        const int MAIN_TXT_WIDTH = 310;
                        const int MAIN_TXT_HEIGHT = 310;
                        const int MAIN_LINE_SPACING = 25;
                        
                        // 检查鼠标是否在记谱区内
                        if (msg.x >= MAIN_TXT_START_X && msg.x <= MAIN_TXT_START_X + MAIN_TXT_WIDTH &&
                            msg.y >= MAIN_TXT_START_Y && msg.y <= MAIN_TXT_START_Y + MAIN_TXT_HEIGHT) {
                            
                            // 计算滚轮移动量
                            int wheelDelta = (msg.wheel > 0) ? MAIN_LINE_SPACING : -MAIN_LINE_SPACING;
                            int newOffset = scrollOffsetY + wheelDelta;
                            
                            // 边界检查
                            int maxScroll = 0;  // 最大滚动位置（顶部）
                            int minScroll = std::min(0, MAIN_TXT_HEIGHT - totalTextHeight);  // 最小滚动位置（底部）
                            
                            // 确保新偏移在有效范围内
                            newOffset = std::max(newOffset, minScroll);
                            newOffset = std::min(newOffset, maxScroll);
                            
                            // 如果滚动位置有变化，更新并触发重绘
                            if (newOffset != scrollOffsetY) {
                                scrollOffsetY = newOffset;
                                needRedrawNotation = true;
                                
                                // 重绘棋盘和记谱
                                redrawBoard(g_isWhiteUp);
                                FlushBatchDraw();
                            }
                        }
                    }
                    break;
            }
        }
        
        // 检查游戏结束条件
        // 移除PAGE_START页面的自动退出条件，防止程序过早结束
        if (_kbhit() && getch() == 27) { // 仅当按下ESC键时退出
            break;
        }
    }
    
    // 6. 清理资源
    closegraph();
    return 0;
}