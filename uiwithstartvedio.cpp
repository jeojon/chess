
#include "chess.h"
#include <vector>
#include <string>
#include <cctype>
Button g_buttons[30];     //按钮个数
int g_buttonCount = 0;    //当前按钮个数
PageType g_currentPage = PAGE_START;   //当前页面状态（初始）
bool g_isHover[30] = {false};       //按钮是否处于悬浮状态
Resource res;          
bool g_isWhiteUp = false;// 添加视角标记变量，用于区分白上黑下和白下黑上视角
ChessPiece board[BOARD_SIZE][BOARD_SIZE];
int currentPlayer;
int gameState;
int selectedX, selectedY;
int lastMoveFromX, lastMoveFromY;
int lastMoveToX, lastMoveToY;
bool isPieceSelected = false;

// 记谱功能相关变量
int round_num = 1; // 回合数
char notationRecords[100][20]; // 存储记谱记录
int notationCount = 0; // 记谱数量
bool isWhiteMove = true; // 当前是否为白方走棋

// 滚动记谱功能相关变量
#define UI_TXT_START_X (600+20)
#define UI_TXT_START_Y 125
#define UI_TXT_WIDTH 310
#define UI_TXT_HEIGHT 310
#define UI_LINE_SPACING 25

// 全局滚动状态
int scrollOffsetY = 0;
int totalTextHeight = 0;
static std::vector<std::string> recordLines;
bool needRedrawNotation = true;  // 添加重绘标志



// 将棋盘坐标转换为国际象棋记谱坐标（a1-h8）
char getFileChar(int x) {
    return 'a' + x;
}

int getRank(int y, bool isWhiteUp) {
    if (isWhiteUp) {
        return y + 1;
    } else {
        return BOARD_SIZE - y;
    }
}

// 生成棋步的记谱字符串
void generateNotation(Move move, char* notation) {
    char pieceType = ' ';
    switch (move.piecetype) {
        case KING: pieceType = 'K'; break;
        case QUEEN: pieceType = 'Q'; break;
        case ROOK: pieceType = 'R'; break;
        case BISHOP: pieceType = 'B'; break;
        case KNIGHT: pieceType = 'N'; break;
        case PAWN: pieceType = ' '; break; // 兵的记谱不需要显示棋子类型
        default: pieceType = '?';
    }
    
    char fromFile = getFileChar(move.fromX);
    int fromRank = getRank(move.fromY, g_isWhiteUp);
    char toFile = getFileChar(move.toX);
    int toRank = getRank(move.toY, g_isWhiteUp);
    
    if (move.piecetype == PAWN) {
        // 兵的记谱格式：目标位置（如果吃子则显示起始列）
        sprintf(notation, "%c%d", toFile, toRank);
    } else {
        // 其他棋子的记谱格式：棋子类型 + 目标位置
        sprintf(notation, "%c%c%d", pieceType, toFile, toRank);
    }
}

// 将记谱添加到记录列表并保存到文件
void addNotationRecord(Move move) {
    char notation[20];
    generateNotation(move, notation);
    
    if (isWhiteMove) {
        // 白方走棋，添加回合号
        sprintf(notationRecords[notationCount], "%d.%s", round_num, notation);
    } else {
        // 黑方走棋
        sprintf(notationRecords[notationCount], "%s", notation);
        round_num++;
    }
    
    notationCount++;
    isWhiteMove = !isWhiteMove;
    
    // 保存记谱到文件
    FILE* file;
    const char* filename = "data.txt";
    
    if (notationCount == 1) {
        // 第一次记谱，创建新文件
        file = fopen(filename, "w");
    } else {
        // 后续记谱，追加到文件
        file = fopen(filename, "a");
    }
    
    if (file != NULL) {
        if (isWhiteMove) {
            // 上一步是白方走棋，黑方要走了
            fprintf(file, "%d. %s  ", round_num - 1, notationRecords[notationCount - 1]);
        } else {
            // 上一步是黑方走棋，白方要走了
            fprintf(file, "%s\n", notationRecords[notationCount - 1]);
        }
        fclose(file);
    }
    
    // 更新滚动记谱界面
    needRedrawNotation = true;
}

// 更新滚动记谱列表
void updateRecordLines() {
    recordLines.clear();
    
    for (int i = 0; i < notationCount; i += 2) {
        std::string line = "";
        
        // 添加回合号
        int currentRound = i / 2 + 1;
        line += std::to_string(currentRound) + ". ";
        
        // 添加白方走棋
        line += std::string(notationRecords[i], notationRecords[i] + strlen(notationRecords[i])) + "  ";
        
        // 添加黑方走棋（如果有的话）
        if (i + 1 < notationCount) {
            line += std::string(notationRecords[i + 1], notationRecords[i + 1] + strlen(notationRecords[i + 1]));
        }
        
        recordLines.push_back(line);
    }
    
    // 计算文本总高度
    totalTextHeight = (int)recordLines.size() * UI_LINE_SPACING;
}

// 绘制滚动记谱界面
void drawNotation() {
    // 如果需要重绘，更新记录列表
    if (needRedrawNotation) {
        updateRecordLines();
        needRedrawNotation = false;
    }
    
    // 设置记谱区的位置和大小
    int notationX = UI_TXT_START_X;
    int notationY = UI_TXT_START_Y;
    int notationWidth = UI_TXT_WIDTH;
    int notationHeight = UI_TXT_HEIGHT;
    
    // 绘制记谱区背景
    setfillcolor(0xF8F8F8);
    solidrectangle(notationX, notationY, notationX + notationWidth, notationY + notationHeight);
    
    // 绘制记谱区边框
    setlinecolor(0x404040);
    rectangle(notationX, notationY, notationX + notationWidth, notationY + notationHeight);
    
    // 创建裁剪区域
    HRGN hRgn = CreateRectRgn(notationX, notationY, notationX + notationWidth, notationY + notationHeight);
    setcliprgn(hRgn);
    
    // 设置文本样式
    settextcolor(BLACK);
    setbkcolor(TRANSPARENT);
    settextstyle(24, 0, "微软雅黑");
    
    // 绘制记谱内容
    for (int i = 0; i < recordLines.size(); i++) {
        int textY = notationY + scrollOffsetY + i * UI_LINE_SPACING;
        
        // 检查当前行是否在可见区域内
        if (textY + UI_LINE_SPACING >= notationY && textY <= notationY + notationHeight) {
            outtextxy(notationX + 10, textY, recordLines[i].c_str());
        }
    }
    
    // 恢复裁剪区域
    DeleteObject(hRgn);
    setcliprgn(NULL);
    
    // 绘制滚动条（如果需要）
    if (totalTextHeight > notationHeight) {
        // 绘制滚动条轨道
        setfillcolor(0xD0D0D0);
        solidrectangle(notationX + notationWidth - 12, notationY,
                      notationX + notationWidth, notationY + notationHeight);
        
        // 计算滑块位置和大小
        float ratio = (float)-scrollOffsetY / (totalTextHeight - notationHeight);
        int barHeight = std::max(30, (int)(notationHeight * notationHeight / totalTextHeight));
        int barY = notationY + (int)((notationHeight - barHeight) * ratio);
        
        // 绘制滚动滑块
        setfillcolor(0x606060);
        solidrectangle(notationX + notationWidth - 10, barY,
                      notationX + notationWidth - 2, barY + barHeight);
    }
    
    // 绘制标题
    settextcolor(0x000080);
    settextstyle(30, 0, "微软雅黑");
    outtextxy(notationX, notationY - 30, "游戏记谱区");
}

void res_init(Resource* res){  // 参数类型改为Resource*（原struct Resource*，简化写法）加载图片
    loadimage(&res->startpage,"chessimage/startpage.png",1000,600);
    loadimage(&res->startbutton,"chessimage/startbutton.png",200,80);
    loadimage(&res->startbuttonhighlight,"chessimage/startbuttonhighlight.png",200,80);
    loadimage(&res->chessboard,"chessimage/chessboard.png",512,512);
    loadimage(&res->chesspage,"chessimage/chesspage.png",1000,600);
    loadimage(&res->record,"chessimage/record.png",350,350);
    loadimage(&res->BishopB,"chessimage/BishopB.png",64,64);
    loadimage(&res->BishopW,"chessimage/BishopW.png",64,64);
    loadimage(&res->KingB,"chessimage/KingB.png",64,64);
    loadimage(&res->KingW,"chessimage/KingW.png",64,64);
    loadimage(&res->PawnB,"chessimage/PawnB.png",64,64);
    loadimage(&res->PawnW,"chessimage/PawnW.png",64,64);
    loadimage(&res->KnightB,"chessimage/KnightB.png",64,64);
    loadimage(&res->KnightW,"chessimage/KnightW.png",64,64);
    loadimage(&res->QueenB,"chessimage/QueenB.png",64,64);
    loadimage(&res->QueenW,"chessimage/QueenW.png",64,64);
    loadimage(&res->RookB,"chessimage/RookB.png",64,64);
    loadimage(&res->RookW,"chessimage/RookW.png",64,64);
    loadimage(&res->buttonloseup,"chessimage/buttonlose.png",135,75);
    loadimage(&res->buttonlosedown,"chessimage/buttonlose.png",135,75);
    loadimage(&res->buttonlosehighlightup,"chessimage/buttonlosehighlight.png",135,75);
    loadimage(&res->buttonlosehighlightdown,"chessimage/buttonlosehighlight.png",135,75);
    loadimage(&res->buttondrawup,"chessimage/buttondraw.png",135,75);
    loadimage(&res->buttondrawhighlightup,"chessimage/buttondrawhighlight.png",135,75);
    loadimage(&res->buttondrawdown,"chessimage/buttondraw.png",135,75);
    loadimage(&res->buttondrawhighlightdown,"chessimage/buttondrawhighlight.png",135,75);
    loadimage(&res->chooseside,"chessimage/chooseside.png",400,175);
    loadimage(&res->choosesideWN,"chessimage/choosesideWN.png",125,50);
    loadimage(&res->choosesideWH,"chessimage/choosesideWH.png",125,50);
    loadimage(&res->choosesideBN,"chessimage/choosesideBN.png",125,50);
    loadimage(&res->choosesideBH,"chessimage/choosesideBH.png",125,50);
    loadimage(&res->Bdrawbg,"chessimage/Bdrawbg.png",400,300);  //黑方求和弹窗
    loadimage(&res->Blosebg,"chessimage/Blosebg.png",400,300);  //黑方认输弹窗
    loadimage(&res->Wdrawbg,"chessimage/Wdrawbg.png",400,300);   //白方求和弹窗
    loadimage(&res->Wlosebg,"chessimage/Wlosebg.png",400,300);  //白方认输弹窗
    loadimage(&res->drawacceptup,"chessimage/drawaccept.png",150,50);  //接受
    loadimage(&res->drawrefuseup,"chessimage/drawrefuse.png",150,50);   //拒绝
    loadimage(&res->drawacceptdown,"chessimage/drawaccept.png",150,50);  //接受
    loadimage(&res->drawrefusedown,"chessimage/drawrefuse.png",150,50);   //拒绝
    loadimage(&res->reset,"chessimage/reset.png",150,50);    //重开一局
    loadimage(&res->exit,"chessimage/exit.png",150,50);   //退出游戏
    loadimage(&res->drawaccepthighlightup,"chessimage/drawaccepthighlight.png",150,50);  //接受高亮
    loadimage(&res->drawrefusehighlightup,"chessimage/drawrefusehighlight.png",150,50);   //拒绝高亮
    loadimage(&res->drawaccepthighlightdown,"chessimage/drawaccepthighlight.png",150,50);  //接受高亮
    loadimage(&res->drawrefusehighlightdown,"chessimage/drawrefusehighlight.png",150,50);   //拒绝高亮
    loadimage(&res->resethighlight,"chessimage/resethighlight.png",150,50);    //重开一局高亮
    loadimage(&res->exithighlight,"chessimage/exithighlight.png",150,50);   //退出游戏高亮
    loadimage(&res->Wkill,"chessimage/Wkill.png",400,300);   //白方胜利图片
    loadimage(&res->Bkill,"chessimage/Bkill.png",400,300);   //黑方胜利图片
}

void drawAlpha(IMAGE* picture, int picture_x, int picture_y) {     //实现透明贴图
    DWORD *dst = GetImageBuffer();
    DWORD *draw = GetImageBuffer();
    DWORD *src = GetImageBuffer(picture);
    int picture_width = picture->getwidth();
    int picture_height = picture->getheight();
    int graphWidth = getwidth();
    int graphHeight = getheight();
    int dstX = 0;

    for (int iy = 0; iy < picture_height; iy++)
    {
        for (int ix = 0; ix < picture_width; ix++)
        {
            int srcX = ix + iy * picture_width;
            int sa = ((src[srcX] & 0xff000000) >> 24);
            int sr = ((src[srcX] & 0xff0000) >> 16);
            int sg = ((src[srcX] & 0xff00) >> 8);
            int sb = src[srcX] & 0xff;
            if (ix >= 0 && ix <= graphWidth && iy >= 0 && iy <= graphHeight && dstX <= graphWidth * graphHeight)
            {
                dstX = (ix + picture_x) + (iy + picture_y) * graphWidth;
                int dr = ((dst[dstX] & 0xff0000) >> 16);
                int dg = ((dst[dstX] & 0xff00) >> 8);
                int db = dst[dstX] & 0xff;
                draw[dstX] = ((sr * sa / 255 + dr * (255 - sa) / 255) << 16)
                    | ((sg * sa / 255 + dg * (255 - sa) / 255) << 8)
                    | (sb * sa / 255 + db * (255 - sa) / 255);
            }
        }
    }
}

bool isMouseOnImage(int mouseX, int mouseY, int imgX, int imgY, int imgW, int imgH) {  //判断鼠标位置
    return (mouseX >= imgX && mouseX <= imgX + imgW &&
            mouseY >= imgY && mouseY <= imgY + imgH);
}

void registerButton(int x, int y, int w, int h, IMAGE* normalImg, IMAGE* highlightImg, PageType belongPage) {  //按钮注册，便于后续调用
    if (g_buttonCount >= 15) return;
    g_buttons[g_buttonCount].x = x;
    g_buttons[g_buttonCount].y = y;
    g_buttons[g_buttonCount].w = w;
    g_buttons[g_buttonCount].h = h;
    g_buttons[g_buttonCount].normalImg = normalImg;
    g_buttons[g_buttonCount].highlightImg = highlightImg;
    g_buttons[g_buttonCount].belongPage = belongPage;
    g_isHover[g_buttonCount] = false;
    g_buttonCount++;
}

IMAGE* getPieceImage(int type, int color) {      //棋子图片，加入颜色判断便于一次性呈现在棋盘上
    if (color == White) {
        switch (type) {
            case KING:    return &res.KingW;
            case QUEEN:   return &res.QueenW;
            case ROOK:    return &res.RookW;
            case BISHOP:  return &res.BishopW;
            case KNIGHT:  return &res.KnightW;
            case PAWN:    return &res.PawnW;
            default:      return NULL;
        }
    } else if (color == Black) {
        switch (type) {
            case KING:    return &res.KingB;
            case QUEEN:   return &res.QueenB;
            case ROOK:    return &res.RookB;
            case BISHOP:  return &res.BishopB;
            case KNIGHT:  return &res.KnightB;
            case PAWN:    return &res.PawnB;
            default:      return NULL;
        }
    }
    return NULL;
}
//李颖萱的函数，初始化棋盘，定义棋盘坐标以左下角为（0，0）
void initBoard(void)
{
    gameState = GAME_RUNNING;
    currentPlayer = White;
    selectedX = -1;
    selectedY = -1;
    lastMoveFromX = lastMoveFromY = lastMoveToX = lastMoveToY = -1;

    for (int y = 2; y <= 5; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            board[x][y].type = EMPTY;
            board[x][y].color = NONE;
            board[x][y].moved = false;
        }
    }

    for (int x = 0; x < 8; x++)
    {
        board[x][0].moved = false;
        board[x][1].moved = false;
        board[x][0].color = White;
        board[x][1].color = White;
    }
    
    int whiteBackRow[8] = {ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK};
    for (int x = 0; x < 8; x++)
    {
        board[x][0].type = whiteBackRow[x];
        board[x][1].type = PAWN;
    }

    for (int x = 0; x < 8; x++)
    {
        board[x][6].moved = false;
        board[x][7].moved = false;
        board[x][6].color = Black;
        board[x][7].color = Black;
    }

    int blackBackRow[8] = {ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK};
    for (int x = 0; x < 8; x++)
    {
        board[x][6].type = PAWN;
        board[x][7].type = blackBackRow[x];
    }
}


int screenToBoardX(int screenX) {
    int x = (int)((screenX - BOARD_OFFSET_X) / CELL_SIZE);
    return (x >= 0 && x < BOARD_SIZE) ? x : -1;
}
// 坐标转换(白下黑上)
int screenToBoardY(int screenY) {
    int y = BOARD_SIZE - 1 - (int)((screenY - BOARD_OFFSET_Y) / CELL_SIZE);
    return (y >= 0 && y < BOARD_SIZE) ? y : -1;
}

// 白上黑下视角的Y坐标转换函数
int screenToBoardYWhiteUp(int screenY) {
    int y = (int)((screenY - BOARD_OFFSET_Y) / CELL_SIZE);
    return (y >= 0 && y < BOARD_SIZE) ? y : -1;
}

// 绘制选中棋子的红色边框
// 修改函数参数，添加视角判断参数
void drawSelectedHighlight(int x, int y, bool isWhiteUp) {
    int screenX = BOARD_OFFSET_X + x * CELL_SIZE;    // 计算棋子在屏幕上的位置
    // 根据视角计算不同的Y坐标
    int screenY;
    if (isWhiteUp) {
        screenY = BOARD_OFFSET_Y + y * CELL_SIZE;
    } else {
        screenY = BOARD_OFFSET_Y + (BOARD_SIZE - 1 - y) * CELL_SIZE;
    }
    setlinecolor(RED);     // 绘制红色边框（略小于棋子尺寸）
    setlinestyle(PS_SOLID, 3);  // 实线，3像素宽
    rectangle(
        screenX + 1,           // 左边距+2避免贴边
        screenY + 1,           // 上边距+2
        screenX + CELL_SIZE - 2,  // 右边距-2
        screenY + CELL_SIZE - 2   // 下边距-2
    );
}

void drawPiecesWhiteUp() {                      //具体注释见白方在下的视角
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            ChessPiece* piece = &board[x][y];
            if (piece->type == EMPTY) continue;
            
            int screenX = BOARD_OFFSET_X + x * 64;
            int screenY = BOARD_OFFSET_Y + y * 64;
            
            IMAGE* img = getPieceImage(piece->type, piece->color);
            if (img != NULL) {
                drawAlpha(img, screenX, screenY);
            }
        }
    }
}

// 定义函数：绘制棋子（以白方在下的视角）
void drawPiecesWhiteDown(void) { 
    for (int y = 0; y < BOARD_SIZE; y++) {         // 外层循环遍历棋盘的行（y坐标，0到7，共8行） 
        for (int x = 0; x < BOARD_SIZE; x++) {     // 内层循环遍历棋盘的列（x坐标，0到7，共8列）
            ChessPiece* piece = &board[x][y];      // 获取当前位置(x,y)的棋子指针（简化后续访问）
            if (piece->type == EMPTY) continue;    // 如果当前位置没有棋子（类型为EMPTY），跳过本次循环（不绘制）
            int screenX = BOARD_OFFSET_X + x * CELL_SIZE;    // 计算棋子在屏幕上的X坐标：棋盘左上角X偏移量 + 列索引*x格子像素大小（64）
            int screenY = BOARD_OFFSET_Y + (BOARD_SIZE - 1 - y) * CELL_SIZE;  // 计算棋子在屏幕上的Y坐标：棋盘左上角Y偏移量 + (7 - y) * 格子像素大小（棋盘行坐标需要翻转）
            IMAGE* img = getPieceImage(piece->type, piece->color);   // 根据棋子类型和颜色，获取对应的图片资源
            
            if (img != NULL) {                     // 如果图片资源存在，绘制带透明通道的棋子图片到计算好的屏幕坐标
                drawAlpha(img, screenX, screenY);
            }
        }
    }
}

// 重绘棋盘并添加选中高亮
// 修改函数，添加视角参数并根据视角绘制不同棋子布局
void redrawBoard(bool isWhiteUp) {
    drawAlpha(&res.chessboard, 44, 44);
    // 这是这一轮新增的代码：根据视角选择不同的绘制函数
    if (isWhiteUp) {
        drawPiecesWhiteUp();
    } else {
        drawPiecesWhiteDown();
    }
    if (isPieceSelected && selectedX != -1 && selectedY != -1) {   // 如果有选中的棋子，即2个条件均为真时绘制红色边框
        //传递视角参数
        drawSelectedHighlight(selectedX, selectedY, isWhiteUp);
    }
    
    // 绘制记谱
    drawNotation();
    
    FlushBatchDraw();
}




//修改函数，添加视角参数
void handleBoardClick(int x, int y, bool isWhiteUp) {
    if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) {     //即不在棋盘范围内，这里的x，y指代定义的棋盘坐标,
                                                                    //所以后续使用的时候是需转换的，详见case WM_LBUTTONDOWN                                                            
        return;
    }

    // 无论是否已选中，点击同色棋子都进行选择（包括重新选择）
    if (board[x][y].type != EMPTY && board[x][y].color == currentPlayer) {
        if (isPieceSelected && x == selectedX && y == selectedY) {
            // 点击已选中的棋子，取消选中
            isPieceSelected = false;
        } else {
            // 点击新的同色棋子，切换选中
            selectedX = x;
            selectedY = y;
            isPieceSelected = true;
        }
        //传递视角参数
        redrawBoard(isWhiteUp);  // 刷新显示
        return;
    }
    
    // 如果已经选中棋子，且点击的是空格或对方棋子，尝试移动
    if (isPieceSelected && selectedX != -1 && selectedY != -1) {
        // 调用isValidMove验证走法合法性
            if (isValidMove(selectedX, selectedY, x, y)) {
                // 检查是否要吃掉对方的王
                bool capturedKing = false;
                if (board[x][y].type == KING && board[x][y].color != currentPlayer) {
                    capturedKing = true;
                }
                
                // 创建Move对象记录这次移动
                Move move;
                move.fromX = selectedX;
                move.fromY = selectedY;
                move.toX = x;
                move.toY = y;
                move.piecetype = board[selectedX][selectedY].type;
                
                makeMove(selectedX, selectedY, x, y);
                
                // 记录棋步并生成记谱
                addNotationRecord(move);
                
                // 移动成功后重置选中状态（关键修改）
                isPieceSelected = false;
                selectedX = -1;
                selectedY = -1;
                // 传递视角参数
                redrawBoard(isWhiteUp);  // 刷新棋盘，红框消失
                
                // 检查胜负条件
                int checkmateResult = isCheckmate();
                
                // 如果吃掉了对方的王，直接判当前玩家胜利
                if (capturedKing || checkmateResult != 0) {
                    int winner = capturedKing ? currentPlayer : checkmateResult;
                    // 显示胜负界面
                    if (winner == White) {
                        // 白方胜
                        g_currentPage=PAGE_TEMP4;
                        drawAlpha(&res.Bkill, 300, 150);
                        FlushBatchDraw();
                    } else if (winner == Black) {
                        // 黑方胜
                        g_currentPage=PAGE_TEMP4;
                        drawAlpha(&res.Wkill, 300, 150);
                        FlushBatchDraw();
                    }
                } else {
                    // 调用switchPlayer函数切换玩家
                    switchPlayer();
                }
            }
    }
}

//绘画初始页面和非高亮状态下的按钮
void drawBasePage() {
    if (g_currentPage == PAGE_START) {
        drawAlpha(&res.startpage, 0, 0); // 绘制启动页背景
        // 初始绘制默认按钮（避免首次空白）
        for (int i = 0; i < g_buttonCount; i++) {
            if (g_buttons[i].belongPage == PAGE_START) {                            //判断需要绘制那一页的按钮
                drawAlpha(g_buttons[i].normalImg, g_buttons[i].x, g_buttons[i].y);  //按钮图片坐标参数
            }
        }
    } else if (g_currentPage == PAGE_CHOOSE) {
        cleardevice();     //先清空绘画区，不然会有第一页残留
        drawAlpha(&res.startpage, 0, 0); // 绘制初始页面
        drawAlpha(&res.chooseside,300,212);
        
        // 初始绘玩家选边页默认按钮
        for (int i = 0; i < g_buttonCount; i++) {
            if (g_buttons[i].belongPage == PAGE_CHOOSE) {
                drawAlpha(g_buttons[i].normalImg, g_buttons[i].x, g_buttons[i].y);   //同样的逻辑
            }
        }
    } else if (g_currentPage == PAGE_CHESS) {
        cleardevice();     //先清空绘画区，不然会有第一页残留
        drawAlpha(&res.chesspage, 0, 0);
        drawAlpha(&res.chessboard, 44, 44); // 绘制棋盘
        
        // 清空记谱记录
        notationCount = 0;
        round_num = 1;
        isWhiteMove = true;
        memset(notationRecords, 0, sizeof(notationRecords));
        
        // 初始绘制棋盘页默认按钮
        for (int i = 0; i < g_buttonCount; i++) {
            if (g_buttons[i].belongPage == PAGE_CHESS) {
                drawAlpha(g_buttons[i].normalImg, g_buttons[i].x, g_buttons[i].y);   //同样的逻辑
            }
        }
        
        // 绘制空的记谱框
        drawNotation();
    } 
}

////////所有与按钮点击相关的事件控制 
bool handleButtonClick(int mouseX, int mouseY) {
    for (int i = 0; i < g_buttonCount; i++) {         // 遍历所有按钮（g_buttons是全局按钮数组，g_buttonCount是实际注册的按钮数量）
        Button* btn = &g_buttons[i];        // 取当前索引i对应的按钮
        if (btn->belongPage != g_currentPage) continue;     // 只处理当前页面的按钮
        if (isMouseOnImage(mouseX, mouseY, btn->x, btn->y, btn->w, btn->h)) {       // 判断鼠标是否点击在当前按钮的范围内
            if (btn->normalImg == &res.startbutton) {    // 判定：当前按钮是“开始”按钮
                g_currentPage = PAGE_CHOOSE;        // 切换页面状态
                initBoard();
                drawBasePage(); // 切换页面时重新绘制基础界面
                FlushBatchDraw(); // 刷新双缓冲
            } else if (btn->normalImg == &res.choosesideWN){
                g_currentPage = PAGE_CHESS;        // 切换页面状态
                initBoard();
                drawBasePage(); // 切换页面时重新绘制基础界面
                // 设置白下黑上视角
                g_isWhiteUp = false;
                drawPiecesWhiteDown();
                FlushBatchDraw(); // 刷新双缓冲

            } else if (btn->normalImg == &res.choosesideBN){
                g_currentPage = PAGE_CHESS;        // 切换页面状态
                initBoard();
                drawBasePage(); // 切换页面时重新绘制基础界面
                // 设置白上黑下视角
                g_isWhiteUp = true;
                drawPiecesWhiteUp();
                FlushBatchDraw(); // 刷新双缓冲

            }else if (btn->normalImg == &res.buttondrawup){
                g_currentPage = PAGE_TEMP1;        // 切换页面状态
                if (g_isWhiteUp){
                    drawAlpha(&res.Wdrawbg,300,150);
                    drawAlpha(&res.drawacceptup,325,345);
                    drawAlpha(&res.drawrefuseup,525,345);
                } else {
                    drawAlpha(&res.Bdrawbg,300,150);
                    drawAlpha(&res.drawacceptup,325,345);
                    drawAlpha(&res.drawrefuseup,525,345);    
                }
                FlushBatchDraw(); // 刷新双缓冲

            }else if (btn->normalImg == &res.buttondrawdown){
                g_currentPage = PAGE_TEMP2;        // 切换页面状态
                if (g_isWhiteUp){
                    drawAlpha(&res.Bdrawbg,300,150);
                    drawAlpha(&res.drawacceptdown,325,345);
                    drawAlpha(&res.drawrefusedown,525,345);
                } else {
                    drawAlpha(&res.Wdrawbg,300,150);
                    drawAlpha(&res.drawacceptdown,325,345);
                    drawAlpha(&res.drawrefusedown,525,345);
                }
                FlushBatchDraw(); // 刷新双缓冲

            }else if (btn->normalImg == &res.buttonloseup){
                g_currentPage = PAGE_TEMP3;        // 切换页面状态
                if (g_isWhiteUp){
                    drawAlpha(&res.Wlosebg,300,150);
                    drawAlpha(&res.reset,325,345);
                    drawAlpha(&res.exit,525,345);
                    
                } else{
                    drawAlpha(&res.Blosebg,300,150);
                    drawAlpha(&res.reset,325,345);
                    drawAlpha(&res.exit,525,345);
                    
                }
                FlushBatchDraw(); // 刷新双缓冲

            }else if (btn->normalImg == &res.buttonlosedown){
                g_currentPage = PAGE_TEMP3;        // 切换页面状态
                if (g_isWhiteUp){
                    drawAlpha(&res.Blosebg,300,150);
                    drawAlpha(&res.reset,325,345);
                    drawAlpha(&res.exit,525,345);
                    
                } else{
                    drawAlpha(&res.Wlosebg,300,150);
                    drawAlpha(&res.reset,325,345);
                    drawAlpha(&res.exit,525,345);
                    
                }
                FlushBatchDraw(); // 刷新双缓冲
            }else if (btn->normalImg == &res.drawacceptup){
                g_currentPage = PAGE_START;
                drawBasePage(); // 切换页面时重新绘制基础界面
                FlushBatchDraw(); // 刷新双缓冲

            }else if (btn->normalImg == &res.drawrefusedown){
                g_currentPage = PAGE_CHESS;
                // 先清空当前缓冲区
                cleardevice();
                // 绘制棋盘页基础元素
                drawBasePage();
                // 重新绘制棋子（根据当前视角）
                if (g_isWhiteUp) {
                    drawPiecesWhiteUp();
                } else {
                    drawPiecesWhiteDown();
                }
                // 刷新整个缓冲区
                FlushBatchDraw();
                return true;
            }else if (btn->normalImg == &res.drawacceptdown){
                g_currentPage = PAGE_START;
                drawBasePage(); // 切换页面时重新绘制基础界面
                FlushBatchDraw(); // 刷新双缓冲

            }else if (btn->normalImg == &res.drawrefuseup){
                g_currentPage = PAGE_CHESS;
                // 先清空当前缓冲区
                cleardevice();
                // 绘制棋盘页基础元素
                drawBasePage();
                // 重新绘制棋子（根据当前视角）
                if (g_isWhiteUp) {
                    drawPiecesWhiteUp();
                } else {
                    drawPiecesWhiteDown();
                }
                // 刷新整个缓冲区
                FlushBatchDraw();
                return true;
            }else if (btn->normalImg == &res.reset){
                g_currentPage = PAGE_START;       
                drawBasePage(); // 切换页面时重新绘制基础界面
                FlushBatchDraw(); // 刷新双缓冲

            }else if (btn->normalImg == &res.exit){
                closegraph();
            }
            return true;
        }
    }
    return false;
}

// 核心：循环监听鼠标消息，仅重绘按钮（无闪烁）

///前端逻辑的实现（不包含开场视频的播放）
void uiloop() {
ExMessage msg;
    // 注册所有按钮
    registerButton(385, 225, 200, 80, &res.startbutton, &res.startbuttonhighlight, PAGE_START);
    registerButton(350,312,125, 50,&res.choosesideWN, &res.choosesideWH, PAGE_CHOOSE);
    registerButton(525,312,125, 50,&res.choosesideBN, &res.choosesideBH, PAGE_CHOOSE);
    registerButton(600, 25, 135, 75, &res.buttondrawup, &res.buttondrawhighlightup, PAGE_CHESS);
    registerButton(815, 25, 135, 75, &res.buttonloseup, &res.buttonlosehighlightup, PAGE_CHESS);
    registerButton(600, 500, 135, 75, &res.buttondrawdown, &res.buttondrawhighlightdown, PAGE_CHESS);
    registerButton(815, 500, 135, 75, &res.buttonlosedown, &res.buttonlosehighlightdown, PAGE_CHESS);
    registerButton(325, 345, 150,50, &res.drawacceptup, &res.drawaccepthighlightup, PAGE_TEMP1 );
    registerButton(325, 345, 150,50, &res.drawacceptdown, &res.drawaccepthighlightdown, PAGE_TEMP2 );
    registerButton(525, 345, 150,50, &res.drawrefuseup, &res.drawrefusehighlightup, PAGE_TEMP1 );
    registerButton(525, 345, 150,50, &res.drawrefusedown, &res.drawrefusehighlightdown, PAGE_TEMP2 );
    registerButton(325, 345, 150,50, &res.reset, &res.resethighlight, PAGE_TEMP3);
    registerButton(525, 345, 150,50, &res.exit, &res.exithighlight,PAGE_TEMP3);



    // 初始化页面（只画一次基础界面）
    drawBasePage();
    FlushBatchDraw(); // 首次刷新双缓冲

    while (true) {
        // 1. 监听鼠标消息（非阻塞，避免卡顿）
        if (peekmessage(&msg, EX_MOUSE)) {
            switch (msg.message) {
                case WM_MOUSEMOVE:
                    // 遍历当前页面按钮，仅重绘状态变化的按钮
                    for (int i = 0; i < g_buttonCount; i++) {
                        Button* btn = &g_buttons[i];
                        if (btn->belongPage != g_currentPage) continue; //页面判断

                        // 判断当前是否悬浮（鼠标是否停留在特定区域）
                        bool currentHover = isMouseOnImage(msg.x, msg.y, btn->x, btn->y, btn->w, btn->h);
                        // 只有状态变化时才重绘（避免重复绘制导致闪烁）
                        if (currentHover != g_isHover[i]) {
                            if (currentHover) {
                                drawAlpha(btn->highlightImg, btn->x, btn->y); // 悬浮→高亮图
                            } else {
                                drawAlpha(btn->normalImg, btn->x, btn->y); // 离开→默认图
                            }
                            g_isHover[i] = currentHover; // 更新状态
                            FlushBatchDraw(btn->x, btn->y, btn->w, btn->h); // 局部刷新按钮区域（关键防闪）
                        }
                    }
                    break;

                case WM_LBUTTONDOWN:
                    if (g_currentPage == PAGE_CHESS) {
                        // 根据视角选择不同的坐标转换方式
                        int boardX = screenToBoardX(msg.x);
                        int boardY;
                        if (g_isWhiteUp) {
                            boardY = screenToBoardYWhiteUp(msg.y);
                        } else {
                            boardY = screenToBoardY(msg.y);
                        }
                        if (boardX != -1 && boardY != -1) {
                            //传递视角参数
                            handleBoardClick(boardX, boardY, g_isWhiteUp);
                            break;
                        }
                    }
                    handleButtonClick(msg.x, msg.y);
                    break;                                
            }
        }

        if (_kbhit() && _getch() == 27) {
            break;
        }

        Sleep(5);
    }
}

// 动画资源结构体
IMAGE g_splashFrames[FRAME_COUNT];
bool g_splashFramesLoaded = false;

// 初始化启动动画
void initSplashAnimation() {
    // 加载所有动画帧
    char framePath[256];
    for (int i = 0; i < FRAME_COUNT; i++) {
        sprintf_s(framePath, FRAME_PATH, i + FRAME_START);
        loadimage(&g_splashFrames[i], framePath, WIN_WIDTH, WIN_HEIGHT);
    }
    g_splashFramesLoaded = true;
}

// 播放启动动画
void playSplashAnimation() {
    if (!g_splashFramesLoaded) {
        initSplashAnimation();
    }
    
    // 逐帧播放动画
    for (int i = 0; i < FRAME_COUNT; i++) {
        putimage(0, 0, WIN_WIDTH, WIN_HEIGHT, &g_splashFrames[i], 0, 0, SRCCOPY);
        FlushBatchDraw();
        
        // 控制帧率
        Sleep(DELAY_TIME);
        
        // 检测 ESC 键退出
        if (_kbhit() && _getch() == 27) {
            break;
        }
    }
}

// 释放启动动画资源
void freeSplashAnimation() {
    // EasyX会自动管理IMAGE对象的内存，不需要手动释放
    g_splashFramesLoaded = false;
}

//前端所有的实现（包含开场视频的实现，注意，主函数只需要调动这一个就行了）
void startvedio()
{
    // 1. 创建 EasyX 图形窗口（固定尺寸）
    initgraph(WIN_WIDTH, WIN_HEIGHT);

    // 2. 逐帧播放（只播放一次）
    char framePath[256];
    for (int i = 1; i <= FRAME_COUNT; i++)
    {
        // 拼接当前帧路径
        sprintf_s(framePath, FRAME_PATH, i);

        IMAGE img;
        loadimage(&img, framePath,1000,600);
        
        // 拉伸绘制图片以适应窗口大小（0,0为起点，窗口宽高为目标尺寸）
        putimage(0, 0, WIN_WIDTH, WIN_HEIGHT, &img, 0, 0, SRCCOPY);

        // 控制帧率
        Sleep(DELAY_TIME);

        // 检测 ESC 键退出
        if (_kbhit() && _getch() == 27)
        {
            goto ExitPlay;
        }    
    }
    res_init(&res);
    uiloop();
    _getch();
    closegraph();

ExitPlay:
    // 3. 播放结束或退出时关闭窗口
    closegraph();
}



// main函数已移至main.c文件中
























