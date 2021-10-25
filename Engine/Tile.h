#pragma once

#include "SpriteCodex.h"
#include "Mouse.h"

enum class ObjT // object type (the actual thing that resides in the tile)
{
    Meme,
    Number // the tile contains number of adj.memes
};
enum class DrawSt // drawing state
{
    Normal,
    Flag,
    // after losing:
    FatalMeme,
    CorrectFlag,
    WrongFlag,
    HiddenMeme
};
enum class ClickRes
{
    Nothing = 0,
    GameOver,
    GameWin
};

class Tile
{
public:
    int numOfAdjMemes = -1; // if no number!
//////////// member functions
    ClickRes parseMouse(Mouse::Event::Type mouseEv); // returns TRUE if you hit a meme
    void revealForLoser();
    void reset();
    ObjT Tile::getObj() const;
    void setObj(ObjT type);
    void setNumber(int memeCount);
    DrawSt Tile::getDrawSt() const;
    bool isRevealed() const;
    void reveal();

private:
    ObjT obj = ObjT::Number;
    DrawSt drawState = DrawSt::Normal;
    bool revealed = false;

    static constexpr int dimension = SpriteCodex::tileSize;
};

