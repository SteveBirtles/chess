#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

const int windowWidth = 1024;
const int windowHeight = 768;

enum Pieces {
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

class ChessGame : public olc::PixelGameEngine {
 public:
  ChessGame() { sAppName = "Chess"; }

 private:
  std::unique_ptr<olc::Sprite> spritePieces;
  std::unique_ptr<olc::Decal> decalPieces;
  olc::vi2d vGridSize = {80, 80};
  olc::vi2d vPieceSize = {320, 320};

  int board[8][8] = {
      {BLACK_ROOK, BLACK_PAWN, 0, 0, 0, 0, WHITE_PAWN, WHITE_ROOK},
      {BLACK_KNIGHT, BLACK_PAWN, 0, 0, 0, 0, WHITE_PAWN, WHITE_KNIGHT},
      {BLACK_BISHOP, BLACK_PAWN, 0, 0, 0, 0, WHITE_PAWN, WHITE_BISHOP},
      {BLACK_QUEEN, BLACK_PAWN, 0, 0, 0, 0, WHITE_PAWN, WHITE_QUEEN},
      {BLACK_KING, BLACK_PAWN, 0, 0, 0, 0, WHITE_PAWN, WHITE_KING},
      {BLACK_BISHOP, BLACK_PAWN, 0, 0, 0, 0, WHITE_PAWN, WHITE_BISHOP},
      {BLACK_KNIGHT, BLACK_PAWN, 0, 0, 0, 0, WHITE_PAWN, WHITE_KNIGHT},
      {BLACK_ROOK, BLACK_PAWN, 0, 0, 0, 0, WHITE_PAWN, WHITE_ROOK},
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

    int m;
    if (u > -1 && v > -1) {
      m = board[u][v];

      space[u][v] = 5;

      if (m == BLACK_PAWN && v < 7) {
        if (board[u][v + 1] == 0) {
          space[u][v + 1] = 2;
          if (v == 1 && board[u][v + 2] == 0) space[u][v + 2] = 2;
        }
        if (board[u + 1][v + 1] > 0 && board[u + 1][v + 1] < BLACK_KING) {
          space[u + 1][v + 1] = 2;
        }
        if (board[u - 1][v + 1] > 0 && board[u - 1][v + 1] < BLACK_KING) {
          space[u - 1][v + 1] = 2;
        }
      }

      if (m == WHITE_PAWN && v > 0) {
        if (board[u][v - 1] == 0) {
          space[u][v - 1] = 2;
          if (v == 6 && board[u][v - 2] == 0) space[u][v - 2] = 2;
        }
        if (board[u + 1][v + 1] > WHITE_PAWN) {
          space[u + 1][v + 1] = 2;
        }
        if (board[u - 1][v + 1] > WHITE_PAWN) {
          space[u - 1][v + 1] = 2;
        }
      }

      if (m == WHITE_KING || m == WHITE_QUEEN || m == WHITE_ROOK ||
          m == BLACK_KING || m == BLACK_QUEEN || m == BLACK_ROOK) {
        for (int north = 1; north < 8; north++) {
          if (v - north < 0) break;
          if (board[u][v - north] > 0) {
            if (m > WHITE_PAWN && board[u][v - north] < BLACK_KING ||
                m < BLACK_KING && board[u][v - north] > WHITE_PAWN)
              space[u][v - north] = 1;
            break;
          }
          space[u][v - north] = 2;
          if (m == WHITE_KING || m == BLACK_KING) break;
        }

        for (int south = 1; south < 8; south++) {
          if (v + south > 7) break;
          if (board[u][v + south] > 0) {
            if (m > WHITE_PAWN && board[u][v + south] < BLACK_KING ||
                m < BLACK_KING && board[u][v + south] > WHITE_PAWN)
              space[u][v + south] = 1;
            break;
          }
          space[u][v + south] = 2;
          if (m == WHITE_KING || m == BLACK_KING) break;
        }

        for (int east = 1; east < 8; east++) {
          if (u + east > 7) break;
          if (board[u + east][v] > 0) {
            if (m > WHITE_PAWN && board[u + east][v] < BLACK_KING ||
                m < BLACK_KING && board[u + east][v] > WHITE_PAWN)
              space[u + east][v] = 1;
            break;
          }
          space[u + east][v] = 2;
          if (m == WHITE_KING || m == BLACK_KING) break;
        }

        for (int west = 1; west < 8; west++) {
          if (u - west < 0) break;
          if (board[u - west][v] > 0) {
            if (m > WHITE_PAWN && board[u - west][v] < BLACK_KING ||
                m < BLACK_KING && board[u - west][v] > WHITE_PAWN)
              space[u - west][v] = 1;
            break;
          }
          space[u - west][v] = 2;
          if (m == WHITE_KING || m == BLACK_KING) break;
        }
      }

      if (m == WHITE_KING || m == WHITE_QUEEN || m == WHITE_BISHOP ||
          m == BLACK_KING || m == BLACK_QUEEN || m == BLACK_BISHOP) {
        for (int northEast = 1; northEast < 8; northEast++) {
          if (u + northEast > 7 || v - northEast < 0) break;
          if (board[u + northEast][v - northEast] > 0) {
            if (m > WHITE_PAWN &&
                    board[u + northEast][v - northEast] < BLACK_KING ||
                m < BLACK_KING &&
                    board[u + northEast][v - northEast] > WHITE_PAWN)
              space[u + northEast][v - northEast] = 1;
            break;
          }
          space[u + northEast][v - northEast] = 2;
          if (m == WHITE_KING || m == BLACK_KING) break;
        }

        for (int northWest = 1; northWest < 8; northWest++) {
          if (u - northWest < 0 || v - northWest < 0) break;
          if (board[u - northWest][v - northWest] > 0) {
            if (m > WHITE_PAWN &&
                    board[u - northWest][v - northWest] < BLACK_KING ||
                m < BLACK_KING &&
                    board[u - northWest][v - northWest] > WHITE_PAWN)
              space[u - northWest][v - northWest] = 1;
            break;
          }
          space[u - northWest][v - northWest] = 2;
          if (m == WHITE_KING || m == BLACK_KING) break;
        }

        for (int southWest = 1; southWest < 8; southWest++) {
          if (u - southWest < 0 || v + southWest > 7) break;
          if (board[u - southWest][v + southWest] > 0) {
            if (m > WHITE_PAWN &&
                    board[u - southWest][v + southWest] < BLACK_KING ||
                m < BLACK_KING &&
                    board[u - southWest][v + southWest] > WHITE_PAWN)
              space[u - southWest][v + southWest] = 1;
            break;
          }
          space[u - southWest][v + southWest] = 2;
          if (m == WHITE_KING || m == BLACK_KING) break;
        }

        for (int southEast = 1; southEast < 8; southEast++) {
          if (u + southEast > 7 || v + southEast > 7) break;
          if (board[u + southEast][v + southEast] > 0) {
            if (m > WHITE_PAWN &&
                    board[u + southEast][v + southEast] < BLACK_KING ||
                m < BLACK_KING &&
                    board[u + southEast][v + southEast] > WHITE_PAWN)
              space[u + southEast][v + southEast] = 1;
            break;
          }
          space[u + southEast][v + southEast] = 2;
          if (m == WHITE_KING || m == BLACK_KING) break;
        }
      }

      if (m == 4 || m == 10) {
        for (int i = -2; i < 3; i++) {
          for (int j = -2; j < 3; j++) {
            if (i == 0 || j == 0 || abs(i) == abs(j)) continue;
            if (u + i < 0 || u + i > 7 || v + j < 0 || v + j > 7) continue;
            if (board[u + i][v + j] == 0)
              space[u + i][v + j] = 2;
            else if (m > WHITE_PAWN && board[u + i][v + j] < BLACK_KING ||
                     m < BLACK_KING && board[u + i][v + j] > WHITE_PAWN)
              space[u + i][v + j] = 1;
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
