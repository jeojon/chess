//一、函数：在终端输出记谱内容
#include <stdio.h>
#include <string.h>

# include <graphics.h>
# include <easyx.h>
# include <windows.h>


// 玩家和棋子颜色
#define NONE 0
#define White 1
#define Black 2
#define BOARD_SIZE  8        // 棋盘大小 8x8
// 棋子类型
#define EMPTY   0
#define KING    1
#define QUEEN   2
#define ROOK    3
#define BISHOP  4
#define KNIGHT  5
#define PAWN    6

// 游戏状态
#define GAME_RUNNING     0
#define GAME_WHITE_WIN   1
#define GAME_BLACK_WIN   2
#define GAME_STALEMATE   3

# define true 1
# define false 0
// ================= 数据结构声明 =================

typedef struct {
    int type;      // 棋子类型
    int color;     // 棋子颜色: WHITE或BLACK
    bool moved;    // 是否移动过（用于王车易位判断）
} ChessPiece;

// 单步走法
typedef struct {
    int fromX, fromY;   // 起始格坐标
    int toX, toY;       // 目标格坐标
    int piecetype;      // 移动棋子的类型
    // 后期可加 capturedType, promotionType, isCastling 等
} Move;

// MoveList 用于存储多步走法生成列表
typedef struct {
    Move moves[256];    // 一般棋盘走法不会超过256步
    int count;          // 当前走法数量
} MoveList;

// ================= 全局变量 =================
extern ChessPiece board[BOARD_SIZE][BOARD_SIZE];  // 棋盘
extern int currentPlayer;      // 当前玩家

// 使用全局变量--记录当前回合数
int round_num = 1; // 避免与标准库函数round冲突

static int draw_record(char record[], int if_write)
                      
{
    if (if_write == 1)
    {
        // 白方走棋：输出回合号和棋步
        printf("%s", record);
        round_num++;
    }
    else
    {
        // 黑方走棋：只输出棋步，但会换行
        printf("  %s\n", record);
    }
    return 0;
}

int main()
{
    char record[10]; // 增大缓冲区确保安全

    // 第一回合：白方走棋
    strcpy_s(record, sizeof(record), "1.e4");
    draw_record(record, 1);

    // 第一回合：黑方走棋
    strcpy_s(record, sizeof(record), "e5");
    draw_record(record, 0);

    // 第二回合：白方走棋
    strcpy_s(record, sizeof(record), "2.Nf3");
    draw_record(record, 1);

    // 第二回合：黑方走棋
    strcpy_s(record, sizeof(record), "Nc6");
    draw_record(record, 0);

    return 0;
}

//二、函数：移动


// 执行走法（修改 board）
int makeMove(int fromX, int fromY, int toX, int toY)
{
    //移动棋子在棋盘上的broad坐标
    board[toX][toY] = board[fromX][fromY];
    board[toX][toY].moved = true;//需要吗？
    board[fromX][fromY].color = NULL;
    board[fromX][fromY].moved = NULL;
    board[fromX][fromY].type = NULL;
    //待解决：将数组上位置移动了，需要把原broad[fromX][fromY]位置上的棋子图片移动到broad to吗？


    //转变currentPlayer
    currentPlayer = (currentPlayer == 1) ? 2 : 1;

    //返回值有没有规范
    return 1;
}


int round = 1;//记录轮数

//每次switchplayer到白方时加1；
typedef struct {
    Move moves[256];    // 一般棋盘走法不会超过256步
    int count;          // 当前走法数量
} MoveList;
typedef struct {
    int fromX, fromY;   // 起始格坐标
    int toX, toY;       // 目标格坐标
    int piecetype;      // 移动棋子的类型
    // 后期可加 capturedType, promotionType, isCastling 等
} Move;

MoveList legallist;

// 棋谱记录（生成棋谱字符串）
//生成一个TXT，让用户可以看到下棋记录
int recordMove(int toX, int toY, ChessPiece piece, int fromX, int fromY)
{                                                 //如果按照要求要在走棋之前判断在不在list里面，那么from to 棋子的变量都要传进来，不过也可以在main中实现，看要求

    //总结：单独窗口实现记谱输出，记谱内容保存在文件中

    char piece_type;
    //1.棋子类型替换
    switch (piece.type)
    {
    case 1:piece_type = 'K'; break;
    case 2:piece_type = 'Q'; break;
    case 3:piece_type = 'R'; break;
    case 4:piece_type = 'B'; break;
    case 5:piece_type = 'N'; break;
    case 6:piece_type = ' '; break;
    }

    //2.棋子坐标记谱
    char toX_letter;

    switch (toX)
    {
    case 0:toX_letter = 'a'; break;
    case 1:toX_letter = 'b'; break;
    case 2:toX_letter = 'c'; break;
    case 3:toX_letter = 'd'; break;
    case 4:toX_letter = 'e'; break;
    case 5:toX_letter = 'f'; break;
    case 6:toX_letter = 'g'; break;
    case 7:toX_letter = 'h'; break;
    }
    //3.标记第几轮-使用全局变量

    //4.组合记谱到文件
    FILE* file;
    const char* filename = "data.txt";  // 文件名，可根据需要修改
    if (round != 1)
    {
        //情况1：非第一轮，以读写模式打开现有文件(r + 模式)
        file = fopen(filename, "r+");
        fseek(file, 0, SEEK_END);
    }
    else
    {
        //情况2: 第一次记谱操作，直接创建新文件
        file = fopen(filename, "w+");  // w+ 模式：创建新文件并允许读写
        if (file == NULL)
        {
            //perror("创建记谱文件失败");
            return 1;
        }
    }

    //步骤3：统一写入
    if (currentPlayer == White)
    {
        fprintf(file, "%d . %c%c%d", round, piece_type, toX_letter, toY);
    }
    else
    {
        fprintf(file, "%c%c%d \n", piece_type, toX_letter, toY);
    }

    // 步骤4: 关闭文件（无论新老文件）
    fclose(file);

    //5.输出文字-记录到新的页面
    char record[5];
    strcpy(record, &piece_type);
    strcat(record, &toX_letter);
    strcat(record, (char*)toY);


    //调用函数，给参数record(本次记谱内容) ;
    draw_record(record, currentPlayer);
    //暂不实现滑动页面功能
}


//三
# define TXT_SIZE 24
# define TXT_START_X (600+20)
# define TXT_START_Y 125

/*
函数名：roll_record_f(char* record，int n，int round)；
 record是数组名，储存了需要输出在页面上的字符
 n为record的长度
 round是游戏进行轮数

可使用的宏常量： TXT_START_X  TXT_START_Y 记谱区的左上角的位置
                HEIGHT  WIDTH  游戏页面的长宽
                TXT_WIDTH TXT_HEIGHT 记谱区的宽度、长度
功能实现：
从记谱区的左上角的位置，生成记谱区的宽度、长度的记谱区区域

在这一区域上保持字符可以由于鼠标滚轮滑动而实现记谱区区域上字符的滑动
*/

#include <cctype>   // 用于字符类型判断
#include <cstring>  // 用于strlen函数


#include <string>
#include <vector>
#include <algorithm>
#include <conio.h>
#include <cwchar>

using namespace std;

const int BOARD_OFFSET_X = 10;
const int WIN_WIDTH = 800;
//const int BOARD_SIZE = 100;
const int BOARD_OFFSET_Y = 20;
const int WIN_HEIGHT = 600;

// 宏常量定义
//const int TXT_START_X = 400;
//const int TXT_START_Y = 100;
const int HEIGHT = 600;
const int WIDTH = 800;
const int TXT_WIDTH = 200;
const int TXT_HEIGHT = 300;
const int LINE_SPACING = 25;

// 全局滚动状态
static int scrollOffsetY = 0;
static int totalTextHeight = 0;
static vector<wstring> recordLines;
static bool needRedraw = true;  // 添加重绘标志


// 滚动记录函数 - 负责处理棋谱记录、滚动和显示
static void roll_record_f(char* record, int n, int round) {
    // ========================= 第一部分：记录数据处理 =========================

    // 静态变量缓存上一次处理的状态（用于优化性能）
    static int lastRound = -1;      // 上一次处理的回合数（初始值-1表示未处理过）
    static char* lastRecord = NULL; // 上一次处理的记录字符串指针
    static int lastRecordLen = 0;   // 上一次处理的记录长度
    static wstring currentLine = L""; // 当前正在构建的行（确保开头是数字）

    // 检查记录是否需要重新处理（回合变化、记录指针变化或长度变化）
    if (lastRound != round || lastRecord != record || lastRecordLen != n)
    {
        // 标记需要重新绘制界面
        needRedraw = true;

        // 处理新记录
        wstring wstr = wstring(record, record + n);

        // 检查当前记录类型
        if (!wstr.empty() && isdigit(wstr[0])) {
            // 数字开头（白方记录）：开始新的一行
            if (!currentLine.empty()) {
                // 如果当前行不为空，先保存它
                recordLines.push_back(currentLine);
                currentLine.clear();
            }
            currentLine = wstr;
        }
        else if (!wstr.empty() && isalpha(wstr[0])) {
            // 字母开头（黑方记录）：添加到当前行，并添加3个空格
            if (!currentLine.empty()) {
                currentLine += L"           " + wstr;
                // 完成一行（白方+黑方），保存到记录行
                recordLines.push_back(currentLine);
                currentLine.clear();
            }
            else {
                // 没有白方记录，直接作为一行
                recordLines.push_back(L"     " + wstr);
            }
        }
        //else {
            // 其他情况直接添加到记录行
         //   if (!wstr.empty()) {
         //       recordLines.push_back(wstr);
         //   }
        //}

        // 添加当前回合信息（如果有记录或回合数大于0）
        if ((!recordLines.empty() || round > 0) && lastRound != round) {
            // 在列表开头插入回合标题（格式：===== 回合 X =====）
            recordLines.insert(recordLines.begin(), L"===== 回合 " + to_wstring(round) + L" =====");
        }

        // 更新缓存状态（记录当前处理参数）
        lastRound = round;     // 保存当前回合数
        lastRecord = record;   // 保存当前记录指针
        lastRecordLen = n;     // 保存当前记录长度
    }

    // ========================= 第二部分：滚动状态计算 =========================

    // 计算文本总高度（行数 × 行间距）
    totalTextHeight = (int)recordLines.size() * LINE_SPACING;

    // ========================= 第三部分：鼠标交互处理 =========================

    // 检查是否有鼠标事件
    if (MouseHit()) {
        // 获取鼠标消息
        MOUSEMSG msg = GetMouseMsg();

        // 检查鼠标是否在记谱区内（仅在此区域内响应滚轮）
        if (msg.x >= TXT_START_X && msg.x <= TXT_START_X + TXT_WIDTH &&
            msg.y >= TXT_START_Y && msg.y <= TXT_START_Y + TXT_HEIGHT) {

            // 检查是否为鼠标滚轮事件
            if (msg.uMsg == WM_MOUSEWHEEL) {
                // 计算滚轮移动量（上滚正，下滚负）
                int wheelDelta = (msg.wheel > 0) ? LINE_SPACING : -LINE_SPACING;
                // 计算新的滚动偏移
                int newOffset = scrollOffsetY + wheelDelta;

                // 边界检查（防止滚动超出内容范围）
                int maxScroll = 0;  // 最大滚动位置（顶部）
                // 最小滚动位置（底部）：内容高度 - 可视高度（负数）
                int minScroll = min(0, TXT_HEIGHT - totalTextHeight);
                // 确保新偏移在有效范围内
                newOffset = max(newOffset, minScroll);
                newOffset = min(newOffset, maxScroll);

                // 检查滚动位置是否实际发生变化
                if (newOffset != scrollOffsetY) {
                    // 更新滚动偏移
                    scrollOffsetY = newOffset;
                    // 标记需要重新绘制界面
                    needRedraw = true;
                }
            }
        }
    }

    // ========================= 第四部分：界面绘制 =========================

    // 检查是否需要重新绘制界面
    if (needRedraw) {
        // 使用双缓冲开始绘制（减少闪烁）
        BeginBatchDraw();

        // 4.1 绘制记谱区背景
        setfillcolor(0xF8F8F8); // 设置浅灰色背景色
        // 绘制实心矩形作为背景
        solidrectangle(TXT_START_X, TXT_START_Y,
            TXT_START_X + TXT_WIDTH,
            TXT_START_Y + TXT_HEIGHT);

        setlinecolor(0x404040); // 设置深灰色边框颜色
        // 绘制矩形边框
        rectangle(TXT_START_X, TXT_START_Y,
            TXT_START_X + TXT_WIDTH,
            TXT_START_Y + TXT_HEIGHT);

        // 4.2 设置裁剪区（仅记谱区内部有效）
        // 创建矩形裁剪区域
        HRGN hRgn = CreateRectRgn(TXT_START_X, TXT_START_Y,
            TXT_START_X + TXT_WIDTH,
            TXT_START_Y + TXT_HEIGHT);

        // 添加错误处理提示
        if (hRgn == NULL) {
            // 提示：区域创建失败，可能由于内存不足
            // 考虑添加错误处理逻辑或使用默认绘制
        }

        // 应用裁剪区域（后续绘制只在此区域内有效）
        setcliprgn(hRgn);

        // 4.3 绘制文本内容
        settextcolor(BLACK); // 设置默认文本颜色为黑色
        settextstyle(24, 0, L"微软雅黑"); // 设置默认文本样式（24号微软雅黑）

        // 遍历所有记录行
        for (int i = 0; i < recordLines.size(); i++) {
            // 计算当前行的垂直位置（考虑滚动偏移）
            int textY = TXT_START_Y + scrollOffsetY + i * LINE_SPACING;

            // 检查当前行是否在可见区域内
            if (textY + LINE_SPACING >= TXT_START_Y &&
                textY <= TXT_START_Y + TXT_HEIGHT) {

                // 特殊行样式：回合标题行
                if (recordLines[i].find(L"回合") != wstring::npos) {
                    settextcolor(0x800000); // 设置深红色
                    settextstyle(24, 0, L"微软雅黑"); // 确保字体样式一致
                }
                else {
                    // 普通行恢复默认样式
                    settextcolor(BLACK);
                    settextstyle(24, 0, L"微软雅黑");
                }

                // 在指定位置绘制文本（X位置+10像素偏移）
                outtextxy(TXT_START_X + 10, textY, recordLines[i].c_str());
            }
        }



        // 4.4 绘制滚动条（如果需要）
        if (totalTextHeight > TXT_HEIGHT) {
            // 绘制滚动条轨道（背景）
            setfillcolor(0xD0D0D0); // 浅灰色
            solidrectangle(TXT_START_X + TXT_WIDTH - 12, TXT_START_Y,
                TXT_START_X + TXT_WIDTH, TXT_START_Y + TXT_HEIGHT);

            // 计算滑块位置和大小
            float ratio = (float)-scrollOffsetY / (totalTextHeight - TXT_HEIGHT);
            int barHeight = max(30, (int)(TXT_HEIGHT * TXT_HEIGHT / totalTextHeight));
            int barY = TXT_START_Y + (int)((TXT_HEIGHT - barHeight) * ratio);

            // 绘制滚动滑块
            setfillcolor(0x606060); // 深灰色
            solidrectangle(TXT_START_X + TXT_WIDTH - 10, barY,
                TXT_START_X + TXT_WIDTH - 2, barY + barHeight);
        }

        // 4.5 恢复裁剪区
        DeleteObject(hRgn);
        setcliprgn(NULL);

        // 4.6 绘制标题
        settextcolor(0x000080); // 深蓝色
        settextstyle(30, 0, L"微软雅黑");
        outtextxy(TXT_START_X, TXT_START_Y - 30, L"游戏记谱区");

        // 结束双缓冲绘制
        EndBatchDraw();

        needRedraw = false;  // 重置重绘标志
    }
}

//这个main函数适用于单独测试记谱页面实现的函数的
int main()
{
    initgraph(WIDTH, HEIGHT);
    setbkmode(TRANSPARENT);

    int round = 1;
    char chessRecord1[] = "1.e1";
    char chessRecord2[] = "f3";
    char chessRecord3[] = "2.j3";
    char chessRecord4[] = "g4";

    // 主循环
    //while (!_kbhit()) {
        // 第一次调用
    roll_record_f(chessRecord1, strlen(chessRecord1), round);

    // 第二次调用
    roll_record_f(chessRecord2, strlen(chessRecord2), round);

    // 第三次调用
    roll_record_f(chessRecord3, strlen(chessRecord3), round);

    // 第四次调用
    roll_record_f(chessRecord4, strlen(chessRecord4), round);

    // 处理绘制
    if (needRedraw) {
        BeginBatchDraw();
        cleardevice();
        // 触发重绘
        roll_record_f(nullptr, 0, round);
        EndBatchDraw();
        needRedraw = false;
    }

    Sleep(30000);
    //}

    closegraph();
    return 0;
}

//四、鼠标
#define PIC_SIZE 64
#define START_X 44
#define START_Y 44

struct Location {
    int row;
    int col;
};
// 解析坐标，实现鼠标位置获取、并且转化为二维数组board中的位置
Location parsePos(const char* str)
{
    Location piece_location;
    int x;
    int y;
    //鼠标位置获取
    while (1)
    {
        MOUSEMSG msg = GetMouseMsg();
        if (msg.uMsg == WM_LBUTTONDOWN)
        {
            // 将鼠标坐标转换为棋盘坐标--从左上开始的--只为中间量temp，不是实际记谱坐标，但是不输出可以这么记录
            x = (msg.x - START_X) / PIC_SIZE; //左上开始，0-7
            y = (msg.y - START_Y) / PIC_SIZE;

            // 确保获取的点击是在在棋盘范围内
            if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE)
            {
                break;
            }
        }
    }
    //横着的是x轴
    piece_location.row = x;
    piece_location.col = y;
    return piece_location;
}