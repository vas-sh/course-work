#include "cell.cc"
#include "sector.cc"
#include "shape.cc"
#include "combination.cc"
#include <vector>
#include <map>
#include <array>
#include <atomic>
#include <iostream>
#include <sstream>
#include <thread>
#include <mutex>
#include <condition_variable>
using namespace std;


class Board {
    public:
        vector<Cell> Cells;
	    vector<Sector> Sectors;
        map<array<int, 2>, int> Indexes;
		vector<vector<int>> WhiteIndex;

        bool isCorrect();
        void run();
        void setNumbers();
        Board copy() const;
        bool fill(atomic<bool>& canselFlag, const vector<Cell>& filledCells, bool checkSectors);      
        bool add(int i);
        bool canAdd(const Cell& cell);
        bool checkWhiteLines();
		void checkNexts(vector<Cell>& white, const vector<Cell>& nexts, const vector<vector<int>>& m);
		vector<Cell> getNexts(const vector<vector<int>>& m, vector<Cell> & white, const Cell& start);
		bool fullSectors() const;
        int checkHorizontalWhite() const;
        vector<vector<Cell>> getRows(int rowIndx) const;
        int getSectionIndx(int row, int col) const;
        int checkVerticalWhite() const;
		vector<Cell> getCol(int colIndx) const;
        vector<Cell> getPossibleSectors();
        vector<Cell> getPossibleCells(int row, int col);
        string display() const;
        bool valid();
        void cleanFilled();
        int cellIndex(int i, int j) const;
        Cell* findCell(int i, int j);
        const Cell* findCell(int i, int j) const;
        int inSector(const Sector& sector) const;
        bool fullSector(const Sector& sector) const;
        bool canAddToSector(const Sector& sector, const Cell& cell) const;
        bool nextToFilled(const Cell& cell) const;
        vector<Cell> white() const;

		Board(const Board& other) :
        Cells(other.Cells),
        Sectors(other.Sectors.size()),
        Indexes(other.Indexes),
        WhiteIndex(other.WhiteIndex)
    {
  
        for (size_t i = 0; i < other.Sectors.size(); ++i) {
            Sectors[i].Cells = other.Sectors[i].Cells;
            Sectors[i].Number = other.Sectors[i].Number;
        }
    }
    Board(vector<Cell> cells, vector<Sector> sectors, map<array<int, 2> ,int> indexes = {}, vector<vector<int>> whiteIndx = {})
    : Cells(std::move(cells)), Sectors(std::move(sectors)),
      Indexes(std::move(indexes)), WhiteIndex(std::move(whiteIndx)) {}

    Board& operator=(const Board& other) {
        if (this == &other) {
            return *this;
        }
        Cells = other.Cells;
        Sectors.resize(other.Sectors.size());
         for (size_t i = 0; i < other.Sectors.size(); ++i) {
            Sectors[i].Cells = other.Sectors[i].Cells;
            Sectors[i].Number = other.Sectors[i].Number;
        }
        Indexes = other.Indexes;
        WhiteIndex = other.WhiteIndex;
        return *this;
    }
};

void Board::run() {
    int workers = 0;
    atomic<bool> solutionFound{false};
    mutex resultMutex;
    condition_variable resultCV;
    vector<thread> workerThreads;
    mutex workerMutex;

    auto resultHandler = [&](const vector<Cell>& comb) {
        if (solutionFound.load()) {
            return;
        }
        workers++;
        Board board_copy = copy();
        thread worker([this, board_copy, comb, &solutionFound, &resultMutex, &resultCV, workers ]() mutable {
            vector<Cell> CombVec = {comb};
			try {
                bool validSolution = board_copy.fill(solutionFound, CombVec, true);
                if (solutionFound.load()) {
                    return;
                }
                if (validSolution) {
                    if (board_copy.valid()) {
                        lock_guard<mutex> lock(resultMutex);
                        if (!solutionFound.load()) {
							cout << "Result:";
                            cout << board_copy.display();
							cout << "Solution found by worker: " << workers << endl;
                            solutionFound.store(true); 
                            resultCV.notify_all();
                        }
                    }
                }
			} catch (const exception& e) {
				cerr << "Exception in fill worker thread: " << e.what() << endl;
			} catch (...) {
				cerr << "Unknown exception in fill worker thread." << endl;
			}
        });
        worker.detach();
    };
    thread generator = Combs(solutionFound, Sectors, Cells, resultHandler);
    {
        unique_lock<mutex> lock(resultMutex);
        resultCV.wait(lock, [&]{ return solutionFound.load(); });
    }
    if (generator.joinable()) {
        generator.join();
    }
	cout << "Total combinations processed: " << workers << endl;
}

bool Board::isCorrect() {
    unordered_map<string, size_t> board_cell_index;
    for (size_t i = 0; i < Cells.size(); i++) {
        board_cell_index[Cells[i].Coords()] = i;
    }
    vector<bool> cell_filled_check(Cells.size(), false);
    vector<vector<bool>> sector_cell_check(Sectors.size());

    for (size_t i = 0; i < Sectors.size(); ++i) {
        sector_cell_check[i].resize(Sectors[i].Cells.size(), false);
        for (size_t j = 0; j < Sectors[i].Cells.size(); j++) {
            string coord = Sectors[i].Cells[j].Coords();
            if (!board_cell_index.count(coord)) {
                cout << "invalid, unaccounted sector cell, Sector" << i << "Cell " << j << endl;
                return false;
            }
            size_t boardIndex = board_cell_index[coord];
            if (cell_filled_check[boardIndex]) {
                cout << "invalid, overlap or duplicate, Board Cell " << coord << " in multiple sectors" << endl;
                return false;
            }
            cell_filled_check[boardIndex] = true;
            sector_cell_check[i][j] = true;
        }
    }
    for (size_t i = 0; i < Cells.size(); ++i) {
        if (!cell_filled_check[i]) {
            cout << "invalid, unaccounted board cell: " << Cells[i].Coords() <<endl;
            return false;
        }
    }
    for (size_t i =0; i < sector_cell_check.size(); ++i) {
        for (size_t j = 0; j < sector_cell_check[i].size(); ++j) {
            if (!sector_cell_check[i][j]) {
                cout << "invalid, unaccounted sector cell: Sector " << i << " Cell " << Sectors[i].Cells[j].Coords() <<endl;
                return false;
            }
        }
    }
    return true;
}

string Board::display() const {
    Shape shape = getShape(Cells);
    stringstream ss;
    for (int i = shape.MinI; i <= shape.MaxI; ++i) {
        ss << "\n";
        for (int j = shape.MinJ; j <= shape.MaxJ; ++j) {
            const Cell* cell = findCell(i, j);
            if (cell != nullptr) {
                ss << cell->String();
            } else {
                ss << "  ";
            }
        }
    }
    ss << "\n";
    return ss.str();
}

void Board::setNumbers() {
    for (size_t i = 0; i < Sectors.size(); ++i) {
        if (Sectors[i].Number == nullptr) {
            continue;
        }
        if (Sectors[i].Cells.empty()) {
            continue;
        }

        for (size_t j = 0; j < Cells.size(); ++j) {
            if (Cells[j].Equal(Sectors[i].Cells[0])) {
                Cells[j].NumberDisplay = Sectors[i].Number;
                break; 
            }
        }
    }

    Indexes.clear();
    for (size_t i = 0; i < Cells.size(); ++i) {
        Indexes[{Cells[i].i, Cells[i].j}] = static_cast<int>(i);
    }

    Shape shape = getShape(Cells);
        int num_rows = (shape.MaxI >= shape.MinI) ? (shape.MaxI + 1) : 0;
        WhiteIndex.assign(num_rows, vector<int>()); 
        if (num_rows > 0) {
            int num_cols = (shape.MaxJ >= shape.MinJ) ? (shape.MaxJ + 1) : 0;
            if (num_cols > 0) {
                for (int i = 0; i < num_rows; ++i) {
                    WhiteIndex[i].assign(num_cols, -1);
                }
            }
        }
}

Board Board::copy() const {
    return Board(*this);
}

bool Board::add(int cell_index) {
    if (cell_index < 0 || static_cast<size_t>(cell_index) >= Cells.size()) {
		return false;
	}
	if (!canAdd(Cells[cell_index])) {
		return false;
	}
	Cells[cell_index].filled = true;
	return true;
}

bool Board::canAdd(const Cell& cell) { 
	for (size_t i = 0; i < Sectors.size(); ++i) {
		if (!canAddToSector(Sectors[i], cell)) {
			return false;
		}
	}
	if (nextToFilled(cell)) {
		return false;
	}

	Cell* c_ptr = findCell(cell.i, cell.j);
	if (c_ptr != nullptr && !c_ptr->filled) {
		c_ptr->filled = true; 
		bool ok = checkWhiteLines();
		c_ptr->filled = false;
		return ok;
	}
	return true;
}

bool Board::checkWhiteLines() {
	vector<Cell> white_cells;
        white_cells.reserve(Cells.size());
        int indx = -1;
        for (size_t i = 0; i < Cells.size(); ++i) {
            if (!Cells[i].filled) {
                white_cells.push_back(Cells[i]);
                indx++;
                 if (Cells[i].i >= 0 && Cells[i].i < static_cast<int>(WhiteIndex.size()) &&
                    Cells[i].j >= 0 && Cells[i].j < static_cast<int>(WhiteIndex[Cells[i].i].size())) {
                    WhiteIndex[Cells[i].i][Cells[i].j] = indx;
                 }
            } else {
                 if (Cells[i].i >= 0 && Cells[i].i < static_cast<int>(WhiteIndex.size()) &&
                    Cells[i].j >= 0 && Cells[i].j < static_cast<int>(WhiteIndex[Cells[i].i].size())) {
                    WhiteIndex[Cells[i].i][Cells[i].j] = -1;
                 }
            }
        }

        if (white_cells.empty()) {
            return true; 
        }

        white_cells[0].filled = true;
        extern vector<Cell> getNexts(const vector<vector<int>>& m, vector<Cell>& white, const Cell& start);
        extern void checkNexts(vector<Cell>& white, const vector<Cell>& nexts, const vector<vector<int>>& m);

        this->checkNexts(white_cells, vector<Cell>{white_cells[0]}, WhiteIndex);

        for (size_t i = 0; i < white_cells.size(); ++i) {
            if (!white_cells[i].filled) {
                return false;
            }
        }
        return true;
    }

void Board::checkNexts(vector<Cell>& white, const vector<Cell>& nexts, const vector<vector<int>>& m) {
		if (nexts.empty()) { 
        return;
    }
	vector<Cell> children;
    children.reserve(nexts.size() * 4);
	for (size_t i = 0; i < nexts.size(); ++i) {
		vector<Cell> grand_children = getNexts(m, white, nexts[i]);
        children.insert(children.end(), grand_children.begin(), grand_children.end());
	}

	checkNexts(white, children, m);
}

vector<Cell> Board::getNexts(const vector<vector<int>>& m, vector<Cell> & white, const Cell& start) {
	vector<Cell> nexts;
	nexts.reserve(4); 
	const std::array<array<int, 2>, 4> cords = {{
		{start.i - 1, start.j},
		{start.i + 1, start.j},
		{start.i, start.j - 1},
		{start.i, start.j + 1},
	}};

	for (size_t i = 0; i < cords.size(); ++i) {
        int r = cords[i][0];
        int c = cords[i][1];

		if (r < 0 || r >= static_cast<int>(m.size())) {
			continue;
		}
		const vector<int>& arr = m[r];
		if (c < 0 || c >= static_cast<int>(arr.size())) {
			continue;
		}

		int idx = arr[c];
		if (idx == -1) { 
			continue;
		}

        if (idx < 0 || idx >= static_cast<int>(white.size())) {
             continue;
        }


		if (!white[idx].filled) {
			white[idx].filled = true; 
			nexts.push_back(white[idx]); 
		}
	}
	return nexts;
}

bool Board::fullSectors() const {
	for (size_t i = 0; i < Sectors.size(); ++i) {
        if (!fullSector(Sectors[i])) {
            return false;
        }
    }
    return true;
}


vector<vector<Cell>> Board::getRows(int rowIndx) const {
	vector<Cell> row;
	for (const auto& cell : Cells) {
		if (cell.i == rowIndx) {
			row.push_back(cell);
		}
	}
	sort(row.begin(), row.end(), [](const Cell& a, const Cell& b) {
		return a.j < b.j;
	});

	vector<vector<Cell>> rows;
	for (size_t i = 0; i < row.size(); ++i) {
		if (i == 0 || row[i].j - 1 != row[i - 1].j) {
			rows.push_back({row[i]}); 
		} else {
            rows.back().push_back(row[i]);
        }
	}
	return rows;
}

int Board::getSectionIndx(int row, int col) const {
	for (size_t i = 0; i < Sectors.size(); ++i) {
		for (const auto& cell : Sectors[i].Cells) {
			if (cell.i == row && cell.j == col) {
				return static_cast<int>(i);
			}
		}
	}
	return -1; 
}
vector<Cell> Board::getCol(int colIndx) const {
	vector<Cell> col;
	for (const auto& cell : Cells) {
		if (cell.j == colIndx) {
			col.push_back(cell);
		}
	}
	sort(col.begin(), col.end(), [](const Cell& a, const Cell& b) {
		return a.i < b.i;
	});
	return col;
}

int Board::checkHorizontalWhite() const {
    vector<Cell> whiteCells = white();
    Shape shape = getShape(whiteCells);

    for (int i = shape.MaxI; i <= shape.MaxI; i++) {
        for (const auto& _ [[maybe_unused]] : getRows(i)) {
            vector<Cell> currentRow;
            for (const auto& cell : Cells) {
                if (cell.i == i) {
                    currentRow.push_back(cell);
                }
            }
        
            sort(currentRow.begin(), currentRow.end(), [](const Cell& a, const Cell& b) {
                return a.j < b.j;
            });
            int whiteCross = 0;
            for (const auto& cell : currentRow) {
                if (cell.filled) {
                    whiteCross = 0;
                    continue;
                }
                const Cell* preCell = findCell(cell.i, cell.j -1);
                if (preCell != nullptr && preCell->filled) {
                    whiteCross = 0;
                    continue;
                }

                int sectionIdx = getSectionIndx(cell.i, cell.j);
                int preSectionIdx = (preCell != nullptr) ? getSectionIndx(preCell->i, preCell->j) : -1;
                if (preCell != nullptr && preSectionIdx > -1 && sectionIdx != preSectionIdx) {
                    whiteCross++;
                    if (whiteCross > 1) {
                        return i;
                    }
                } 
            }
        }
    }
    return -1;
}

int Board::checkVerticalWhite() const {
	vector<Cell> whiteCells = white();
	Shape shape = getShape(whiteCells);

	for (int j = shape.MinJ; j <= shape.MaxJ; ++j) { 
		vector<Cell> cols = getCol(j);
		int whiteCross = 0;

		for (const auto& cell : cols) {
			if (cell.filled) {
				whiteCross = 0;
				continue;
			}

			const Cell* preCell = findCell(cell.i - 1, cell.j);
			if (preCell != nullptr && preCell->filled) {
                whiteCross = 0;
				continue;
			}

			int sectionIdx = getSectionIndx(cell.i, cell.j);
			int preSectionIdx = (preCell != nullptr) ? getSectionIndx(preCell->i, preCell->j) : -1;

			if (preCell != nullptr && preSectionIdx > -1 && sectionIdx != preSectionIdx) {
				whiteCross++;
				if (whiteCross > 1) {
					return j;
				}
			}
		}
	}
	return -1;
}

vector<Cell> Board::getPossibleSectors() {
	vector<Cell> res;
	for (size_t i = 0; i < Sectors.size(); ++i) {
		if (fullSector(Sectors[i])) {
			continue;
		}
		for (size_t j = 0; j < Sectors[i].Cells.size(); ++j) {
            const Cell& sectorCell = Sectors[i].Cells[j];
            if (!canAddToSector(Sectors[i], sectorCell)) {
                continue;
            }
            Cell* c = findCell(sectorCell.i, sectorCell.j);
            if (c != nullptr && !c->filled) {
                res.push_back(*c);
            }
        }
        return res;
    }
    return res;
}

vector<Cell> Board::getPossibleCells(int row, int col) {
    vector<Cell> cells;
    cells.reserve(Cells.size());
    for (size_t i = 0; i < Cells.size(); ++i) {
        if (Cells[i].filled) {
            continue;
        }
        if (row > -1 && Cells[i].i != row) {
            continue;
        }
        if (col > -1 && Cells[i].j != col) {
            continue;
        }
        cells.push_back(Cells[i]);
    }
    return cells;
}

bool Board::fill(atomic<bool>& cancelFlag, const vector<Cell>& filledCells, bool checkSectors) {
    if (cancelFlag.load()) return false;
    cleanFilled();

    for (const auto& cellToFill : filledCells) {
        int idx = cellIndex(cellToFill.i, cellToFill.j);
        if (idx < 0) {
            cerr << "error: cell to fill not found on board"<<endl;
            return false;
        }

        if (!add(idx)) {
            return false;
        }
    }
    vector<Cell> posibles = getPossibleSectors();
    if (posibles.empty()) {
        if (checkSectors) {
            checkSectors = false;
            if (!fullSectors()) {
                return false;
            }
        }
        int problRow = checkHorizontalWhite();
        if (problRow > -1) {
            posibles = getPossibleCells(problRow, -1);
        } else {
            int problCol = checkVerticalWhite();
            if (problCol > -1) {
                posibles = getPossibleCells(-1, problCol);
            } else {
                return true;
            }
        }
    
        if (posibles.empty() && (problRow > -1 || checkVerticalWhite() > -1)) {
            return false;
        }
    }
    vector<Cell> children = filledCells;
    children.emplace_back();

    bool solutionFoundInBranch = false;
    for (size_t i = 0; i < posibles.size(); ++i) {
        if (cancelFlag.load()) {
            return false;
        }
        children.back() = posibles[i];
        if (fill(cancelFlag, children, checkSectors)) {
            solutionFoundInBranch = true;
            break;
        }
    }
   return solutionFoundInBranch;
}

bool Board::valid() {
	if (!fullSectors()) {
		return false;
	}
	if (!checkWhiteLines()) {
	   return false;
    }
	if (checkHorizontalWhite() > -1) {
		return false;
	}
	if (checkVerticalWhite() > -1) {
		return false;
	}
	return true;
}
 
void Board::cleanFilled() {
    for (size_t i = 0; i < Cells.size(); ++i) {
        Cells[i].filled = false;
    }
}

int Board::cellIndex(int i, int j) const {
    array<int, 2> key = {i, j};
    auto it = Indexes.find(key);
    if (it != Indexes.end()) {
        return it->second; 
    }
    return -1;
}

Cell* Board::findCell(int i, int j) {
    int idx = cellIndex(i, j);
    if (idx < 0 || idx >= static_cast<int>(Cells.size())) {
        return nullptr;
    }
    return &Cells[idx];
}

const Cell* Board::findCell(int i, int j) const { 
    int idx = cellIndex(i, j);
    if (idx < 0 || idx >= static_cast<int>(Cells.size())) {
        return nullptr;
    }
    return &Cells[idx];
}

int Board::inSector(const Sector& sector) const {
    if (sector.Number == nullptr) {
        return 0;
    }
    int cnt = 0;
    for (size_t i = 0; i < Cells.size(); ++i) {
        if (!Cells[i].filled) {
            continue;
        }
        if (sector.Contains(Cells[i])) {
            cnt++;
        }
    }
    return cnt;
}

bool Board::fullSector(const Sector& sector) const {
    if (sector.Number == nullptr) {
        return true; 
    }
    return inSector(sector) == *sector.Number;
}

bool Board::canAddToSector(const Sector& sector, const Cell& cell) const {
    if (!sector.Contains(cell)) {
        return true;
    }
    if (sector.Number == nullptr) {
         return true; 
    }
    return inSector(sector) < *sector.Number;
}

bool Board::nextToFilled(const Cell& cell) const {
    for (size_t i = 0; i < Cells.size(); ++i) {
        if (!Cells[i].filled) {
            continue;
        }
        if (Cells[i].Equal(cell)) {
            continue; 
        }
        if (Cells[i].NextTo(cell)) {
            return true;
        }
    }
    return false;
}

vector<Cell> Board::white() const {
    vector<Cell> white_cells;
    white_cells.reserve(Cells.size());
    for (size_t i = 0; i < Cells.size(); ++i) {
        if (!Cells[i].filled) {
            white_cells.push_back(Cells[i]);
        }
    }
    return white_cells;
}