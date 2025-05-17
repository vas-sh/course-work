#pragma once
#include "cell.h"
#include "sector.h"
#include <vector>
#include <map>
#include <array>
#include <atomic>
#include <thread>
#include <functional>

class Board {
    public:
        std::vector<Cell> Cells;
        std::vector<Sector> Sectors;
        std::map<std::array<int, 2>, int> Indexes;
		std::vector<std::vector<int>> WhiteIndex;

        bool isCorrect();
        void run();
        void setNumbers();
        Board copy() const;
        bool fill(std::atomic<bool>& canselFlag, const std::vector<Cell>& filledCells, bool checkSectors);      
        bool add(int i);
        bool canAdd(const Cell& cell);
        bool checkWhiteLines();
		void checkNexts(std::vector<Cell>& white, const std::vector<Cell>& nexts, const std::vector<std::vector<int>>& m);
		std::vector<Cell> getNexts(const std::vector<std::vector<int>>& m, std::vector<Cell> & white, const Cell& start);
		bool fullSectors() const;
        int checkHorizontalWhite() const;
        std::vector<std::vector<Cell>> getRows(int rowIndx) const;
        int getSectionIndx(int row, int col) const;
        int checkVerticalWhite() const;
		std::vector<Cell> getCol(int colIndx) const;
        std::vector<Cell> getPossibleSectors();
        std::vector<Cell> getPossibleCells(int row, int col);
        std::string display() const;
        bool valid();
        void cleanFilled();
        int cellIndex(int i, int j) const;
        Cell* findCell(int i, int j);
        const Cell* findCell(int i, int j) const;
        int inSector(const Sector& sector) const;
        bool fullSector(const Sector& sector) const;
        bool canAddToSector(const Sector& sector, const Cell& cell) const;
        bool nextToFilled(const Cell& cell) const;
        std::vector<Cell> white() const;
        Board& operator=(const Board& other);

        // Copy constructor (need for board.copy())
		Board(const Board& other) :
        Cells(other.Cells),
        Sectors(other.Sectors.size()),
        Indexes(other.Indexes),
        WhiteIndex(other.WhiteIndex)
    {
        // Deep copy Sectors (Cells vector and shared_ptr Number)
        for (size_t i = 0; i < other.Sectors.size(); ++i) {
            Sectors[i].Cells = other.Sectors[i].Cells;
            Sectors[i].Number = other.Sectors[i].Number;
        }
    }
    Board(std::vector<Cell> cells, std::vector<Sector> sectors, std::map<std::array<int, 2> ,int> indexes = {}, std::vector<std::vector<int>> whiteIndx = {})
    : Cells(std::move(cells)), Sectors(std::move(sectors)),
      Indexes(std::move(indexes)), WhiteIndex(std::move(whiteIndx)) {}
};
