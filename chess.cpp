#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

const int windowWidth = 1024;
const int windowHeight = 768;

class ChessGame : public olc::PixelGameEngine {
 public:
  ChessGame() { sAppName = "Chess"; }

 private:
  std::unique_ptr<olc::Sprite> spritePieces;
  std::unique_ptr<olc::Decal> decalPieces;
  olc::vi2d vGridSize = {80, 80};
  olc::vi2d vPieceSize = {320, 320};

  int board[8][8] 

 public:
  bool OnUserCreate() override {
    // Called once at the start, so create things here

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
    olc::vf2d vOffset = {(float)windowWidth / 2 - vGridSize.x * 4,
                         (float)windowHeight / 2 - vGridSize.y * 4};

    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 8; j++) {
        if ((i + j) % 2 == 0) {
          c = olc::Pixel(192, 192, 192);
        } else {
          c = olc::Pixel(128, 128, 128);
        }
        FillRectDecal(vOffset + olc::vf2d(i, j) * vGridSize, vGridSize, c);
      }
    }

    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 8; j++) {

        int p = (j * 8 + i) % 12 + 1;

        DrawPartialDecal(vOffset + olc::vi2d(i, j) * vGridSize, decalPieces.get(),
                         olc::vi2d((p - 1) % 6, p > 6 ? 1 : 0) * vPieceSize, vPieceSize, vScale);
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