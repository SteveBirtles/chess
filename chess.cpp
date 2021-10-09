#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

const int windowWidth = 1024;
const int windowHeight = 768;

enum Turns {
    WHITE,
    BLACK,
};

enum Pieces {
    NONE = 0,
    WHITE_KING = 1,
    WHITE_QUEEN,
    WHITE_BISHOP,
    WHITE_KNIGHT,
    WHITE_ROOK,
    WHITE_PAWN,
    BLACK_KING,
    BLACK_QUEEN,
    BLACK_BISHOP,
    BLACK_KNIGHT,
    BLACK_ROOK,
    BLACK_PAWN,
};

enum Spaces {
    INVALID = 0,
    ATTACK,
    MOVE,
    GREEN,
    CYAN,
    START,
    PURPLE
};

bool isKing(Pieces p) { return p == WHITE_KING || p == BLACK_KING; }
bool isQueen(Pieces p) { return p == WHITE_QUEEN || p == BLACK_QUEEN; }
bool isBishop(Pieces p) { return p == WHITE_BISHOP || p == BLACK_BISHOP; }
bool isKnight(Pieces p) { return p == WHITE_KNIGHT || p == BLACK_KNIGHT; }
bool isRook(Pieces p) { return p == WHITE_ROOK || p == BLACK_ROOK; }
bool isPawn(Pieces p) { return p == WHITE_PAWN || p == BLACK_PAWN; }
bool isWhite(Pieces p) { return p != NONE && p < BLACK_KING; }
bool isBlack(Pieces p) { return p > WHITE_PAWN; }

class ChessGame : public olc::PixelGameEngine {
   public:
    ChessGame() { sAppName = "Chess"; }

   private:
    std::unique_ptr<olc::Sprite> spritePieces;
    std::unique_ptr<olc::Decal> decalPieces;
    olc::vi2d vGridSize = {80, 80};
    olc::vi2d vPieceSize = {320, 320};

    olc::Pixel shade[7] = {
        olc::BLACK,
        olc::Pixel(63, -63, -63),
        olc::Pixel(63, 63, -63),
        olc::Pixel(-63, 63, -63),
        olc::Pixel(-63, 63, 63),
        olc::Pixel(-63, -63, 63),
        olc::Pixel(63, -63, 63),
    };

    olc::vi2d deltas[8] = {
        olc::vi2d(1, 0),
        olc::vi2d(1, 1),
        olc::vi2d(0, 1),
        olc::vi2d(-1, 1),
        olc::vi2d(-1, 0),
        olc::vi2d(-1, -1),
        olc::vi2d(0, -1),
        olc::vi2d(1, -1),
    };

    Pieces board[8][8] = {
        {BLACK_ROOK, BLACK_PAWN, NONE, NONE, NONE, NONE, WHITE_PAWN, WHITE_ROOK},
        {BLACK_KNIGHT, BLACK_PAWN, NONE, NONE, NONE, NONE, WHITE_PAWN, WHITE_KNIGHT},
        {BLACK_BISHOP, BLACK_PAWN, NONE, NONE, NONE, NONE, WHITE_PAWN, WHITE_BISHOP},
        {BLACK_QUEEN, BLACK_PAWN, NONE, NONE, NONE, NONE, WHITE_PAWN, WHITE_QUEEN},
        {BLACK_KING, BLACK_PAWN, NONE, NONE, NONE, NONE, WHITE_PAWN, WHITE_KING},
        {BLACK_BISHOP, BLACK_PAWN, NONE, NONE, NONE, NONE, WHITE_PAWN, WHITE_BISHOP},
        {BLACK_KNIGHT, BLACK_PAWN, NONE, NONE, NONE, NONE, WHITE_PAWN, WHITE_KNIGHT},
        {BLACK_ROOK, BLACK_PAWN, NONE, NONE, NONE, NONE, WHITE_PAWN, WHITE_ROOK},
    };

    Spaces space[8][8] = {
        {INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID},
        {INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID},
        {INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID},
        {INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID},
        {INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID},
        {INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID},
        {INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID},
        {INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID},
    };

    olc::vi2d moveStart = olc::vi2d(-1, -1);
    olc::vi2d moveEnd = olc::vi2d(-1, -1);

    Turns turn = WHITE;

   public:
    bool OnUserCreate() override {
        spritePieces = std::make_unique<olc::Sprite>("./pieces.png");
        decalPieces = std::make_unique<olc::Decal>(spritePieces.get());

        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override {
        Clear(olc::Pixel(0, 0, 0));
        SetPixelMode(olc::Pixel::MASK);

        olc::Pixel c;
        olc::vf2d vScale = {(float)vGridSize.x / (float)vPieceSize.x,
                            (float)vGridSize.y / (float)vPieceSize.y};
        olc::vi2d vOffset = {windowWidth / 2 - vGridSize.x * 4,
                             windowHeight / 2 - vGridSize.y * 4};

        olc::vi2d mouse = GetMousePos() - vOffset;

        olc::HWButton mouseButton = GetMouse(0);

        int u = -1, v = -1;
        if (mouse.x >= 0 && mouse.y >= 0 && mouse.x < 8 * vGridSize.x &&
            mouse.y < 8 * vGridSize.y) {
            u = mouse.x / vGridSize.x;
            v = mouse.y / vGridSize.y;
        }

        if (mouseButton.bPressed && u >= 0 && v >= 0 && u < 8 && v < 8) {
            if (space[u][v] != INVALID) {
                if (space[u][v] != START) {
                    board[u][v] = board[moveStart.x][moveStart.y];
                    board[moveStart.x][moveStart.y] = NONE;
                    if (turn == WHITE)
                        turn = BLACK;
                    else
                        turn = WHITE;
                }

                moveStart = olc::vi2d(-1, -1);

                for (int i = 0; i < 8; i++) {
                    for (int j = 0; j < 8; j++) {
                        space[i][j] = INVALID;
                    }
                }
            } else if (moveStart.x == -1 && moveStart.y == -1) {
                if (board[u][v] != NONE) {
                    Pieces piece;
                    piece = board[u][v];

                    if (isWhite(piece) && turn == WHITE ||
                        isBlack(piece) && turn == BLACK) {
                        for (int i = 0; i < 8; i++) {
                            for (int j = 0; j < 8; j++) {
                                space[i][j] = INVALID;
                            }
                        }
                        space[u][v] = START;
                        moveStart = olc::vi2d(u, v);

                        if (piece == BLACK_PAWN && v < 7) {
                            if (board[u][v + 1] == 0) {
                                space[u][v + 1] = MOVE;
                                if (v == 1 && board[u][v + 2] == 0) space[u][v + 2] = MOVE;
                            }
                            if (u < 7 && isWhite(board[u + 1][v + 1])) space[u + 1][v + 1] = ATTACK;
                            if (u > 0 && isWhite(board[u - 1][v + 1])) space[u - 1][v + 1] = ATTACK;
                        }

                        if (piece == WHITE_PAWN && v > 0) {
                            if (board[u][v - 1] == 0) {
                                space[u][v - 1] = MOVE;
                                if (v == 6 && board[u][v - 2] == 0) space[u][v - 2] = MOVE;
                            }
                            if (u < 7 && isBlack(board[u + 1][v - 1])) space[u + 1][v - 1] = ATTACK;
                            if (u > 0 && isBlack(board[u - 1][v - 1])) space[u - 1][v - 1] = ATTACK;
                        }

                        if (!(isPawn(piece) || isKnight(piece))) {
                            for (int direction = 0; direction < 8; direction++) {
                                if (direction % 2 != 0 && isRook(piece))
                                    continue;
                                if (direction % 2 == 0 && isBishop(piece))
                                    continue;
                                for (int steps = 1; steps <= (isKing(piece) ? 1 : 7); steps++) {
                                    int x = u + deltas[direction].x * steps;
                                    int y = v + deltas[direction].y * steps;
                                    if (x < 0 || x > 7 || y < 0 || y > 7)
                                        break;
                                    if (board[x][y] > 0) {
                                        if (isWhite(piece) != isWhite(board[x][y])) space[x][y] = ATTACK;
                                        break;
                                    }
                                    space[x][y] = MOVE;
                                }
                            }
                        }

                        if (isKnight(piece)) {
                            for (int i = -2; i < 3; i++) {
                                for (int j = -2; j < 3; j++) {
                                    if (i == 0 || j == 0 || abs(i) == abs(j)) continue;
                                    int x = u + i;
                                    int y = v + j;
                                    if (x < 0 || x > 7 || y < 0 || y > 7)
                                        continue;
                                    if (board[x][y] == 0)
                                        space[x][y] = MOVE;
                                    else if (isWhite(piece) != isWhite(board[x][y]))
                                        space[x][y] = ATTACK;
                                }
                            }
                        }
                    }
                }
            }
        }

        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                c = (i + j) % 2 == 0 ? olc::Pixel(192, 192, 192) : olc::Pixel(128, 128, 128);

                Pieces p = board[i][j];
                Spaces a = space[i][j];

                if (a < 7) {
                    c.r += shade[a].r;
                    c.g += shade[a].g;
                    c.b += shade[a].b;
                }

                FillRectDecal(vOffset + olc::vf2d(i, j) * vGridSize, vGridSize, c);

                if (p > 0) {
                    olc::Pixel tint = olc::WHITE;
                    DrawPartialDecal(
                        vOffset + olc::vi2d(i, j) * vGridSize, decalPieces.get(),
                        olc::vi2d((p - 1) % 6, p > WHITE_PAWN ? 1 : 0) * vPieceSize,
                        vPieceSize, vScale, tint);
                }
            }
        }

        return true;
    }
};

int main() {
    ChessGame game;
    if (game.Construct(windowWidth, windowHeight, 1, 1))
        game.Start();
    return 0;
}
