#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

const int windowWidth = 640;
const int windowHeight = 480;

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
  START,
  SAFE_MOVE,
  HAZARDOUS_MOVE,
  UNDEFENDED_ATTACK,
  THREAT,
  DEFENDED_MUTUAL_THREAT,
  XRAY,
  CONTROL,
  DEFENCE,
  CHECK,
  DEFENDED_ATTACK,
  UNDEFENDED_MUTUAL_THREAT
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
  olc::vi2d vGridSize = {windowHeight / 9, windowHeight / 9};
  olc::vi2d vPieceSize = {320, 320};

  olc::Pixel shade[13] = {
      olc::BLACK,
      olc::Pixel(-63, 0, 0),     // START (DARK CYAN)
      olc::Pixel(-63, 63, 63),   // SAFE MOVE (CYAN)
      olc::Pixel(0, -63, 63),    // HAZARDOUS MOVE (PURPLE)
      olc::Pixel(-63, 63, -63),  // UNDEFENDED ATTACK (GREEN)
      olc::Pixel(63, -63, -63),  // THREAT (RED)
      olc::Pixel(0, -63, -63),   // DEFENDED MUTUAL THREAT (DARK RED)
      olc::Pixel(-63, 0, -63),   // XRAY (DARK GREEN)
      olc::Pixel(-63, -63, 0),   // CONTROL (DARK BLUE)
      olc::Pixel(-63, -63, 63),  // DEFENCE (BLUE)
      olc::Pixel(63, 63, -63),   // CHECK (YELLOW)
      olc::Pixel(63, -63, 63),   // DEFENDED ATTACK (MAGENTA)
      olc::Pixel(63, 0, -63),    // UNDEFENDED MUTUAL THREAT (ORANGE)
  };

  std::string spaceWord[13] = {
      "",
      "Selected Piece",
      "Safe Move",
      "Hazardous Move",
      "Undefended Attack",
      "Threat*",
      "Defended Mutal Threat",
      "X-Ray Attack*",
      "Control*",
      "Defence*",
      "Check*",
      "Defended Attack",
      "Undefended Mutual Threat",
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

  Turns turn = WHITE;
  olc::vi2d mouse;
  bool useTurns = false;

  bool calculateAttacks = true;
  bool calculateDefences = true;
  bool calculateXrays = true;
  bool calculateThreats = true;
  bool calculateControlsAndHazards = true;
  bool calculateDefendedAttacks = true;

  bool miniSpaces = false;

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

    olc::vf2d vScale = {(float)vGridSize.x / (float)vPieceSize.x,
                        (float)vGridSize.y / (float)vPieceSize.y};
    olc::vi2d vOffset = {windowWidth / 2 - vGridSize.x * 4,
                         windowHeight / 2 - vGridSize.y * 4};

    if (GetKey(olc::Key::ESCAPE).bPressed) {
      for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
          board[i][j] = NONE;
        }
      }
      moveStart = olc::vi2d(-1, -1);
      recalculateSpaces();
    }

    mouse = GetMousePos() - vOffset;

    olc::HWButton mouseButton = GetMouse(0);

    int u = -1, v = -1;
    if (mouse.x >= 0 && mouse.y >= 0 && mouse.x < 8 * vGridSize.x &&
        mouse.y < 8 * vGridSize.y) {
      u = mouse.x / vGridSize.x;
      v = mouse.y / vGridSize.y;

      if (GetKey(olc::Key::SPACE).bPressed && useTurns) {
        if (turn == WHITE)
          turn = BLACK;
        else
          turn = WHITE;
      }

      if (GetKey(olc::Key::BACK).bPressed) {
        board[u][v] = NONE;
        if (u == moveStart.x && v == moveStart.y) moveStart = olc::vi2d(-1, -1);
        recalculateSpaces();
      }

      if (GetKey(olc::Key::F1).bPressed) {
        board[u][v] = BLACK_KING;
        recalculateSpaces();
      }

      if (GetKey(olc::Key::F2).bPressed) {
        board[u][v] = BLACK_QUEEN;
        recalculateSpaces();
      }

      if (GetKey(olc::Key::F3).bPressed) {
        board[u][v] = BLACK_BISHOP;
        recalculateSpaces();
      }

      if (GetKey(olc::Key::F4).bPressed) {
        board[u][v] = BLACK_KNIGHT;
        recalculateSpaces();
      }

      if (GetKey(olc::Key::F5).bPressed) {
        board[u][v] = BLACK_ROOK;
        recalculateSpaces();
      }

      if (GetKey(olc::Key::F6).bPressed) {
        board[u][v] = BLACK_PAWN;
        recalculateSpaces();
      }

      if (GetKey(olc::Key::K1).bPressed) {
        board[u][v] = WHITE_KING;
        recalculateSpaces();
      }

      if (GetKey(olc::Key::K2).bPressed) {
        board[u][v] = WHITE_QUEEN;
        recalculateSpaces();
      }

      if (GetKey(olc::Key::K3).bPressed) {
        board[u][v] = WHITE_BISHOP;
        recalculateSpaces();
      }

      if (GetKey(olc::Key::K4).bPressed) {
        board[u][v] = WHITE_KNIGHT;
        recalculateSpaces();
      }

      if (GetKey(olc::Key::K5).bPressed) {
        board[u][v] = WHITE_ROOK;
        recalculateSpaces();
      }

      if (GetKey(olc::Key::K6).bPressed) {
        board[u][v] = WHITE_PAWN;
        recalculateSpaces();
      }
    }

    if (GetKey(olc::Key::A).bPressed) {
      calculateAttacks = !calculateAttacks;
      recalculateSpaces();
    }

    if (GetKey(olc::Key::T).bPressed) {
      calculateThreats = !calculateThreats;
      recalculateSpaces();
    }

    if (GetKey(olc::Key::H).bPressed) {
      calculateControlsAndHazards = !calculateControlsAndHazards;
      recalculateSpaces();
    }

    if (GetKey(olc::Key::D).bPressed) {
      calculateDefences = !calculateDefences;
      recalculateSpaces();
    }

    if (GetKey(olc::Key::F).bPressed) {
      calculateDefendedAttacks = !calculateDefendedAttacks;
      recalculateSpaces();
    }

    if (GetKey(olc::Key::X).bPressed) {
      calculateXrays = !calculateXrays;
      recalculateSpaces();
    }

    if (GetKey(olc::Key::R).bPressed) {
      calculateAttacks = true;
      calculateDefences = true;
      calculateThreats = true;
      calculateXrays = true;
      calculateControlsAndHazards = true;
      calculateDefendedAttacks = true;
      recalculateSpaces();
    }

    if (GetKey(olc::Key::N).bPressed) {
      calculateAttacks = false;
      calculateDefences = false;
      calculateThreats = false;
      calculateXrays = false;
      calculateControlsAndHazards = false;
      calculateDefendedAttacks = false;
      recalculateSpaces();
    }

    if (mouseButton.bPressed && u >= 0 && v >= 0 && u < 8 && v < 8) {
      if (moveStart.x == -1 && moveStart.y == -1) {
        if (board[u][v] != NONE) {
          Pieces piece;
          piece = board[u][v];

          if (!useTurns || (isWhite(piece) && turn == WHITE ||
                            isBlack(piece) && turn == BLACK)) {
            moveStart = olc::vi2d(u, v);
          }
        }
      } else if (moveStart.x == u && moveStart.y == v) {
        moveStart = olc::vi2d(-1, -1);
      } else if (moveStart.x != -1 && moveStart.y != -1 &&
                 (space[moveStart.x][moveStart.y][u][v] == SAFE_MOVE ||
                  space[moveStart.x][moveStart.y][u][v] == UNDEFENDED_ATTACK ||
                  space[moveStart.x][moveStart.y][u][v] ==
                      DEFENDED_MUTUAL_THREAT ||
                  space[moveStart.x][moveStart.y][u][v] ==
                      UNDEFENDED_MUTUAL_THREAT ||
                  space[moveStart.x][moveStart.y][u][v] == HAZARDOUS_MOVE ||
                  space[moveStart.x][moveStart.y][u][v] == DEFENDED_ATTACK)) {
        board[u][v] = board[moveStart.x][moveStart.y];
        board[moveStart.x][moveStart.y] = NONE;
        if (useTurns) {
          if (turn == WHITE)
            turn = BLACK;
          else
            turn = WHITE;
        }
        recalculateSpaces();

        moveStart = olc::vi2d(-1, -1);
      }
    }

    for (int i = 7; i >= 0; i--) {
      for (int j = 0; j < 8; j++) {
        olc::Pixel squareColour = (i + j) % 2 == 0 ? olc::Pixel(192, 192, 192)
                                                   : olc::Pixel(96, 96, 96);

        int startX = -1;
        int startY = -1;
        if (moveStart.x != -1 && moveStart.y != -1) {
          startX = moveStart.x;
          startY = moveStart.y;
        }

        int mouseX = mouse.x / vGridSize.x;
        int mouseY = mouse.y / vGridSize.y;

        FillRectDecal(vOffset + olc::vf2d(i, j) * vGridSize, vGridSize,
                      squareColour);

        if (startX != -1 && startY != -1) {
          Spaces a = space[startX][startY][i][j];
          if (a != INVALID) {
            olc::Pixel borderColour =
                olc::Pixel((shade[a].r + 63) * 2, (shade[a].g + 63) * 2,
                           (shade[a].b + 63) * 2);

            FillRectDecal(vOffset + olc::vf2d(i, j) * vGridSize, vGridSize,
                          borderColour);

            squareColour.r += shade[a].r;
            squareColour.g += shade[a].g;
            squareColour.b += shade[a].b;

            FillRectDecal(
                vOffset + olc::vf2d(i, j) * vGridSize + olc::vi2d(5, 5),
                vGridSize - olc::vi2d(10, 10), squareColour);

            if (mouseX == i && mouseY == j) {
              DrawStringPropDecal(vOffset + olc::vi2d(0, -10), spaceWord[a],
                                  squareColour, olc::vf2d(1, 1));
            }
          }
        }

        if (startX == -1 && startY == -1 && mouse.x >= 0 && mouse.y >= 0 &&
            mouse.x < 8 * vGridSize.x && mouse.y < 8 * vGridSize.y) {
          if (mouseX != -1 && mouseY != -1) {
            Spaces a = space[mouseX][mouseY][i][j];
            if (a != INVALID) {
              olc::Pixel highlightColour =
                  (i + j) % 2 == 0
                      ? olc::Pixel(192 + shade[a].r, 192 + shade[a].g,
                                   192 + shade[a].b)
                      : olc::Pixel(96 + shade[a].r, 96 + shade[a].g,
                                   96 + shade[a].b);

              FillRectDecal(vOffset + olc::vf2d(i, j) * vGridSize, vGridSize,
                            highlightColour);
            }
          }
        }

        Pieces p = board[i][j];
        if (p > 0) {
          olc::Pixel tint = olc::WHITE;

          DrawPartialDecal(
              vOffset + olc::vi2d(i, j) * vGridSize, decalPieces.get(),
              olc::vi2d((p - 1) % 6, p > WHITE_PAWN ? 1 : 0) * vPieceSize,
              vPieceSize, vScale, tint);

          if (miniSpaces) {
            for (int u = 0; u < 8; u++) {
              for (int v = 0; v < 8; v++) {
                Spaces a = space[i][j][u][v];
                if (a != INVALID) {
                  olc::Pixel miniColour =
                      olc::Pixel((shade[a].r + 63) * 2, (shade[a].g + 63) * 2,
                                 (shade[a].b + 63) * 2);

                  FillRectDecal(
                      vOffset + olc::vf2d(i + (float)u / 8, j + (float)v / 8) *
                                    vGridSize,
                      olc::vf2d(vGridSize.x / 8, vGridSize.y / 8), miniColour);
                }
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

        space[u][v][u][v] = START;

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
                    if (calculateXrays) space[u][v][x][y] = XRAY;
                    break;
                  }
                  if (calculateAttacks) space[u][v][x][y] = UNDEFENDED_ATTACK;
                } else {
                  if (!xRay && !isKing(board[x][y])) {
                    if (calculateDefences) space[u][v][x][y] = DEFENCE;
                  }
                }
                xRay = true;
              }
              if (!xRay) space[u][v][x][y] = SAFE_MOVE;
            }
          }
        }

        if (piece == BLACK_PAWN && v < 7) {
          if (board[u][v + 1] == 0) {
            space[u][v][u][v + 1] = SAFE_MOVE;
            if (v == 1 && board[u][v + 2] == 0)
              space[u][v][u][v + 2] = SAFE_MOVE;
          }
          if (u < 7) {
            if (isWhite(board[u + 1][v + 1])) {
              if (calculateAttacks)
                space[u][v][u + 1][v + 1] = UNDEFENDED_ATTACK;
            } else if (isBlack(board[u + 1][v + 1]) &&
                       !isKing(board[u + 1][v + 1])) {
              if (calculateDefences) space[u][v][u + 1][v + 1] = DEFENCE;
            } else if (board[u + 1][v + 1] == NONE) {
              if (calculateControlsAndHazards)
                space[u][v][u + 1][v + 1] = CONTROL;
            }
          }
          if (u > 0) {
            if (isWhite(board[u - 1][v + 1])) {
              if (calculateAttacks)
                space[u][v][u - 1][v + 1] = UNDEFENDED_ATTACK;
            } else if (isBlack(board[u - 1][v + 1]) &&
                       !isKing(board[u - 1][v + 1])) {
              if (calculateDefences) space[u][v][u - 1][v + 1] = DEFENCE;
            } else if (board[u - 1][v + 1] == NONE) {
              if (calculateControlsAndHazards)
                space[u][v][u - 1][v + 1] = CONTROL;
            }
          }
        }

        if (piece == WHITE_PAWN && v > 0) {
          if (board[u][v - 1] == 0) {
            space[u][v][u][v - 1] = SAFE_MOVE;
            if (v == 6 && board[u][v - 2] == 0)
              space[u][v][u][v - 2] = SAFE_MOVE;
          }
          if (u < 7) {
            if (isBlack(board[u + 1][v - 1])) {
              if (calculateAttacks)
                space[u][v][u + 1][v - 1] = UNDEFENDED_ATTACK;
            } else if (isWhite(board[u + 1][v - 1]) &&
                       !isKing(board[u + 1][v - 1])) {
              if (calculateDefences) space[u][v][u + 1][v - 1] = DEFENCE;
            } else if (board[u + 1][v - 1] == NONE) {
              if (calculateControlsAndHazards)
                space[u][v][u + 1][v - 1] = CONTROL;
            }
          }
          if (u > 0) {
            if (isBlack(board[u - 1][v - 1])) {
              if (calculateAttacks)
                space[u][v][u - 1][v - 1] = UNDEFENDED_ATTACK;
            } else if (isWhite(board[u - 1][v - 1]) &&
                       !isKing(board[u - 1][v - 1])) {
              if (calculateDefences) space[u][v][u - 1][v - 1] = DEFENCE;
            } else if (board[u - 1][v - 1] == NONE) {
              if (calculateControlsAndHazards)
                space[u][v][u - 1][v - 1] = CONTROL;
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
                space[u][v][x][y] = SAFE_MOVE;
              else if (isWhite(piece) != isWhite(board[x][y])) {
                if (calculateAttacks) space[u][v][x][y] = UNDEFENDED_ATTACK;
              } else if (!isKing(board[x][y])) {
                if (calculateDefences) space[u][v][x][y] = DEFENCE;
              }
            }
          }
        }
      }
    }

    if (calculateThreats) {
      for (int u = 0; u < 8; u++) {
        for (int v = 0; v < 8; v++) {
          if (board[u][v] == NONE) continue;

          for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
              if (u == i && v == j) continue;

              if (space[u][v][i][j] == UNDEFENDED_ATTACK) {
                if (isKing(board[i][j])) {
                  space[u][v][i][j] = CHECK;
                  space[i][j][u][v] = THREAT;
                } else if (space[i][j][u][v] == UNDEFENDED_ATTACK) {
                  space[i][j][u][v] = UNDEFENDED_MUTUAL_THREAT;
                  space[u][v][i][j] = UNDEFENDED_MUTUAL_THREAT;
                } else {
                  space[i][j][u][v] = THREAT;
                }
              }
            }
          }
        }
      }
    }

    if (calculateControlsAndHazards) {
      for (int u = 0; u < 8; u++) {
        for (int v = 0; v < 8; v++) {
          if (board[u][v] == NONE) continue;

          for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
              if (u == i && v == j) continue;

              for (int x = 0; x < 8; x++) {
                for (int y = 0; y < 8; y++) {
                  if (u == x && v == y) continue;
                  if (board[x][y] == NONE) continue;
                  if (isWhite(board[u][v]) == isWhite(board[x][y])) continue;
                  if (isPawn(board[x][y])) {
                    if (space[x][y][i][j] == CONTROL &&
                        space[u][v][i][j] == SAFE_MOVE) {
                      space[u][v][i][j] = HAZARDOUS_MOVE;
                    }
                  } else {
                    if ((space[x][y][i][j] == SAFE_MOVE ||
                         space[x][y][i][j] == HAZARDOUS_MOVE) &&
                        space[u][v][i][j] == SAFE_MOVE) {
                      space[u][v][i][j] = HAZARDOUS_MOVE;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }

    if (calculateDefendedAttacks) {
      for (int u = 0; u < 8; u++) {
        for (int v = 0; v < 8; v++) {
          if (board[u][v] == NONE) continue;

          for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
              if (u == i && v == j) continue;
              if (board[i][j] == NONE) continue;

              for (int x = 0; x < 8; x++) {
                for (int y = 0; y < 8; y++) {
                  if (board[x][y] == NONE) continue;
                  if (u == x && v == y) continue;
                  if (space[x][y][i][j] == DEFENCE) {
                    if (space[u][v][i][j] == UNDEFENDED_ATTACK)
                      space[u][v][i][j] = DEFENDED_ATTACK;
                    if (space[u][v][i][j] == UNDEFENDED_MUTUAL_THREAT)
                      space[u][v][i][j] = DEFENDED_MUTUAL_THREAT;
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
