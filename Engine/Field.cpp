#include "Field.h"
#include "Mouse.h"
#include "SpriteCodex.h"
#include <random>
#include "Image.h"

std::random_device Field::randDevice;
std::mt19937 Field::mt(Field::randDevice());
std::uniform_int_distribution<int> Field::intDistr(0, 999999);

Field::Field(Graphics& Gfx, int TilesInW, int TilesInH, float MemesFillness)
    :gfx(Gfx),
    tilesInW(TilesInW),
    tilesInH(TilesInH),
    memesFillness(MemesFillness)
{
    loadImages();
    
    tiles = new Tile[tilesInW * TilesInH];
    for(int i = 0; i < getTileCount(); ++i)
        tiles[i].index ={i %tilesInW, i /tilesInW};

    reset();
}

Field::~Field()
{
    for(auto* e : imgNumbers)
        delete e;

    delete imgHiddenTile;
    delete imgFlagged;
    delete imgMine;
    delete imgCorrectMeme;
    delete imgBoom;
    delete imgWrongFlag;
    delete [] tiles;
}

void Field::draw() const
{
    // draw borderf
    Vei2 dim = getSizeInPx();
    RectI boardR{drawOff, dim.x, dim.y};
    gfx.DrawRect(boardR.GetExpanded(borderThickness), Colors::Gray);
    
    // draw field
    gfx.DrawRect(boardR, SpriteCodex::baseColor);

    for(int y=0; y < tilesInH; ++y)
    {
        for(int x=0; x < tilesInW; ++x)
        {
            const Tile& tile = tiles[x +y*tilesInW];
            Vei2 tilePos{x,y};
            tilePos *= SpriteCodex::tileSize;
            tilePos += drawOff;
            
            switch (tile.getDrawSt())
            {
                case DrawSt::Normal:      gfx.drawImage(tilePos, *imgHiddenTile); break;
                case DrawSt::Flagged:     gfx.drawImage(tilePos, *imgFlagged);    break;
                // after win | lose
                case DrawSt::Sneaky:      gfx.drawImage(tilePos, *imgMine);       break;
                case DrawSt::FlaggedGood: gfx.drawImage(tilePos, *imgCorrectMeme);break;
                case DrawSt::FlaggedBad:  gfx.drawImage(tilePos, *imgWrongFlag);  break;
                case DrawSt::FatalMeme:   gfx.drawImage(tilePos, *imgBoom);       break;
            }

            if(tile.isRevealed() && tile.getObj() == ObjT::Number)
                gfx.drawImage(tilePos, *imgNumbers.at(tile.numOfAdjMemes));
        }
    }
}

void Field::parseMouse(Mouse::Event event, Vei2& offset)
{
    auto rightBot = getSizeInPx() + offset;
    // sanity check
    if(event.GetPosX() < offset.x   || event.GetPosY() < offset.y
    || event.GetPosX() > rightBot.x || event.GetPosY() > rightBot.y)
        return;
    
    // get index
    Vei2 tileInd = (event.GetPosVei() -offset) /SpriteCodex::tileSize;

    if(firstClick)
    {
        if(event.GetType() == rmbUp)
            return;
        else if(generationType == random)
            parseFirstClick(tileInd, event.GetType());
        else if(generationType == solvable100)
            ai->regenerateUntilSolvable100(tileAt(tileInd));
        else if(generationType == unsolvableForAI)
            ai->regenerateUntilAIcantSolve(tileAt(tileInd));
        else if(generationType == unsolvable100)
            ai->regenerateUntilUnsolvable100percent(tileAt(tileInd));
        else if(generationType == without100Unsolved)
            ai->regenerateWithout100Unsolv(tileAt(tileInd));
        else if(generationType == GenType::loadFromFile)
        {
            loadFromFile(L"img/maps/1 3 1.bmp");
            clickTile(tileInd, event.GetType());
        }
        
        firstClick = false;
    }
    else   
        clickTile(tileInd, event.GetType());
}

void Field::parseFirstClick(Vei2 tileInd, Mouse::Event::Type eventType)
{    
    if(!tileIsValid(tileInd) || eventType == rmbUp)
        return;

    ai->regenerateUntilClickedTileIsSave(tileAt(tileInd));

    if(generationType == random)     // normal randomized game
        clickTile(tileInd, eventType);
}

void Field::clickTile(Vei2 index, Mouse::Event::Type eventType)
{
    if(!tileIsValid(index) || *Tile::gameState == GameSt::GameOver)
        return;

    tileAt(index).parseMouse(eventType);
    if(*Tile::gameState == GameSt::GameOver)
        return;
    
    if (eventType == lmbUp && *Tile::gameState == GameSt::Running
         &&  0 == tiles[index.x +index.y *tilesInW].numOfAdjMemes)
    {
        revealAdjTiles(index);
    }

    if(checkWinCondition())
        *Tile::gameState = GameSt::Win;
}

bool Field::checkWinCondition() const
{
    int hiddenTilesCount = 0;
    for(int i=0; i < getTileCount(); ++i)
        hiddenTilesCount += !tiles[i].isRevealed() ? 1 : 0;

    return getMemeCount() == hiddenTilesCount;
}

int Field::getTileCount() const
{
    return tilesInW * tilesInH;
}

void Field::loadImages()
{
    imgHiddenTile = new Image(L"img/hidden.bmp");
    imgFlagged = new Image(L"img/flagged.bmp");
    imgCorrectMeme = new Image(L"img/correctMeme.bmp");
    imgMine = new Image(L"img/meme.bmp");
    imgWrongFlag = new Image(L"img/wrongFlag.bmp");
    imgBoom = new Image(L"img/boom.bmp");

    imgNumbers.emplace_back(new Image(L"img/0.bmp"));
    imgNumbers.emplace_back(new Image(L"img/1.bmp"));
    imgNumbers.emplace_back(new Image(L"img/2.bmp"));
    imgNumbers.emplace_back(new Image(L"img/3.bmp"));
    imgNumbers.emplace_back(new Image(L"img/4.bmp"));
    imgNumbers.emplace_back(new Image(L"img/5.bmp"));
    imgNumbers.emplace_back(new Image(L"img/6.bmp"));
    imgNumbers.emplace_back(new Image(L"img/7.bmp"));
    imgNumbers.emplace_back(new Image(L"img/8.bmp"));
}

void Field::unfoldTheField()
{
    for (int i = 0; i < getTileCount(); ++i)
        tiles[i].unfold();
}

void Field::revealAdjTiles(const Vei2& pos)
{
    // terminate condition for the recursion
    if(0 != tileAt(pos).numOfAdjMemes || tileAt(pos).isFlagged())
        return;

    // reveal all 9 tiles around POS
    Vei2 ind{pos.x -1, pos.y -1};
    for (int i = 0; i < 9; ++i, ++ind.x)
    {
        if (i && i %3 == 0)
        {
            ++ind.y;
            ind.x = pos.x-1;
        }

        if (tileIsValid(ind) == false)
            continue;

        Tile& tile = tileAt(ind);
        if (tile.getObj() == ObjT::Number && tile.isBlack())
        {
            tile.reveal();
            revealAdjTiles(ind);    // recurse!
        }
    }
}

void Field::putMemes()
{
    for (int i = 0; i < getMemeCount(); ++i)
    {
        int x{ 0 };
        int y{ 0 };
        do
        {
            x = getRand() % tilesInW;
            y = getRand() % tilesInH;
        } while (tiles[x +y*tilesInW].getObj() == ObjT::Meme);
        tiles[x +y*tilesInW].setMeme();
    }
}

bool Field::tileIsValid(const Vei2& index) const
{
    return index.x >= 0 && index.y >= 0 && index.x < tilesInW && index.y < tilesInH;
}

void Field::putNumbers()
{
    for (int y=0; y < tilesInH; ++y)
    {
        for(int x=0; x < tilesInW; ++x)
        {
            // don't count for memes
            if(tiles[x +y *tilesInW].getObj() == ObjT::Meme)
                continue;

            int count = 0;
            int nx = x-1;
            int ny = y-1;
            for(int i=0; i < 9; ++i, ++nx)
            {
                if(i && i %3 == 0)
                {
                    ++ny;
                    nx = x-1;
                }

                if(tileIsValid({nx,ny}) == false)
                    continue;

                count += tiles[nx +ny*tilesInW].getObj() == ObjT::Meme ? 1 : 0;
            }

            tiles[x +y*tilesInW].setNumber(count);
        }
    }
}

Tile& Field::tileAt(const Vei2& index) const
{
    return tiles[index.x +index.y *tilesInW];
}

void Field::hideEverything()
{
    for(int i = 0; i < getTileCount(); ++i)
    {
        tiles[i].hide();
        tiles[i].setFlag(false);
    }
}

void Field::reset(bool resetFlags, bool randomize)
{
    if(Tile::gameState)
        *Tile::gameState = GameSt::Running;

    firstClick = true;

    if(!randomize)
    {
        for (int i = 0; i < getTileCount(); ++i)
            tiles[i].softReset();

        return;
    }

    for (int i = 0; i < getTileCount(); ++i)
        tiles[i].reset(resetFlags);

    putMemes();
    putNumbers();
}

void Field::loadFromFile(const std::wstring& path)
{
    // do the normal things that reset does
    if(Tile::gameState)
        *Tile::gameState = GameSt::Running;
    
    firstClick = true;
    for(int i=0; i < getTileCount(); ++i)
        tiles[i].reset();

    Image img(path);
    Vei2 dim = {img.getWidth(), img.getHeight()};
    for(int i=0; i < dim.x * dim.y; ++i)
        if(img.getPixel(i %dim.x, i /dim.x).dword == 0x000000)
            tiles[i].setMeme();
    putNumbers();
}

void Field::setDrawingOffset(Vei2 offset)
{
    drawOff = offset;
}

Vei2 Field::getSizeInPx() const
{
    return {tilesInW * SpriteCodex::tileSize, tilesInH *SpriteCodex::tileSize};
}

int Field::getRand() const
{
    return intDistr(mt);
}

int Field::getMemeCount() const
{
    if(generationType == GenType::loadFromFile)
    {
        int count = 0;
        for(int i=0; i < getTileCount(); ++i)
            if(tiles[i].getObj() == ObjT::Meme)
                ++count;

        return count;
    }

    return int(getTileCount() *memesFillness);
}

int Field::getRemainingMemeCount() const
{
    int flaggedCount = 0;
    for (int i = 0; i < getTileCount(); ++i)
    {
        bool isFlagged = tiles[i].getDrawSt() == DrawSt::Flagged
                      || tiles[i].getDrawSt() == DrawSt::FlaggedGood;
        flaggedCount += isFlagged ? 1 : 0;
    }

    return getMemeCount() -flaggedCount;
}

bool Field::willBeFirstClick() const
{
    return firstClick;
}

void Field::nextGenType()
{
    if(willBeFirstClick() && generationType +1 <= GenType::LAST)
        generationType = (GenType)(generationType +1);
}

void Field::prevGenType()
{
    if(willBeFirstClick() && generationType -1 >= GenType::FIRST)
        generationType = (GenType)(generationType -1);
}

GenType Field::getGenType() const
{
    return generationType;
}

void Field::setAI(AI *ai_ptr)
{
    ai = ai_ptr;
}

