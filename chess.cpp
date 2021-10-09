#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

const int windowWidth = 1024;
const int windowHeight = 768;

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

  Pieces board[8][8] = {
      {BLACK_ROOK, BLACK_PAWN, NONE, NONE, NONE, NONE, WHITE_PAWN, WHITE_ROOK},
      {BLACK_KNIGHT, BLACK_PAWN, NONE, NONE, NONE, NONE, WHITE_PAWN,
       WHITE_KNIGHT},
      {BLACK_BISHOP, BLACK_PAWN, NONE, NONE, NONE, NONE, WHITE_PAWN,
       WHITE_BISHOP},
      {BLACK_QUEEN, BLACK_PAWN, NONE, NONE, NONE, NONE, WHITE_PAWN,
       WHITE_QUEEN},
      {BLACK_KING, BLACK_PAWN, NONE, NONE, NONE, NONE, WHITE_PAWN, WHITE_KING},
      {BLACK_BISHOP, BLACK_PAWN, NONE, NONE, NONE, NONE, WHITE_PAWN,
       WHITE_BISHOP},
      {BLACK_KNIGHT, BLACK_PAWN, NONE, NONE, NONE, NONE, WHITE_PAWN,
       WHITE_KNIGHT},
      {BLACK_ROOK, BLACK_PAWN, NONE, NONE, NONE, NONE, WHITE_PAWN, WHITE_ROOK},
  };

  int space[8][8];

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
    int u = -1, v = -1;
    if (mouse.x >= 0 && mouse.y >= 0 && mouse.x < 8 * vGridSize.x &&
        mouse.y < 8 * vGridSize.y) {
      u = mouse.x / vGridSize.x;
      v = mouse.y / vGridSize.y;
    }

    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 8; j++) {
        space[i][j] = 0;
      }
    }

    Pieces piece;
    if (u > -1 && v > -1) {
      piece = board[u][v];

      space[u][v] = 5;

      if (piece == BLACK_PAWN && v < 7) {
        if (board[u][v + 1] == 0) {
          space[u][v + 1] = 2;
          if (v == 1 && board[u][v + 2] == 0) space[u][v + 2] = 2;
        }
        if (isWhite(board[u + 1][v + 1])) {
          space[u + 1][v + 1] = 2;
        }
        if (isWhite(board[u - 1][v + 1])) {
          space[u - 1][v + 1] = 2;
        }
      }

      if (piece == WHITE_PAWN && v > 0) {
        if (board[u][v - 1] == 0) {
          space[u][v - 1] = 2;
          if (v == 6 && board[u][v - 2] == 0) space[u][v - 2] = 2;
        }
        if (isBlack(board[u + 1][v + 1])) {
          space[u + 1][v + 1] = 2;
        }
        if (isBlack(board[u - 1][v + 1])) {
          space[u - 1][v + 1] = 2;
        }
      }

      olc::vi2d deltas[4] = {
          olc::vi2d(1, 0),  olc::vi2d(1, 1),  olc::vi2d(0, 1),
          olc::vi2d(-1, 1), olc::vi2d(-1, 0), olc::vi2d(-1, -1),
          olc::vi2d(0, -1), olc::vi2d(1, -1),
      };

      if (!(isPawn(piece) || isKnight(piece))) {
        for (int direction = 0; direction < 8; direction++) {
          if (direction % 2 == 0 && isRook(piece)) continue;
          if (direction % 2 != 0 && isBishop(piece)) continue;
          for (int steps = 1; steps <= isKing(piece) ? 1 : 7; steps++) {
            int x = u + deltas[direction].x * steps;
            int y = v + deltas[direction].y * steps;
            if (x < 0 || x > 7 || y < 0 || y > 7) break;
            if (board[x][y] > 0) {
              if (isBlack(piece) && isWhite(board[x][y]) ||
                  isWhite(piece) && isBlack(board[x][y]))
                space[x][y] = 1;
              break;
            }
            space[x][y] = 2;
          }
        }
      }

      if (isKnight(piece)) {
        for (int i = -2; i < 3; i++) {
          for (int j = -2; j < 3; j++) {
            if (i == 0 || j == 0 || abs(i) == abs(j)) continue;
            int x = u + i;
            int y = v + j;
            if (x < 0 || x > 7 || y < 0 || y > 7) continue;
            if (board[x][y] == 0)
              space[x][y] = 2;
            else if (isWhite(piece) && isBlack(board[x][y]) ||
                     isBlack(piece) && isWhite(board[x][y]))
              space[x][y] = 1;
          }
        }
      }
    }

    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 8; j++) {
        if ((i + j) % 2 == 0) {
          c = olc::Pixel(192, 192, 192);
        } else {
          c = olc::Pixel(128, 128, 128);
        }

        int p = board[i][j];
        int a = space[i][j];

        switch (a) {
          case 1:
            c.r += 63;
            c.g -= 63;
            c.b -= 63;
            break;
          case 2:
            c.r += 63;
            c.g += 63;
            c.b -= 63;
            break;
          case 3:
            c.r -= 63;
            c.g += 63;
            c.b -= 63;
            break;
          case 4:
            c.r -= 63;
            c.g += 63;
            c.b += 63;
            break;
          case 5:
            c.r -= 63;
            c.g -= 63;
            c.b += 63;
            break;
          case 6:
            c.r += 63;
            c.g -= 63;
            c.b += 63;
            break;
        }

        FillRectDecal(vOffset + olc::vf2d(i, j) * vGridSize, vGridSize, c);

        if (p > 0) {
          DrawPartialDecal(
              vOffset + olc::vi2d(i, j) * vGridSize, decalPieces.get(),
              olc::vi2d((p - 1) % 6, p > WHITE_PAWN ? 1 : 0) * vPieceSize,
              vPieceSize, vScale);
        }
      }
    }

    return true;
  }
};

int main() {
  ChessGame game;
  if (game.Construct(windowWidth, windowHeight, 1, 1)) game.Start();
  return 0;
}
