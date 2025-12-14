#include "chess.h"

int isCheckmate(void)
{
    // 检查对方玩家是否被将死
    int opposingColor = 3 - currentPlayer;
    MoveList pseudoMoves = generatePseudoMoves(opposingColor);
    MoveList legalMoves = generateLegalMoves(&pseudoMoves);
    if(isInCheck(opposingColor) && legalMoves.count == 0){
        return currentPlayer ;//当前玩家胜
    }
    else{
        return 0 ;//继续
    }
}