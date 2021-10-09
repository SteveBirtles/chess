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
  THIS,
  MOVE,
  HAZARD,
  ATTACK,
  THREAT,
  MUTUAL,
  XRAY,
  CONTROL
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

  olc::Pixel shade[10] = {
      olc::BLACK,
      olc::Pixel(63, 63, 63),    // THIS (WHITE)
      olc::Pixel(-63, 63, -63),  // MOVE (GREEN)
      olc::Pixel(-63, -63, 63),  // HAZARD (BLUE)
      olc::Pixel(63, 63, -63),   // ATTACK (YELLOW)
      olc::Pixel(63, -63, -63),  // THREAT (RED)
      olc::Pixel(63, 0, -63),    // MUTUAL (ORANGE)
      olc::Pixel(0, -63, -63),   // XRAY (DARK RED)
      olc::Pixel(63, -63, 63),   // CONTROL (MAGENTA)
      olc::Pixel(-63, 63, 63),   // DEFEND (CYAN)           // <-------- ADD THIS NEXT!

  };

  olc::vi2d deltas[8] = {
      olc::vi2d(1, 0),  olc::vi2d(1, 1),   olc::vi2d(0, 1),  olc::vi2d(-1, 1),
      olc::vi2d(-1, 0), olc::vi2d(-1, -1), olc::vi2d(0, -1), olc::vi2d(1, -1),
  };

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

  Spaces space[8][8][8][8];

  olc::vi2d moveStart = olc::vi2d(-1, -1);
  olc::vi2d moveEnd = olc::vi2d(-1, -1);

  Turns turn = WHITE;
  olc::vi2d mouse;

 public:
  bool OnUserCreate() override {
    spritePieces = std::make_unique<olc::Sprite>("./pieces.png");
    decalPieces = std::make_unique<olc::Decal>(spritePieces.get());

    recalculateSpaces();

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

    mouse = GetMousePos() - vOffset;

    olc::HWButton mouseButton = GetMouse(0);

    int u = -1, v = -1;
    if (mouse.x >= 0 && mouse.y >= 0 && mouse.x < 8 * vGridSize.x &&
        mouse.y < 8 * vGridSize.y) {
      u = mouse.x / vGridSize.x;
      v = mouse.y / vGridSize.y;
    }

    if (mouseButton.bPressed && u >= 0 && v >= 0 && u < 8 && v < 8) {
      if (moveStart.x == -1 && moveStart.y == -1) {
        if (board[u][v] != NONE) {
          Pieces piece;
          piece = board[u][v];

          if (isWhite(piece) && turn == WHITE ||
              isBlack(piece) && turn == BLACK) {
            moveStart = olc::vi2d(u, v);
          }
        }
      } else if (moveStart.x == u && moveStart.y == v) {
        moveStart = olc::vi2d(-1, -1);
      } else if (moveStart.x != -1 && moveStart.y != -1 &&
                 space[moveStart.x][moveStart.y][u][v] != INVALID) {
        board[u][v] = board[moveStart.x][moveStart.y];
        board[moveStart.x][moveStart.y] = NONE;
        if (turn == WHITE)
          turn = BLACK;
        else
          turn = WHITE;
        recalculateSpaces();

        moveStart = olc::vi2d(-1, -1);
      }
    }

    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 8; j++) {
        c = (i + j) % 2 == 0 ? olc::Pixel(192, 192, 192)
                             : olc::Pixel(96, 96, 96);

        Pieces p = board[i][j];

        int u = -1, v = -1;

        if (moveStart.x != -1 && moveStart.y != -1) {
          u = moveStart.x;
          v = moveStart.y;
        }

        if (u != -1 && v != -1) {
          Spaces a = space[u][v][i][j];
          if (a != INVALID) {
            c.r += shade[a].r;
            c.g += shade[a].g;
            c.b += shade[a].b;
          }
        }

        FillRectDecal(vOffset + olc::vf2d(i, j) * vGridSize, vGridSize, c);

        if (p > 0) {
          olc::Pixel tint = olc::WHITE;

          DrawPartialDecal(
              vOffset + olc::vi2d(i, j) * vGridSize, decalPieces.get(),
              olc::vi2d((p - 1) % 6, p > WHITE_PAWN ? 1 : 0) * vPieceSize,
              vPieceSize, vScale, tint);

          for (int u = 0; u < 8; u++) {
            for (int v = 0; v < 8; v++) {
              if (space[i][j][u][v] != INVALID) {
                c = olc::Pixel((shade[space[i][j][u][v]].r + 63) * 2,
                               (shade[space[i][j][u][v]].g + 63) * 2,
                               (shade[space[i][j][u][v]].b + 63) * 2);

                FillRectDecal(
                    vOffset + olc::vf2d(i + (float)u / 8, j + (float)v / 8) *
                                  vGridSize,
                    olc::vf2d(vGridSize.x / 8, vGridSize.y / 8), c);
              }
            }
          }
        }
      }
    }

    return true;
  }

  void recalculateSpaces() {
    for (int u = 0; u < 8; u++) {
      for (int v = 0; v < 8; v++) {
        for (int i = 0; i < 8; i++) {
          for (int j = 0; j < 8; j++) {
            space[u][v][i][j] = INVALID;
          }
        }
        Pieces piece;
        piece = board[u][v];
        if (piece == NONE) continue;

        space[u][v][u][v] = THIS;

        if (piece == BLACK_PAWN && v < 7) {
          if (board[u][v + 1] == 0) {
            space[u][v][u][v + 1] = MOVE;
            if (v == 1 && board[u][v + 2] == 0) space[u][v][u][v + 2] = MOVE;
          }
          if (u < 7) {
            if (isWhite(board[u + 1][v + 1]))
              space[u][v][u + 1][v + 1] = ATTACK;
            else if (board[u + 1][v + 1] == NONE)
              space[u][v][u + 1][v + 1] = CONTROL;
          }
          if (u > 0) {
            if (isWhite(board[u - 1][v + 1]))
              space[u][v][u - 1][v + 1] = ATTACK;
            else if (board[u - 1][v + 1] == NONE)
              space[u][v][u - 1][v + 1] = CONTROL;
          }
        }

        if (piece == WHITE_PAWN && v > 0) {
          if (board[u][v - 1] == 0) {
            space[u][v][u][v - 1] = MOVE;
            if (v == 6 && board[u][v - 2] == 0) space[u][v][u][v - 2] = MOVE;
          }
          if (u < 7) {
            if (isBlack(board[u + 1][v - 1]))
              space[u][v][u + 1][v - 1] = ATTACK;
            else if (board[u + 1][v - 1] == NONE)
              space[u][v][u + 1][v - 1] = CONTROL;
          }
          if (u > 0) {
            if (isBlack(board[u - 1][v - 1]))
              space[u][v][u - 1][v - 1] = ATTACK;
            else if (board[u - 1][v - 1] == NONE)
              space[u][v][u - 1][v - 1] = CONTROL;
          }
        }

        if (!(isPawn(piece) || isKnight(piece))) {
          for (int direction = 0; direction < 8; direction++) {
            bool xRay = false;
            if (direction % 2 != 0 && isRook(piece)) continue;
            if (direction % 2 == 0 && isBishop(piece)) continue;
            for (int steps = 1; steps <= (isKing(piece) ? 1 : 7); steps++) {
              int x = u + deltas[direction].x * steps;
              int y = v + deltas[direction].y * steps;
              if (x < 0 || x > 7 || y < 0 || y > 7) break;
              if (board[x][y] > 0) {
                if (isWhite(piece) != isWhite(board[x][y])) {
                  if (xRay) {
                    space[u][v][x][y] = XRAY;
                    break;
                  }
                  space[u][v][x][y] = ATTACK;
                }
                xRay = true;
              }
              if (!xRay) space[u][v][x][y] = MOVE;
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
                space[u][v][x][y] = MOVE;
              else if (isWhite(piece) != isWhite(board[x][y]))
                space[u][v][x][y] = ATTACK;
            }
          }
        }
      }
    }

    for (int u = 0; u < 8; u++) {
      for (int v = 0; v < 8; v++) {
        if (board[u][v] == NONE) continue;

        for (int i = 0; i < 8; i++) {
          for (int j = 0; j < 8; j++) {
            if (u == i && v == j) continue;

            if (space[u][v][i][j] == ATTACK) {
              if (space[i][j][u][v] == ATTACK) {
                space[i][j][u][v] = MUTUAL;
                space[u][v][i][j] = MUTUAL;
              } else {
                space[i][j][u][v] = THREAT;
              }
            }

            if (board[i][j] != NONE) continue;

            for (int x = 0; x < 8; x++) {
              for (int y = 0; y < 8; y++) {
                if (u == x && v == y) continue;
                if (board[x][y] == NONE) continue;
                if (isWhite(board[u][v]) == isWhite(board[x][y])) continue;
                if (!isPawn(board[x][y])) {
                  if (space[u][v][i][j] == MOVE &&
                      (space[x][y][i][j] == MOVE ||
                       space[x][y][i][j] == HAZARD)) {
                    space[u][v][i][j] = HAZARD;
                  }
                } else {
                  if (space[u][v][i][j] == MOVE &&
                      space[x][y][i][j] == CONTROL) {
                    space[u][v][i][j] = HAZARD;
                  }
                }
              }
            }
          }
        }
      }
    }
  }
};

int main() {
  ChessGame game;
  if (game.Construct(windowWidth, windowHeight, 1, 1)) game.Start();
  return 0;
}
