#include "AI.h"
#include "Field.h"

AI::AI(Field& inField)
	:field(inField)
{
}

bool AI::areaIsSolved(const Vei2& centerTile)
{
    Tile& t = tileAt(centerTile);
    if(!t.isRevealed())
        return false;

    return getAdjFlagCount(centerTile) == t.numOfAdjMemes;
}

void AI::randClick() const
{
	int x = field.getRand() %field.tilesInW;
	int y = field.getRand() %field.tilesInH;

    field.clickTile({x,y}, Mouse::Event::Type::LRelease);
}

void AI::flagObvious()
{
    for(int i=0; i < field.getTilesCount(); ++i)
    {
        Vei2 ind = { i %field.tilesInW, i /field.tilesInW };
        Tile& t = field.tiles[i];

        auto hidTiles = getHiddenTiles(ind, true);
        if(t.isRevealed() && hidTiles.size() == size_t(t.numOfAdjMemes))
        {
            for(Tile* adjT : hidTiles)
                adjT->setFlag(true);
        }
    }
}

void AI::afterFlag()
{
    for (int i = 0; i < field.getTilesCount(); ++i)
    {
        Vei2 ind = { i %field.tilesInW, i /field.tilesInW };
        Tile& t = field.tiles[i];

        if (t.isRevealed() && t.numOfAdjMemes >= 0)
        {
            auto hidTiles = getHiddenTiles(ind, true);

            // check if the area is complete
            int flaggedCount = 0;
            for(const Tile* adjT : hidTiles)
                if(adjT->getDrawSt() == DrawSt::Flag)
                   ++flaggedCount;

            // yay, we can reveal other tiles!
            if(flaggedCount == t.numOfAdjMemes)
            {
                for(Tile* adjT : hidTiles)
                {
                    if(!adjT->isRevealed() && adjT->getDrawSt() != DrawSt::Flag)
                    {
                        field.clickTile(adjT->index, Mouse::Event::Type::LRelease);
                        field.checkWinCondition();
                    }
                }
            }
        }
    }
}

void AI::traitor()
{
    afterFlag();

    for(int i=0; i < field.getTilesCount(); ++i)
    {
        Tile& t = field.tiles[i];   // our main tile (that we "click")
        if(!t.isRevealed() || areaIsSolved(t.index))
            continue;

        // find potencial unsolved adjacent tiles
        auto adjTiles = getAdjTiles(t.index);
        for(const Tile* adj : adjTiles)
        {
            if(!adj->isRevealed() || areaIsSolved(adj->index))
                continue;

            // if t overlaps adj.cell, reveal all unrevealed tiles for t
            auto overlap = getHidOverlapTiles(t.index, adj->index);
            auto adjHidTiles = getHiddenTiles(adj->index, false);
            excludeTiles(adjHidTiles, overlap);
            
            // t(traitor) = solvable, adj = not
            if(solvableWithoutTiles(&t, overlap)
            && impossibleWithoutTiles(adj, overlap) )
            {
                auto nonOverlap = getNonOverlapTiles(&t, adj);
                for(Tile* tNonOv : nonOverlap)
                    field.clickTile(tNonOv->index, Mouse::Event::Type::LRelease);
            }
        }
    }
}

void AI::iKnowWhereTheOthers()
{
    for (int i = 0; i < field.getTilesCount(); ++i)
    {
        Vei2 ind = { i %field.tilesInW, i /field.tilesInW };
        Tile& t = field.tiles[i]; // our main tile (that we "click")

        if (!t.isRevealed() || areaIsSolved(ind))
            continue;

        // find unsolved adjacent area
        auto adjTiles = getAdjTiles(ind);
        for (const Tile* adj : adjTiles)
        {
            if (!adj->isRevealed() || areaIsSolved(adj->index))
                continue;

            auto overlap = getHidOverlapTiles(t.index, adj->index);
            auto tHid = getHiddenTiles(t.index, false);
            auto adjHid = getHiddenTiles(adj->index, false);
            excludeTiles(adjHid, overlap);
            
            // t is solvable (with only overlap) && adj is 4-1 == 3
            //overlap.size() == tHid.size() &&
            if(areaIsSolvable(t)
            && adjHid.size()
            && requiredCountToSolve(*adj) -requiredCountToSolve(t) == adjHid.size())
            {
                for (Tile* tNonOv : adjHid)
                    tNonOv->setFlag(true); 
            }
        }
    }
}

void AI::useEverything()
{
    for(int i=0; i < 15; ++i)
    {
        flagObvious();
        traitor();
        iKnowWhereTheOthers();
    }
}

Tile& AI::tileAt(const Vei2& indexPos) const
{
    return field.tiles[indexPos.x + indexPos.y *field.tilesInW];
}

std::vector<Tile*> AI::getHidOverlapTiles(const Vei2& cenInd1, const Vei2& cenInd2) const
{
    std::vector<Tile*> overlap;
    overlap.reserve(4); // 2 areas can have 4 tiles overlapping max

    auto area1 = getHiddenTiles(cenInd1, false);
    auto area2 = getHiddenTiles(cenInd2, false);

    for(Tile* t1 : area1)
    {
        for (Tile* t2 : area2)
            if(t1->index == t2->index)
                overlap.push_back(t1);
    }
    return overlap;
}

// t is the guy who has nonOverlap
std::vector<Tile*> AI::getNonOverlapTiles(const Tile* t, const Tile* adjT) const
{
    std::vector<Tile*> nonOverlap;
    auto overlap = getHidOverlapTiles(t->index, adjT->index);

    size_t requiredFlagCount = adjT->numOfAdjMemes -getAdjFlagCount(adjT->index);
    if(overlap.size() >= requiredFlagCount)
    {
        // exclude overlaps for t
        nonOverlap = getHiddenTiles(t->index, false);
        excludeTiles(nonOverlap, overlap);
    }

    return nonOverlap;
}

void AI::excludeTiles(std::vector<Tile*>& mainVec,
                                     std::vector<Tile*>& tilesToExclude) const
{
    for(int i=mainVec.size() -1; i >= 0; --i)
    {
        for(const Tile* texc : tilesToExclude)
            if(mainVec.at(i)->index == texc->index)
            {
                mainVec.erase(mainVec.begin() +i);
                break;
            }
    }
}


bool AI::solvableWithoutTiles(const Tile* t, std::vector<Tile*>& tilesToExclude) const
{
    auto remainingTiles = getHiddenTiles(t->index, false);
    excludeTiles(remainingTiles, tilesToExclude);
    
    return requiredCountToSolve(*t) == 1 
        && (int)remainingTiles.size() >= requiredCountToSolve(*t);
}

bool AI::impossibleWithoutTiles(const Tile* t, std::vector<Tile*>& overlap) const
{
    auto adjTiles = getHiddenTiles(t->index, true);
    excludeTiles(adjTiles, overlap);
    return (int)adjTiles.size() < t->numOfAdjMemes;
}

bool AI::areaIsSolvable(const Tile& t) const
{
    auto hidTiles = getHiddenTiles(t.index, false);
    return int(hidTiles.size()) >= requiredCountToSolve(t);
}

int AI::requiredCountToSolve(const Tile& t) const
{
    return t.numOfAdjMemes - getAdjFlagCount(t.index);
}

int AI::getAdjFlagCount(const Vei2& centerTile) const
{
    auto adjT = getAdjTiles(centerTile);
    int count = 0;
    for(const Tile* t : adjT)
        if(t->getDrawSt() == DrawSt::Flag)
            ++count;

    return count;
}

void AI::parseKB(const Keyboard::Event& event)
{
	switch (event.GetCode())
	{
		case '1':   randClick();             break;
		case '2':   flagObvious();           break;
		case '3':   afterFlag();             break;
        case '4':   traitor();               break;
        case '5':   iKnowWhereTheOthers();   break;
        case 'W':   useEverything();         break;
        case 'Q':   randClick(); useEverything(); break;
	}
}

std::vector<Tile*> AI::getAdjTiles(const Vei2& centerTile) const
{
    std::vector<Tile*> vec;
    vec.reserve(8);

    Vei2 adjInd{ centerTile.x -1, centerTile.y -1 };
    for (int i = 0; i < 9; ++i, ++adjInd.x)
    {
        if (i && i %3 == 0)
        {
            ++adjInd.y;
            adjInd.x = centerTile.x-1;
        }

        if (field.tileIsValid(adjInd) && centerTile != adjInd)
            vec.push_back(&tileAt(adjInd));
    }

    return vec;
}

std::vector<Tile*> AI::getHiddenTiles(const Vei2& centerTile, bool includeFlagged) const
{
    std::vector<Tile*> adjTiles = getAdjTiles(centerTile);

    std::vector<Tile*> hidTiles;
    hidTiles.reserve(8);

    for (Tile* t : adjTiles)
    {
        bool include = includeFlagged ? true
                                      : !(t->getDrawSt() == DrawSt::Flag);
        if (!t->isRevealed() && include)
            hidTiles.push_back(t);
    }

    return hidTiles;
}

