#include "chess.h"

// 一. (总)检测并记录伪合法走法（pseudo-legal moves）
// 因为在判断违例时还需要换边，所以用变量player来代替
MoveList generatePseudoMoves(int player)
{
    MoveList PseudoList;
    PseudoList.count = 0;
    // 1. 循环扫描整个棋盘，循环内判断是否为己方棋子
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            // 跳过不是己方棋子的格子
            if (board[i][j].color != player)
            {
                continue;
            }

            // 用piece变量来存储当前棋子，便于后续判断
            ChessPiece CurrentPiece = board[i][j];

            // 2. 循环内switch-case判断pieceType
            // 对应generateXMoves函数把合法招法加入list列表
            switch (CurrentPiece.type)
            {
            case KING:
                generateKingMoves(i, j, player, &PseudoList);
                break;
            case QUEEN:
                generateQueenMoves(i, j, player, &PseudoList);
                break;
            case ROOK:
                generateRookMoves(i, j, player, &PseudoList);
                break;
            case BISHOP:
                generateBishopMoves(i, j, player, &PseudoList);
                break;
            case KNIGHT:
                generateKnightMoves(i, j, player, &PseudoList);
                break;
            case PAWN:
                generatePawnMoves(i, j, player, &PseudoList);
                break;           
            default:
                break;
            }
        }
    } 
    return PseudoList;
}

// 二. (辅助函数)分别生成KQRBNP的规则招法
// 逻辑：直接依次用循环判断可行招法，每多一步就导入一个到list里面
void generateKingMoves(int x, int y, int player, MoveList *list);
void generateQueenMoves(int x, int y, int player, MoveList *list);
void generateRookMoves(int x, int y, int player, MoveList *list);
void generateBishopMoves(int x, int y, int player, MoveList *list);
void generateKnightMoves(int x, int y, int player, MoveList *list);
void generatePawnMoves(int x, int y, int player, MoveList *list);

// 2.1 生成王的规则招法
void generateKingMoves(int x, int y, int player, MoveList *list)
{
    // 依次判断四周格子
    for(int i = x - 1; i <= x + 1; i++)
    {
        for(int j = y - 1; j <= y + 1; j++)
        {
            // 1. 排除超出边界的格子
            if (i < 0 || i >= 8 || j < 0 || j >= 8)
            {
                continue;
            }

            // 2. 排除有己方棋子的格子（对于王，直接跳过就行，不需要做阻断）
            else if (board[i][j].color == player)
            {
                continue;
            }

            // 3. 空格子直接加
            // 对于王的情况，吃子可以直接和空格子放一起作为else
            else
            {
                // 定义新招法
                Move *newMove = &(list -> moves[list->count]);
                newMove -> fromX = x;
                newMove -> fromY = y;
                newMove -> toX = i;
                newMove -> toY = j;
                newMove -> piecetype = KING;

                // movelist中招法数量+1
                list->count++;
            }
        }
    }
}

// 2.2 生成后的规则招法
// 注意：第一次碰到非EMPTY的格子就不用继续检测了
void generateQueenMoves(int x, int y, int player, MoveList *list)
{
    // 用原点+向量的逻辑进行判断（方便实现“遇到阻隔就不继续延伸”的规则）
    // 依次是 上下左右+斜 {△x, △y}
    int dirs[8][2] = {{0,1}, {0,-1}, {1,0}, {-1,0}, 
                      {1,1}, {1,-1}, {-1,1}, {-1,-1}};

    // 依次判断米字格
    for (int d = 0; d < 8; d++)
    {
        int dx = dirs[d][0]; // 当前方向的X增量（行变化）
        int dy = dirs[d][1]; // 当前方向的Y增量（列变化）

        // 沿当前方向逐步延伸（step=1：第一步，step=2：第二步...直到越界/遇棋子）
        for (int step = 1; ; step++)
        {
            // 计算当前延伸到的目标坐标
            int targetX = x + dx * step;
            int targetY = y + dy * step;

            // 1. 边界判断：超出棋盘（0~7）则停止该方向
            if (targetX < 0 || targetX >= 8 || targetY < 0 || targetY >= 8)
            {
                break;
            }

            // 2. 获取目标位置的棋子
            ChessPiece targetPiece = board[targetX][targetY];

            // 3. 遇到己方棋子：停止该方向
            if (targetPiece.color == player)
            {
                break;
            }

            // 4. 空格子：添加招法，继续延伸
            if (targetPiece.color == NONE)
            {
                // 填充Move信息并加入列表
                Move *newMove = &list -> moves[list -> count];
                newMove->fromX = x;
                newMove->fromY = y;
                newMove->toX = targetX;
                newMove->toY = targetY;
                newMove->piecetype = QUEEN;
                list->count++;
            }

            // 5. 遇到对方棋子：添加吃子招法，停止该方向（被对方棋子挡住）
            else if (targetPiece.color != player && targetPiece.color != NONE)
            {
                // 填充吃子招法
                Move *newMove = &list->moves[list->count];
                newMove->fromX = x;
                newMove->fromY = y;
                newMove->toX = targetX;
                newMove->toY = targetY;
                newMove->piecetype = QUEEN;
                list->count++;

                break; // 吃子后停止该方向
            }
        }
    }
}

// 2.3 生成车的规则招法
void generateRookMoves(int x, int y, int player, MoveList *list)
{
    // 原点 + 向量，依次是 上下左右
    int dirs[4][2] = {{0,1}, {0,-1}, {1,0}, {-1,0}};

    // 依次判断上下左右
    for (int d = 0; d < 4; d++)
    {
        int dx = dirs[d][0]; // 当前方向的X增量（行变化）
        int dy = dirs[d][1]; // 当前方向的Y增量（列变化）

        // 沿当前方向逐步延伸（step=1：第一步，step=2：第二步...直到越界/遇棋子）
        for (int step = 1; ; step++)
        {
            // 计算当前延伸到的目标坐标
            int targetX = x + dx * step;
            int targetY = y + dy * step;

            // 1. 边界判断：超出棋盘（0~7）则停止该方向
            if (targetX < 0 || targetX >= 8 || targetY < 0 || targetY >= 8)
            {
                break;
            }

            // 2. 获取目标位置的棋子
            ChessPiece targetPiece = board[targetX][targetY];

            // 3. 遇到己方棋子：停止该方向
            if (targetPiece.color == player)
            {
                break;
            }

            // 4. 空格子：添加招法，继续延伸
            if (targetPiece.color == NONE)
            {
                // 填充Move信息并加入列表
                Move *newMove = &list -> moves[list -> count];
                newMove->fromX = x;
                newMove->fromY = y;
                newMove->toX = targetX;
                newMove->toY = targetY;
                newMove->piecetype = ROOK;
                list->count++;
            }

            // 5. 遇到对方棋子：添加吃子招法，停止该方向（被对方棋子挡住）
            else if (targetPiece.color != player && targetPiece.color != NONE)
            {
                // 填充吃子招法
                Move *newMove = &list->moves[list->count];
                newMove->fromX = x;
                newMove->fromY = y;
                newMove->toX = targetX;
                newMove->toY = targetY;
                newMove->piecetype = ROOK;
                list->count++;

                break; // 吃子后停止该方向
            }
        }
    }
}

// 2.4 生成象的规则招法
void generateBishopMoves(int x, int y, int player, MoveList *list)
{
    // 原点 + 向量，斜向判断
    int dirs[4][2] = {{1,1}, {1,-1}, {-1,1}, {-1,-1}};

    // 依次判断斜向
    for (int d = 0; d < 4; d++)
    {
        int dx = dirs[d][0]; // 当前方向的X增量（行变化）
        int dy = dirs[d][1]; // 当前方向的Y增量（列变化）

        for (int step = 1; ; step++)
        {
            // 计算当前延伸到的目标坐标
            int targetX = x + dx * step;
            int targetY = y + dy * step;

            // 1. 边界判断：超出棋盘（0~7）则停止该方向
            if (targetX < 0 || targetX >= 8 || targetY < 0 || targetY >= 8)
            {
                break;
            }

            // 2. 获取目标位置的棋子
            ChessPiece targetPiece = board[targetX][targetY];

            // 3. 遇到己方棋子：停止该方向
            if (targetPiece.color == player)
            {
                break;
            }

            // 4. 空格子：添加招法，继续延伸
            if (targetPiece.color == NONE)
            {
                // 填充Move信息并加入列表
                Move *newMove = &list -> moves[list -> count];
                newMove->fromX = x;
                newMove->fromY = y;
                newMove->toX = targetX;
                newMove->toY = targetY;
                newMove->piecetype = BISHOP;
                list->count++;
            }

            // 5. 遇到对方棋子：添加吃子招法，停止该方向（被对方棋子挡住）
            else if (targetPiece.color != player && targetPiece.color != NONE)
            {
                // 填充吃子招法
                Move *newMove = &list->moves[list->count];
                newMove->fromX = x;
                newMove->fromY = y;
                newMove->toX = targetX;
                newMove->toY = targetY;
                newMove->piecetype = BISHOP;
                list->count++;

                break; // 吃子后停止该方向
            }
        }
    }
}

// 2.5 生成马的规则招法
void generateKnightMoves(int x, int y, int player, MoveList *list)
{
    // 用原点+向量的逻辑进行判断（方便实现“遇到阻隔就不继续延伸”的规则）
    // 依次是 上下左右+斜 {△x, △y}
    int dirs[8][2] = {{2,1}, {2,-1}, {-2,1}, {-2,-1}, 
                      {1,2}, {1,-2}, {-1,2}, {-1,-2}};

    // 依次判断日字格
    for (int d = 0; d < 8; d++)
    {
        int dx = dirs[d][0]; // 当前方向的X增量（行变化）
        int dy = dirs[d][1]; // 当前方向的Y增量（列变化）

        // 计算当前延伸到的目标坐标
        int targetX = x + dx;
        int targetY = y + dy;

        // 1. 边界判断
        if (targetX < 0 || targetX >= 8 || targetY < 0 || targetY >= 8)
        {
            continue;
        }

        // 3. 排除己方棋子
        if (board[targetX][targetY].color == player)
        {
            continue;
        }

        // 4. 空格子或对方棋子：添加招法
        else
        {
            // 填充Move信息并加入列表
            Move *newMove = &list -> moves[list -> count];
            newMove -> fromX = x;
            newMove -> fromY = y;
            newMove -> toX = targetX;
            newMove -> toY = targetY;
            newMove -> piecetype = KNIGHT;

            list->count++;
        }
    }
}

// 2.6 生成兵的规则招法
// 兵在直线上只需要考虑空格子，斜线上只考虑吃子
void generatePawnMoves(int x, int y, int player, MoveList *list)
{
    // 白兵
    if(player == White)
    {
        // 判断是否第1步 - 是 → 判断直行2格
        if (y == 1)
        {
            if (board[x][y + 2].color == NONE && board[x][y + 1].color == NONE)
            {
                // 定义新招法
                Move *newMove = &(list -> moves[list -> count]);
                newMove -> fromX = x;
                newMove -> fromY = y;
                newMove -> toX = x;
                newMove -> toY = y + 2;
                newMove -> piecetype = PAWN;

                // movelist中招法数量+1
                list -> count++;
            }
        }

        // 判断1格(有边界判断；升变后续再写)
        if (y + 1 >= 8)
        {
            return; // 因为后续1格和吃子的判断全都纵向移1格，所以直接返回
        }

        if (board[x][y + 1].color == NONE)
        {
            // 定义新招法
            Move *newMove = &(list -> moves[list -> count]);
            newMove -> fromX = x;
            newMove -> fromY = y;
            newMove -> toX = x;
            newMove -> toY = y + 1;
            newMove -> piecetype = PAWN;

            // movelist中招法数量+1
            list -> count++;
        }

        // 判断吃子
        if (x - 1 >= 0)
        {
            if (board[x - 1][y + 1].color == Black)
            {
                // 定义新招法
                Move *newMove = &(list -> moves[list -> count]);
                newMove -> fromX = x;
                newMove -> fromY = y;
                newMove -> toX = x - 1;
                newMove -> toY = y + 1;
                newMove -> piecetype = PAWN;

                // movelist中招法数量+1
                list -> count++;
            }
        }

        if (x + 1 < 8)
        {
            if (board[x + 1][y + 1].color == Black)
            {
                // 定义新招法
                Move *newMove = &(list -> moves[list -> count]);
                newMove -> fromX = x;
                newMove -> fromY = y;
                newMove -> toX = x + 1;
                newMove -> toY = y + 1;
                newMove -> piecetype = PAWN;

                // movelist中招法数量+1
                list -> count++;
            }
        }
    }

    // 黑兵
    else if(player == Black)
    {
        // 判断是否第1步 - 是 → 判断直行2格
        if (y == 6)
        {
            if (board[x][y - 2].color == NONE && board[x][y - 1].color == NONE)
            {
                // 定义新招法
                Move *newMove = &(list -> moves[list -> count]);
                newMove -> fromX = x;
                newMove -> fromY = y;
                newMove -> toX = x;
                newMove -> toY = y - 2;
                newMove -> piecetype = PAWN;

                // movelist中招法数量+1
                list -> count++;
            }
        }

        // 判断1格(有边界判断；升变后续再写)
        if (y - 1 < 0)
        {
            return;
        }

        if (board[x][y - 1].color == NONE)
        {
            // 定义新招法
            Move *newMove = &(list -> moves[list -> count]);
            newMove -> fromX = x;
            newMove -> fromY = y;
            newMove -> toX = x;
            newMove -> toY = y - 1;
            newMove -> piecetype = PAWN;

            // movelist中招法数量+1
            list -> count++;
        }

        // 判断吃子
        if (x - 1 >= 0)
        {
            if (board[x - 1][y - 1].color == White)
            {
                // 定义新招法
                Move *newMove = &(list -> moves[list -> count]);
                newMove -> fromX = x;
                newMove -> fromY = y;
                newMove -> toX = x - 1;
                newMove -> toY = y - 1;
                newMove -> piecetype = PAWN;

                // movelist中招法数量+1
                list -> count++;
            }
        }

        if (x + 1 < 8)
        {
            if (board[x + 1][y - 1].color == White)
            {
                // 定义新招法
                Move *newMove = &(list -> moves[list -> count]);
                newMove -> fromX = x;
                newMove -> fromY = y;
                newMove -> toX = x + 1;
                newMove -> toY = y - 1;
                newMove -> piecetype = PAWN;

                // movelist中招法数量+1
                list -> count++;
            }
        }
    }
}

// 三. 给出合法招法列表（考虑王不被将军）
// 逻辑：对pseudo中的每一步进行模拟执行，用isInCheck判断是否违例，
// 把合法招法传给LegalList之后再undo回复原来的局面
MoveList generateLegalMoves(MoveList *PseudoList)
{
    MoveList LegalList;
    LegalList.count = 0;
    // 方案：局部更改

    // 1. 用主循环对PsuedoList中每一步进行执行(直接改board)
    for(int i = 0; i < PseudoList->count; i++)
    {
        // 存储原数据
        ChessPiece fromPiece = board[PseudoList->moves[i].fromX][PseudoList->moves[i].fromY];
        ChessPiece toPiece = board[PseudoList->moves[i].toX][PseudoList->moves[i].toY];
        // 更改棋盘
        // 修改新位置
        board[PseudoList->moves[i].toX][PseudoList->moves[i].toY] = fromPiece;    // 考虑改：理论上GLMoves函数只生成己方招法，但万一要生成对方合法招法调用就不方便了
        board[PseudoList->moves[i].toX][PseudoList->moves[i].toY].moved = true;
        
        // 清空原位置
        board[PseudoList->moves[i].fromX][PseudoList->moves[i].fromY].type = EMPTY;
        board[PseudoList->moves[i].fromX][PseudoList->moves[i].fromY].color = NONE;
        board[PseudoList->moves[i].fromX][PseudoList->moves[i].fromY].moved = false;


        // 3. 用isInCheck函数判断执行后是否送王
        // 合法招法加入LegalList，违例就跳过
        // 这里应该使用走棋方的颜色，而不是currentPlayer
        if(isInCheck(fromPiece.color) == 0)
        {
            LegalList.moves[LegalList.count++] = PseudoList->moves[i];
        }
        // 4. 判断结束一步之后，将board回复OriginBoard
        // 恢复原位置
        board[PseudoList->moves[i].fromX][PseudoList->moves[i].fromY] = fromPiece;
        // 恢复新位置
        board[PseudoList->moves[i].toX][PseudoList->moves[i].toY] = toPiece;
    }
    return LegalList;
}

// 四. (辅助)将军判断函数
bool isInCheck(int KingColor)
{
    // 基础：要有对应的board
    // 在一方走棋之后，判断王是否被将军的函数
    // 1. 生成对方的规则招法（不需要考虑合法，因为只要在能攻击到的位置就违例）
    bool checked = false;
    MoveList OpposingList = generatePseudoMoves(3 - KingColor);     // 利用代数关系生成对方可行招法
    //（健壮性待改善：KingColor必须是正确的白或黑，否则越界）

    // 2. 遍历找到己方王的位置
    int KingPosX = -1, KingPosY = -1;
    for(int i = 0; i < 8; i++)
    {
        for(int j = 0; j < 8; j++)
        {
            if(board[i][j].color == KingColor && board[i][j].type == KING)
            {
                KingPosX = i;
                KingPosY = j;
                goto endSearch; // 用的goto，后续可以另封装函数。
            }
        }
    }
    endSearch:;

    // 3. 看对方合法招法中toX, toY有没有王的位置
    for(int i = 0; i < OpposingList.count; i++)
    {
        if(KingPosX == OpposingList.moves[i].toX && KingPosY == OpposingList.moves[i].toY)
        {
            checked = true;
            break;
        }
    }

    // 4. 如果有，返回true; 没有，返回false
    return checked;
}

// 执行走法（修改 board）
int makeMove(int fromX, int fromY, int toX, int toY)
{
    //移动棋子在棋盘上的broad坐标
    board[toX][toY] = board[fromX][fromY];
    board[toX][toY].moved = true;//标记棋子已移动
    //修正原位置清空方式，使用正确的空值
    board[fromX][fromY].type = EMPTY;
    board[fromX][fromY].color = NONE;
    board[fromX][fromY].moved = false;

    return 1;
}

// 检查走法合法性
int isValidMove(int fromX, int fromY, int toX, int toY)
{
    // 特殊情况：如果可以吃掉对方的王，直接允许这个移动
    if (board[toX][toY].type == KING && board[toX][toY].color != currentPlayer) {
        return 1; // 合法走法，吃掉对方的王
    }
    
    // 1. 生成当前玩家的所有伪合法走法
    MoveList pseudoMoves = generatePseudoMoves(currentPlayer);
    // 2. 生成合法走法（考虑将军）
    MoveList legalMoves = generateLegalMoves(&pseudoMoves);
    
    // 3. 检查给定的走法是否在合法走法列表中
    for (int i = 0; i < legalMoves.count; i++) {
        if (legalMoves.moves[i].fromX == fromX && 
            legalMoves.moves[i].fromY == fromY && 
            legalMoves.moves[i].toX == toX && 
            legalMoves.moves[i].toY == toY) {
            return 1; // 合法走法
        }
    }
    
    return 0; // 非法走法
}

// 切换玩家
void switchPlayer(void) {
    currentPlayer = (currentPlayer == White) ? Black : White;
}

// 棋谱记录（生成棋谱字符串）