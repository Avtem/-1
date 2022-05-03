#include "Tile.h"

#define lmbUp Mouse::Event::Type::LRelease
#define rmbUp Mouse::Event::Type::RRelease

GameSt *Tile::gameState = nullptr;

void Tile::parseMouse(Mouse::Event::Type mouseEv)
{
    if(revealed)
        return;

    if(mouseEv == lmbUp && drawState != DrawSt::Flag)
        reveal();
    else if (mouseEv == rmbUp)  // toggle flag
        drawState = drawState == DrawSt::Normal ? DrawSt::Flag : DrawSt::Normal;
}

void Tile::revealForLoser()
{
    if(revealed)
        return;
    
    if(drawState == DrawSt::Flag)
        drawState = obj == ObjT::Meme ? DrawSt::CorrectFlag
                                      : DrawSt::WrongFlag;
    else if(obj == ObjT::Meme)
        drawState = DrawSt::HiddenMeme;
}

void Tile::reset(bool resetFlag)
{
    numOfAdjMemes = -1;
    revealed = false;
    obj = ObjT::Number;

    if(resetFlag)
        drawState = DrawSt::Normal;
}

void Tile::softReset()
{
    revealed = false;
    obj = ObjT::Number;

    drawState = DrawSt::Normal;
}

ObjT Tile::getObj() const
{
    return obj;
}

void Tile::hide()
{
    revealed = false;
}

void Tile::setObj(ObjT type)
{
    obj = type;
}

void Tile::setNumber(int memeCount)
{
    obj = ObjT::Number;
    numOfAdjMemes = memeCount;
}

DrawSt Tile::getDrawSt() const
{
    return drawState;
}

bool Tile::isRevealed() const
{
    return revealed;
}

bool Tile::isFlagged() const
{
    return drawState == DrawSt::Flag;
}

// a tile that was not revealed and is not flagged
bool Tile::isHidden() const
{
    return revealed == false && isFlagged() == false;
}

void Tile::setFlag(bool flagged)
{
    drawState = flagged ? DrawSt::Flag : DrawSt::Normal;
}

void Tile::setBlue()
{
    drawState = DrawSt::FatalMeme;
}

void Tile::reveal()
{
    switch (obj)
    {
        case ObjT::Number:
            revealed = true;
            break;
        case ObjT::Meme:
            revealed = true;
            drawState = DrawSt::FatalMeme;
            *gameState = GameSt::GameOver;
            break;
    }
}