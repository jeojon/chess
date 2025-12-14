//============== chess.h 头文件 ==============
#ifndef CHESS_H
#define CHESS_H

// ================= 包含必要的库 =================
#include <graphics.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>
#include <string.h>

// ================= 常量定义 =================
#define WIN_WIDTH   1000      // 窗口宽度
#define WIN_HEIGHT  600      // 窗口高度
#define BOARD_SIZE  8        // 棋盘大小 8x8
#define CELL_SIZE   64       // 每个格子的像素大小
#define BOARD_OFFSET_X 44    // 棋盘左上角X坐标
#define BOARD_OFFSET_Y 44    // 棋盘左上角Y坐标
//伪装视频播放所需的信息（视频拆分成帧图片，以此实现视频播放）
#define FRAME_PATH "chessimage/frame/%03d.png"  // 帧图片路径模板
#define FRAME_START 1                          // 起始帧编号
#define FRAME_END 100                          // 结束帧编号
#define FRAME_COUNT (FRAME_END - FRAME_START + 1)  // 总帧数（001-100）
#define FPS 30                                  // 播放帧率
#define DELAY_TIME (2450 / FPS)                 // 每帧延迟时间（毫秒）


// 玩家和棋子颜色
#define NONE 0
#define White 1
#define Black 2

// 棋子类型
#define EMPTY   0
#define KING    1
#define QUEEN   2
#define ROOK    3
#define BISHOP  4
#define KNIGHT  5
#define PAWN    6

// 游戏状态（我没有用到，不知道走法逻辑这一块有没有用到）
#define GAME_RUNNING     0
#define GAME_WHITE_WIN   1
#define GAME_BLACK_WIN   2
#define GAME_STALEMATE   3

// ================= 数据结构声明 =================
// 有优化空间：可以用short或者char，省内存

//页面状态
enum PageType {
    PAGE_START,
    PAGE_CHOOSE,
    PAGE_CHESS,
    PAGE_TEMP1,
    PAGE_TEMP2,
    PAGE_TEMP3,
    PAGE_TEMP4

};


//按钮
typedef struct Button{
    int x;
    int y;
    int w;
    int h;
    IMAGE* normalImg;
    IMAGE* highlightImg;
    PageType belongPage;
} Button;
//图片资源结构体
typedef struct Resource{                  // 定义具名结构体，原名Resource
    IMAGE startpage;
    IMAGE startbutton;       
    IMAGE startbuttonhighlight;
    IMAGE chesspage;
    IMAGE chessboard;
    IMAGE BishopB;
    IMAGE KingB;
    IMAGE KnightB;
    IMAGE PawnB;
    IMAGE QueenB;
    IMAGE RookB;
    IMAGE BishopW;
    IMAGE KingW;
    IMAGE KnightW;
    IMAGE PawnW;
    IMAGE QueenW;
    IMAGE RookW;
    IMAGE buttonloseup;
    IMAGE buttonlosedown;
    IMAGE buttondrawup;
    IMAGE buttondrawdown;
    IMAGE buttonlosehighlightup;
    IMAGE buttonlosehighlightdown;
    IMAGE buttondrawhighlightup;
    IMAGE buttondrawhighlightdown;
    IMAGE chooseside;
    IMAGE choosesideWN;
    IMAGE choosesideWH;
    IMAGE choosesideBN;
    IMAGE choosesideBH;
    IMAGE Bdrawbg;
    IMAGE Blosebg;
    IMAGE Wdrawbg;
    IMAGE Wlosebg;
    IMAGE drawacceptup;
    IMAGE drawrefuseup;
    IMAGE drawacceptdown;
    IMAGE drawrefusedown;
    IMAGE reset;
    IMAGE exit;
    IMAGE drawaccepthighlightup;
    IMAGE drawrefusehighlightup;
    IMAGE drawaccepthighlightdown;
    IMAGE drawrefusehighlightdown;
    IMAGE resethighlight;
    IMAGE exithighlight;
    IMAGE record;
    IMAGE Wkill;
    IMAGE Bkill;
} Resource;  // 类型别名Resource





// 棋子
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
extern int gameState;          // 游戏状态
extern int selectedX, selectedY;          // 选中的棋子位置
extern bool isPieceSelected;              // 是否有棋子被选中
extern int lastMoveFromX, lastMoveFromY;  // 上一步起始位置
extern int lastMoveToX, lastMoveToY;      // 上一步目标位置

extern Resource res;                      // 图片资源
extern PageType g_currentPage;            // 当前页面状态
extern bool g_isWhiteUp;                  // 视角标记变量

extern Button g_buttons[30];              // 按钮数组
extern int g_buttonCount;                 // 当前按钮个数
extern bool g_isHover[30];                // 按钮是否处于悬浮状态

// 记谱功能相关变量
extern int round_num; // 回合数
extern char notationRecords[100][20]; // 存储记谱记录
extern int notationCount; // 记谱数量
extern bool isWhiteMove; // 当前是否为白方走棋
// 滚动记谱功能相关变量
extern int scrollOffsetY; // 滚动偏移量
extern int totalTextHeight; // 文本总高度
extern bool needRedrawNotation; // 是否需要重绘记谱

// ================= 函数声明 =================

// 1. 初始化模块
void initBoard(void);          // 初始化棋盘（李颖萱的函数，以棋盘左下角为坐标原点）
void res_init(Resource* res);   //图片资源加载
void registerButton(int x, int y, int w, int h, IMAGE* normalImg, IMAGE* highlightImg, PageType belongPage) ;//按钮注册
IMAGE* getPieceImage(int type, int color); //棋子类型与颜色
//这个我不知道要不要，就先留下来了（我没有用）
void initNotation(void);       // 初始化棋谱记）

// 4. 动画模块
void initSplashAnimation();    // 初始化启动动画
void playSplashAnimation();    // 播放启动动画
void freeSplashAnimation();    // 释放启动动画资源

// 2. 绘制模块
void drawAlpha(IMAGE* picture, int picture_x, int picture_y); //实现透明贴图
void drawSelectedHighlight(int x, int y, bool isWhiteUp);    //棋子高亮
void drawPiecesWhiteUp();    //绘制棋子（白上黑下）
void drawPiecesWhiteDown();   //绘制棋子（白下黑上）
void redrawBoard(bool isWhiteUp);  // 重绘棋盘并添加选中高亮
void drawBasePage();         //绘制基础页面，分不同页面绘制背景和按钮



// 3. 游戏逻辑模块（我没有动这一块，因为我没用）

// 生成伪合法走法（不考虑将军）
MoveList generatePseudoMoves(int player);  // 返回MoveList

// 各棋子规则走法生成函数
void generateKingMoves(int x, int y, int player, MoveList *list);
void generateQueenMoves(int x, int y, int player, MoveList *list);
void generateRookMoves(int x, int y, int player, MoveList *list);
void generateBishopMoves(int x, int y, int player, MoveList *list);
void generateKnightMoves(int x, int y, int player, MoveList *list);
void generatePawnMoves(int x, int y, int player, MoveList *list);

// 生成合法走法（考虑王是否被将军）
MoveList generateLegalMoves(MoveList *PseudoList);

// 判断某方是否被将军
bool isInCheck(int KingColor);

// 执行一步走法（修改board）
int makeMove(int fromX, int fromY, int toX, int toY);

// 检查走法合法性（看是否在LegalList中）
int isValidMove(int fromX, int fromY, int toX, int toY);

// 切换玩家
void switchPlayer(void);

// 判断将死或逼和
int isCheckmate(void);
int isStalemate(void);

// 4. 坐标转换函数（这里是我用的）
int screenToBoardX(int screenX);  //坐标转换（不分视角）
int screenToBoardY(int screenY);  //坐标转换（白下黑上）
int screenToBoardYWhiteUp(int screenY);   // 白上黑下视角的Y坐标转换函数


// 5. 事件处理函数
void handleMouseClick(int boardX, int boardY);
bool isMouseOnImage(int mouseX, int mouseY, int imgX, int imgY, int imgW, int imgH); //判断鼠标指针是不是在指定区域
void handleBoardClick(int x, int y, bool isWhiteUp);   //处理棋盘点击（即棋子在棋盘上的移动和吃子画面）
bool handleButtonClick(int mouseX, int mouseY);       //对按钮点击事件的响应
// 6. 前端逻辑
void uiloop(); //实现前端所需的功能（不含开场视频）
void startvedio();   //实现前端所需功能（含开场视频）
#endif // CHESS_H