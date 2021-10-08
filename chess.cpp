#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

const int windowWidth = 1024;
const int windowHeight = 768;

class ChessGame : public olc::PixelGameEngine
{
public:
  ChessGame() { sAppName = "Chess"; }

private:
  std::unique_ptr<olc::Sprite> spritePieces;
  std::unique_ptr<olc::Decal> decalPieces;
  olc::vi2d vGridSize = {80, 80};
  olc::vi2d vPieceSize = {320, 320};

  int board[8][8] = {
      {11, 12, 0, 0, 0, 0, 6, 5},
      {10, 12, 0, 0, 0, 0, 6, 4},
      {9, 12, 0, 0, 0, 0, 6, 3},
      {8, 12, 0, 0, 0, 0, 6, 2},
      {7, 12, 0, 0, 0, 0, 6, 1},
      {9, 12, 0, 0, 0, 0, 6, 3},
      {10, 12, 0, 0, 0, 0, 6, 4},
      {11, 12, 0, 0, 0, 0, 6, 5},
  };

  int space[8][8];

public:
  bool
  OnUserCreate() override
  {
    // Called once at the start, so create things here

    spritePieces = std::make_unique<olc::Sprite>("./pieces.png");
    decalPieces = std::make_unique<olc::Decal>(spritePieces.get());

    return true;
  }

  bool OnUserUpdate(float fElapsedTime) override
  {

    Clear(olc::Pixel(0, 0, 0));
    SetPixelMode(olc::Pixel::MASK);

    olc::Pixel c;
    olc::vf2d vScale = {(float)vGridSize.x / (float)vPieceSize.x,
                        (float)vGridSize.y / (float)vPieceSize.y};
    olc::vi2d vOffset = {windowWidth / 2 - vGridSize.x * 4,
                         windowHeight / 2 - vGridSize.y * 4};

    olc::vi2d mouse = GetMousePos() - vOffset;
    int u = -1, v = -1;
    if (mouse.x >= 0 && mouse.y >= 0 && mouse.x < 8 * vGridSize.x && mouse.y < 8 * vGridSize.y)
    {
      u = mouse.x / vGridSize.x;
      v = mouse.y / vGridSize.y;
    }

    for (int i = 0; i < 8; i++)
    {
      for (int j = 0; j < 8; j++)
      {
        space[i][j] = 0;
      }
    }

    int m;
    if (u > -1 && v > -1)
    {
      m = board[u][v];

      space[u][v] = 5;

      if (m == 12 && v < 7)
      {
        if (board[u][v + 1] == 0)
        {
          space[u][v + 1] = 2;
          if (v == 1 && board[u][v + 2] == 0)
            space[u][v + 2] = 2;
        }
      }

      if (m == 6 && v > 0)
      {
        if (board[u][v - 1] == 0)
        {
          space[u][v - 1] = 2;
          if (v == 6 && board[u][v - 2] == 0)
            space[u][v - 2] = 2;
        }
      }

      if (m == 1 || m == 2 || m == 5 || m == 7 || m == 8 || m == 11)
      {

        for (int north = 1; north < 8; north++)
        {
          if (v - north < 0)
            break;
          if (board[u][v - north] > 0)
          {
            if (m > 6 && board[u][v - north] <= 6 ||
                m <= 6 && board[u][v - north] > 6)
              space[u][v - north] = 1;
            break;
          }
          space[u][v - north] = 2;
          if (m == 1 || m == 6)
            break;
        }

        for (int south = 1; south < 8; south++)
        {
          if (v + south > 7)
            break;
          if (board[u][v + south] > 0)
          {
            if (m > 6 && board[u][v + south] <= 6 ||
                m <= 6 && board[u][v + south] > 6)
              space[u][v + south] = 1;
            break;
          }
          space[u][v + south] = 2;
          if (m == 1 || m == 6)
            break;
        }

        for (int east = 1; east < 8; east++)
        {
          if (u + east > 7)
            break;
          if (board[u + east][v] > 0)
          {
            if (m > 6 && board[u + east][v] <= 6 ||
                m <= 6 && board[u + east][v] > 6)
              space[u + east][v] = 1;
            break;
          }
          space[u + east][v] = 2;
          if (m == 1 || m == 6)
            break;
        }

        for (int west = 1; west < 8; west++)
        {
          if (u - west < 0)
            break;
          if (board[u - west][v] > 0)
          {
            if (m > 6 && board[u - west][v] <= 6 ||
                m <= 6 && board[u - west][v] > 6)
              space[u - west][v] = 1;
            break;
          }
          space[u - west][v] = 2;
          if (m == 1 || m == 6)
            break;
        }
      }

      if (m == 1 || m == 2 || m == 3 || m == 7 || m == 8 || m == 9)
      {

        for (int northEast = 1; northEast < 8; northEast++)
        {
          if (u + northEast > 7 || v - northEast < 0)
            break;
          if (board[u + northEast][v - northEast] > 0)
          {
            if (m > 6 && board[u + northEast][v - northEast] <= 6 ||
                m <= 6 && board[u + northEast][v - northEast] > 6)
              space[u + northEast][v - northEast] = 1;
            break;
          }
          space[u + northEast][v - northEast] = 2;
          if (m == 1 || m == 6)
            break;
        }

        for (int northWest = 1; northWest < 8; northWest++)
        {
          if (u - northWest < 0 || v - northWest < 0)
            break;
          if (board[u - northWest][v - northWest] > 0)
          {
            if (m > 6 && board[u - northWest][v - northWest] <= 6 ||
                m <= 6 && board[u - northWest][v - northWest] > 6)
              space[u - northWest][v - northWest] = 1;
            break;
          }
          space[u - northWest][v - northWest] = 2;
          if (m == 1 || m == 6)
            break;
        }

        for (int southWest = 1; southWest < 8; southWest++)
        {
          if (u - southWest < 0 || v + southWest > 7)
            break;
          if (board[u - southWest][v + southWest] > 0)
          {
            if (m > 6 && board[u - southWest][v + southWest] <= 6 ||
                m <= 6 && board[u - southWest][v + southWest] > 6)
              space[u - southWest][v + southWest] = 1;
            break;
          }
          space[u - southWest][v + southWest] = 2;
          if (m == 1 || m == 6)
            break;
        }

        for (int southEast = 1; southEast < 8; southEast++)
        {
          if (u + southEast > 7 || v + southEast > 7)
            break;
          if (board[u + southEast][v + southEast] > 0)
          {
            if (m > 6 && board[u + southEast][v + southEast] <= 6 ||
                m <= 6 && board[u + southEast][v + southEast] > 6)
              space[u + southEast][v + southEast] = 1;
            break;
          }
          space[u + southEast][v + southEast] = 2;
          if (m == 1 || m == 6)
            break;
        }
      }

      if (m == 4 || m == 10)
      {
        for (int i = -2; i < 3; i++)
        {
          for (int j = -2; j < 3; j++)
          {
            if (i == 0 || j == 0 || abs(i) == abs(j))
              continue;
            if (u + i < 0 || u + i > 7 || v + j < 0 || v + j > 7)
              continue;
            if (board[u + i][v + j] == 0)
              space[u + i][v + j] = 2;
            else if (m > 6 && board[u + i][v + j] <= 6 ||
                     m <= 6 && board[u + i][v + j] > 6)
              space[u + i][v + j] = 1;
          }
        }
      }
    }

    for (int i = 0; i < 8; i++)
    {
      for (int j = 0; j < 8; j++)
      {
        if ((i + j) % 2 == 0)
        {
          c = olc::Pixel(192, 192, 192);
        }
        else
        {
          c = olc::Pixel(128, 128, 128);
        }

        int p = board[i][j];
        int a = space[i][j];

        switch (a)
        {
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

        if (p > 0)
        {
          DrawPartialDecal(vOffset + olc::vi2d(i, j) * vGridSize, decalPieces.get(),
                           olc::vi2d((p - 1) % 6, p > 6 ? 1 : 0) * vPieceSize, vPieceSize, vScale);
        }
      }
    }

    return true;
  }
};

int main()
{
  ChessGame game;
  if (game.Construct(windowWidth, windowHeight, 1, 1))
    game.Start();
  return 0;
}