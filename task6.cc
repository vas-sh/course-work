#include <vector>
#include <memory> 
#include <map>
#include <iostream>
#include <unordered_map>
#include <functional>
#include <limits>
#include <sstream>
#include <chrono>
#include <atomic> 
#include <mutex>
#include <condition_variable>
#include <thread>

using namespace std;

struct Cell {
	int i = 0, j = 0;  
    shared_ptr<int> NumberDisplay = nullptr;
    bool filled = false;
    
    string Coords() const {
        char buf[32]; 
        snprintf(buf, sizeof(buf), "%d %d", i, j);
        return string(buf);
    }

    bool NextTo(const Cell& cell) const {
		if (i == cell.i) {
			return j == cell.j - 1 || j == cell.j + 1;
		}
		if (j == cell.j) {
			return i == cell.i - 1 || i == cell.i + 1;
		}
		return false;
	}

    bool Equal(const Cell& cell) const {
		return i == cell.i && j == cell.j;
	}

    string String() const {
		if (filled) {
			return "|x";
		}
		if (NumberDisplay != nullptr) {
            char buffer[16];
            snprintf(buffer, sizeof(buffer), "|%d", *NumberDisplay);
            return string(buffer);
		}
		return "|_";
	}
    
};

struct Sector {
	vector<Cell> Cells;
	shared_ptr<int> Number = nullptr;

	bool Contains(const Cell& cell) const {
		if (Number == nullptr) {
			return false;
		}
		for (size_t i = 0; i < Cells.size(); ++i) {
			if (Cells[i].Equal(cell)) {
				return true;
			}
		}
		return false;
	}

    vector<vector<Cell>> Combs() const {
        vector<vector<Cell>> res;
        if (Number == nullptr || *Number == 0) {
            return res;
        }

        std::function<void(int, vector<Cell>)> backtrack;
        backtrack = [&](int start, vector<Cell> path) {
            if (path.size() == static_cast<size_t>(*Number)) {
                vector<Cell> comb = path; 
                extern bool validComb(const vector<Cell>& comb);
                if (validComb(comb)) {
                    res.push_back(comb);
                }
                return;
            }
            for (size_t i = start; i < Cells.size(); ++i) {
                vector<Cell> next_path = path;
                next_path.push_back(Cells[i]);
                backtrack(i + 1, next_path);
            }
        };

        backtrack(0, vector<Cell>{});
        return res;
    }
};

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

        void cleanFilled() {
            for (size_t i = 0; i < Cells.size(); ++i) {
                Cells[i].filled = false;
            }
        }

        int cellIndex(int i, int j) const {
            array<int, 2> key = {i, j};
            auto it = Indexes.find(key);
            if (it != Indexes.end()) {
                return it->second; 
            }
            return -1;
        }

        Cell* findCell(int i, int j) {
            int idx = cellIndex(i, j);
            if (idx < 0 || idx >= static_cast<int>(Cells.size())) {
                return nullptr;
            }
            return &Cells[idx];
        }

        const Cell* findCell(int i, int j) const { 
            int idx = cellIndex(i, j);
            if (idx < 0 || idx >= static_cast<int>(Cells.size())) {
                return nullptr;
            }
            return &Cells[idx];
        }
	
		int inSector(const Sector& sector) const {
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
	
		bool fullSector(const Sector& sector) const {
			if (sector.Number == nullptr) {
				return true; 
			}
			return inSector(sector) == *sector.Number;
		}
	
		bool canAddToSector(const Sector& sector, const Cell& cell) const {
			if (!sector.Contains(cell)) {
				return true;
			}
			if (sector.Number == nullptr) {
				 return true; 
			}
			return inSector(sector) < *sector.Number;
		}

		bool nextToFilled(const Cell& cell) const {
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

		vector<Cell> white() const {
			vector<Cell> white_cells;
			white_cells.reserve(Cells.size());
			for (size_t i = 0; i < Cells.size(); ++i) {
				if (!Cells[i].filled) {
					white_cells.push_back(Cells[i]);
				}
			}
			return white_cells;
		}
    };

shared_ptr<int> ptrTo(int i) {
    return make_shared<int>(i);
}

bool validComb(const vector<Cell>& comb) {
    if (comb.size() <= 1) {
        return true;
    }
    for (size_t i = 0; i < comb.size(); ++i) {
        for (size_t j = i + 1; j < comb.size(); ++j) {
            if (comb[i].NextTo(comb[j]) || comb[i].Equal(comb[j])) {
                return false;
            }
        }
    }
    return true;
}
 
struct Shape {
	int MinI = 0, MinJ = 0;
	int MaxI = 0, MaxJ = 0;
};

Shape getShape(const vector<Cell>& cells) {
	Shape res;
    res.MinI = numeric_limits<int>::max();
    res.MinJ = numeric_limits<int>::max();
    res.MaxI = numeric_limits<int>::min();
    res.MaxJ = numeric_limits<int>::min();

    if (cells.empty()) {
        res.MinI = 0; res.MinJ = 0; res.MaxI = 0; res.MaxJ = 0;
        return res;
    }

	for (size_t i = 0; i < cells.size(); ++i) {
		res.MinI = min(cells[i].i, res.MinI);
		res.MinJ = min(cells[i].j, res.MinJ);
		res.MaxI = max(cells[i].i, res.MaxI);
		res.MaxJ = max(cells[i].j, res.MaxJ);
	}
	return res;
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

static thread Combs(atomic<bool>& cancel, const vector<Sector>& sectors, const vector<Cell> cells, function<void(const vector<Cell>)> resultHandler) {
    thread generator_thread([&cancel, sectors, resultHandler]() {
        try {
            vector<vector<vector<Cell>>> groups;
            for (size_t i = 0; i < sectors.size(); ++i) {
                 if (cancel.load()) return; 
                if (sectors[i].Number != nullptr && *sectors[i].Number > 0) {
                    groups.push_back(sectors[i].Combs());
                }
            }

            sort(groups.begin(), groups.end(), [](const vector<vector<Cell>>& a, const vector<vector<Cell>>& b) {
                return a.size() < b.size();
            });

            long long m = 0;
            size_t limit_idx = groups.size();
            for (size_t i = 0; i < groups.size(); ++i) {
                 if (cancel.load()) return;
                if (groups[i].empty() && (sectors[i].Number && *sectors[i].Number > 0)) {
                    
                     return; 
                }
                if (m == 0) {
                    if (!groups[i].empty()) {
                       m = groups[i].size();
                    }
                    continue;
                }
                if (groups[i].empty()) continue; 

                long long current_size = groups[i].size();
                if (current_size > 0 && m > 100'000'000LL / current_size) {
                    limit_idx = i;
                    break;
                }
                m *= current_size;
                if (m > 100'000'000LL) {
                    limit_idx = i;
                    break;
                }
            }
            if (limit_idx < groups.size()) {
                groups.resize(limit_idx);
            }


            function<void(int, vector<Cell>)> backtrack;
            backtrack = [&](int index, vector<Cell>path) {
                if (cancel.load()) return; 

                if (index == static_cast<int>(groups.size())) {
                    if (validComb(path)) {
                      
                        resultHandler(path);
                        if (cancel.load()) return;
                    }
                    return;
                }

                if (groups[index].empty()) {
                     backtrack(index + 1, path);
                     return;
                }


                for (const auto& option : groups[index]) {
                    if (cancel.load()) return; 

                    vector<Cell> next_path = path;
                    next_path.insert(next_path.end(), option.begin(), option.end());

                    if (validComb(next_path)) {
                        backtrack(index + 1, next_path);
                        if (cancel.load()) return;
                    }
                }
            };

            backtrack(0, vector<Cell>{});

        } catch (const exception& e) {
            cerr << "Exception in Combs generator thread: " << e.what() << endl;
        } catch (...) {
            cerr << "Unknown exception in Combs generator thread." << endl;
        }
    }); 

    return generator_thread;
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

Board testCase1 = {
    {
        {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7}, {0, 8}, {0, 9},
        {1, 0}, {1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}, {1, 7}, {1, 8}, {1, 9},
        {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {2, 5}, {2, 6}, {2, 7}, {2, 8}, {2, 9},
        {3, 0}, {3, 1}, {3, 2}, {3, 3}, {3, 4}, {3, 5}, {3, 6}, {3, 7}, {3, 8}, {3, 9},
        {4, 0}, {4, 1}, {4, 2}, {4, 3}, {4, 4}, {4, 5}, {4, 6}, {4, 7}, {4, 8}, {4, 9},
        {5, 0}, {5, 1}, {5, 2}, {5, 3}, {5, 4}, {5, 5}, {5, 6}, {5, 7}, {5, 8}, {5, 9},
        {6, 0}, {6, 1}, {6, 2}, {6, 3}, {6, 4}, {6, 5}, {6, 6}, {6, 7}, {6, 8}, {6, 9},
        {7, 0}, {7, 1}, {7, 2}, {7, 3}, {7, 4}, {7, 5}, {7, 6}, {7, 7}, {7, 8}, {7, 9},
        {8, 0}, {8, 1}, {8, 2}, {8, 3}, {8, 4}, {8, 5}, {8, 6}, {8, 7}, {8, 8}, {8, 9},
        {9, 0}, {9, 1}, {9, 2}, {9, 3}, {9, 4}, {9, 5}, {9, 6}, {9, 7}, {9, 8}, {9, 9},
    },
    {
        {
            { 
                {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, 
            }, 
            ptrTo(1)
        },
        {
            { 
                {0, 7}, {0, 8}, {0, 9}, 
                {1, 7}, {1, 8}, {1, 9}, 
                {2, 7}, {2, 8}, {2, 9},
            }, 
            ptrTo(4)},
        {
            { 
                {1, 0}, 
                {2, 0}, 
                {3, 0},
            }, ptrTo(2)
        },
        {
            {
                {1, 1}, {1, 2}, {1, 3}, 
                {2, 1}, {2, 2}, {2, 3}, 
                {3, 1}, {3, 2}, {3, 3}, 
            }, 
            nullptr
        },
        {
            { 
                {1, 4}, {2, 4}, {3, 4}, 
            }, 
            nullptr
        },
        {
            { 
                {1, 5}, {1, 6}, {2, 5}, {2, 6}, 
            }, 
            nullptr
        },
        {
            { 
                {3, 5}, {3, 6}, {3, 7}, {3, 8}, {3, 9},
            }, 
            nullptr
        },
        {
            { 
                {4, 0}, {4, 1},
                {5, 0}, {5, 1}, 
                {6, 0}, {6, 1},
            }, nullptr
        },
        {
            { 
                {4, 2}, {4, 3}, 
                {5, 2}, {5, 3}, 
                {6, 2}, {6, 3},
            }, 
            ptrTo(3)
        },
        {
            { 
                {4, 4}, {4, 5}, {4, 6}, {4, 7}, {4, 8}, {4, 9}, 
            }, 
            nullptr},
        {
            {
                {5, 4}, {5, 5}, 
                {6, 4}, {6, 5}, 
                {7, 4}, {7, 5}, 
                {8, 4}, {8, 5},
            }, 
            nullptr
        },
        {
            {
                {5, 6},
                {6, 6},
                {7, 6},
                {8, 6}, 
                {9, 6},
            }, 
                nullptr
        },
        {
            {
                 {5, 7}, {5, 8}, {5, 9}, {6, 7}, {6, 8}, {6, 9},
            }, 
            nullptr
        },
        {
            {
                {7, 7}, {7, 8}, {7, 9},
                {8, 7}, {8, 8}, {8, 9},
                {9, 7}, {9, 8}, {9, 9}, 
            }, 
            ptrTo(4)
            }, 
        {
            { 
                {7, 0}, {7, 1}, {7, 2}, 
                {8, 0}, {8, 1}, {8, 2},
            },
            ptrTo(3)
        }, 
        {
            {
                {7, 3}, 
                {8, 3},
            }, 
            nullptr
        },
        {
            {
                 {9, 0}, {9, 1}, {9, 2}, {9, 3}, {9, 4}, {9, 5},
                }, 
            nullptr
        },
    }
};

Board testCase2 = {
	{
		{0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7}, {0, 8}, {0, 9},
		{1, 0}, {1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}, {1, 7}, {1, 8}, {1, 9},
		{2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {2, 5}, {2, 6}, {2, 7}, {2, 8}, {2, 9},
		{3, 0}, {3, 1}, {3, 2}, {3, 3}, {3, 4}, {3, 5}, {3, 6}, {3, 7}, {3, 8}, {3, 9},
		{4, 0}, {4, 1}, {4, 2}, {4, 3}, {4, 4}, {4, 5}, {4, 6}, {4, 7}, {4, 8}, {4, 9},
		{5, 0}, {5, 1}, {5, 2}, {5, 3}, {5, 4}, {5, 5}, {5, 6}, {5, 7}, {5, 8}, {5, 9},
		{6, 0}, {6, 1}, {6, 2}, {6, 3}, {6, 4}, {6, 5}, {6, 6}, {6, 7}, {6, 8}, {6, 9},
		{7, 0}, {7, 1}, {7, 2}, {7, 3}, {7, 4}, {7, 5}, {7, 6}, {7, 7}, {7, 8}, {7, 9},
		{8, 0}, {8, 1}, {8, 2}, {8, 3}, {8, 4}, {8, 5}, {8, 6}, {8, 7}, {8, 8}, {8, 9},
		{9, 0}, {9, 1}, {9, 2}, {9, 3}, {9, 4}, {9, 5}, {9, 6}, {9, 7}, {9, 8}, {9, 9},
	},
	{
		{
			{
				{0, 0}, {0, 1},
				{1, 0}, {1, 1},
				{2, 0}, {2, 1},
			},
			ptrTo(2),
		},
		{
			{
				{0, 2},
				{1, 2},
				{2, 2},
			},
			ptrTo(1),
		},
		{
			{
				{0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7},
			},
		},
		{
			{
				{0, 8}, {0, 9},
				{1, 8}, {1, 9},
				{2, 8}, {2, 9},
				{3, 8}, {3, 9},
				{4, 8}, {4, 9},
			},
			ptrTo(3),
		},
		{
			{
				{1, 3}, {1, 4},
				{2, 3}, {2, 4},
				{3, 3}, {3, 4},
				{4, 3}, {4, 4},
			},
		},
		{
			{
				{1, 5}, {1, 6}, {1, 7},
			},
			ptrTo(2),
		},
		{
			{
				{2, 5}, {2, 6}, {2, 7},
				{3, 5}, {3, 6}, {3, 7},
				{4, 5}, {4, 6}, {4, 7},
			},
			ptrTo(3),
		},
		{
			{
				{3, 0}, {3, 1}, {3, 2},
				{4, 0}, {4, 1}, {4, 2},
				{5, 0}, {5, 1}, {5, 2},
			},
			ptrTo(4),
		},
		{
			{
				{5, 3}, {5, 4}, {5, 5},
				{6, 3}, {6, 4}, {6, 5},
				{7, 3}, {7, 4}, {7, 5},
			},
		},
		{
			{
				{5, 6},
				{6, 6},
				{7, 6},
				{8, 6},
				{9, 6},
			},
			ptrTo(3),
		},
		{
			{
				{5, 7}, {5, 8}, {5, 9},
				{6, 7}, {6, 8}, {6, 9},
			},
			ptrTo(2),
		},
		{
			{
				{6, 0}, {6, 1},
				{7, 0}, {7, 1},
				{8, 0}, {8, 1},
			},
			ptrTo(2),
		},
		{
			{
				{6, 2},
				{7, 2},
				{8, 2},
			},
		},
		{
			{
				{7, 7}, {7, 8}, {7, 9},
				{8, 7}, {8, 8}, {8, 9},
				{9, 7}, {9, 8}, {9, 9},
			},
			ptrTo(2),
		},
		{
			{
				{8, 3}, {8, 4}, {8, 5},
			},
		},
		{
			{
				{9, 0}, {9, 1}, {9, 2}, {9, 3}, {9, 4}, {9, 5},
			},
		},
	},
};

Board testCase3 = {
	{
			{0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7}, {0, 8}, {0, 9},
			{1, 0}, {1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}, {1, 7}, {1, 8}, {1, 9},
			{2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {2, 5}, {2, 6}, {2, 7}, {2, 8}, {2, 9},
			{3, 0}, {3, 1}, {3, 2}, {3, 3}, {3, 4}, {3, 5}, {3, 6}, {3, 7}, {3, 8}, {3, 9},
			{4, 0}, {4, 1}, {4, 2}, {4, 3}, {4, 4}, {4, 5}, {4, 6}, {4, 7}, {4, 8}, {4, 9},
			{5, 0}, {5, 1}, {5, 2}, {5, 3}, {5, 4}, {5, 5}, {5, 6}, {5, 7}, {5, 8}, {5, 9},
			{6, 0}, {6, 1}, {6, 2}, {6, 3}, {6, 4}, {6, 5}, {6, 6}, {6, 7}, {6, 8}, {6, 9},
			{7, 0}, {7, 1}, {7, 2}, {7, 3}, {7, 4}, {7, 5}, {7, 6}, {7, 7}, {7, 8}, {7, 9},
			{8, 0}, {8, 1}, {8, 2}, {8, 3}, {8, 4}, {8, 5}, {8, 6}, {8, 7}, {8, 8}, {8, 9},
			{9, 0}, {9, 1}, {9, 2}, {9, 3}, {9, 4}, {9, 5}, {9, 6}, {9, 7}, {9, 8}, {9, 9},
		},
		{
			{
				{
					{0, 0},
					{1, 0},
				},
			},
			{
				{
					{0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6},
				},
			},
			{
				{
					{0, 7},
					{1, 7},
				},
			},
			{
				{
					{0, 8}, {0, 9},
					{1, 8}, {1, 9},
				},
			},
			{
				{
					{1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6},
					{2, 1}, {2, 2}, {2, 3}, {2, 4}, {2, 5}, {2, 6},
					{3, 1}, {3, 2}, {3, 3}, {3, 4}, {3, 5}, {3, 6},
				},
				ptrTo(8),
			},
			{
				{
					{2, 0},
				},
			},
			{
				{
					{2, 7},
					{3, 7},
				},
			},
			{
				{
					{2, 8},
					{3, 8},
				},
			},
			{
				{
					{2, 9},
					{3, 9},
				},
			},
			{
				{
					{3, 0},
				},
			},
			{
				{
					{4, 0}, {4, 1}, {4, 2},
				},
			},
			{
				{
					{4, 3}, {4, 4},
					{5, 3}, {5, 4},
				},
			},
			{
				{
					{4, 5}, {4, 6},
					{5, 5}, {5, 6},
				},
			},
			{
				{
					{4, 7}, {4, 8}, {4, 9},
					{5, 7}, {5, 8}, {5, 9},
				},
			},
			{
				{
					{5, 0}, {5, 1}, {5, 2},
				},
			},
			{
				{
					{6, 0}, {6, 1}, {6, 2},
					{7, 0}, {7, 1}, {7, 2},
				},
			},
			{
				{
					{6, 3}, {6, 4}, {6, 5}, {6, 6}, {6, 7}, {6, 8},
					{7, 3}, {7, 4}, {7, 5}, {7, 6}, {7, 7}, {7, 8},
					{8, 3}, {8, 4}, {8, 5}, {8, 6}, {8, 7}, {8, 8},
				},
				ptrTo(8),
			},
			{
				{
					{6, 9},
					{7, 9},
					{8, 9},
					{9, 9},
				},
			},
			{
				{
					{8, 0},
					{9, 0},
				},
			},
			{
				{
					{8, 1}, {8, 2},
				},
			},
			{
				{
					{9, 1}, {9, 2}, {9, 3}, {9, 4}, {9, 5}, {9, 6}, {9, 7}, {9, 8},
				},
			},
		},
};

Board testCase4 = {
	{
		{0, 0}, {0, 1}, {0, 2}, {0, 7}, {0, 8}, {0, 9},
		{1, 0}, {1, 1}, {1, 2}, {1, 7}, {1, 8}, {1, 9},
		{2, 0}, {2, 1}, {2, 2}, {2, 7}, {2, 8}, {2, 9},
		{3, 0}, {3, 1}, {3, 2}, {3, 7}, {3, 8}, {3, 9},
		{4, 0}, {4, 1}, {4, 2}, {4, 3}, {4, 4}, {4, 5}, {4, 6}, {4, 7}, {4, 8}, {4, 9},
		{5, 0}, {5, 1}, {5, 2}, {5, 3}, {5, 4}, {5, 5}, {5, 6}, {5, 7}, {5, 8}, {5, 9},
		{6, 0}, {6, 1}, {6, 2}, {6, 3}, {6, 4}, {6, 5}, {6, 6}, {6, 7}, {6, 8}, {6, 9},
		{7, 0}, {7, 1}, {7, 2}, {7, 3}, {7, 4}, {7, 5}, {7, 6}, {7, 7}, {7, 8}, {7, 9},
		{8, 0}, {8, 1}, {8, 2}, {8, 7}, {8, 8}, {8, 9},
		{9, 0}, {9, 1}, {9, 2}, {9, 7}, {9, 8}, {9, 9},
		{10, 0}, {10, 1}, {10, 2}, {10, 7}, {10, 8}, {10, 9},
		{11, 0}, {11, 1}, {11, 2}, {11, 7}, {11, 8}, {11, 9},
	},
	{
		{
			
			{
				{0, 0}, {0, 1}, {0, 2},
				{1, 2},
				{2, 2},
				{3, 2},
				{4, 2}, {4, 3}, {4, 4},
			},
				ptrTo(3),
		},
		{
			
			{
				{0, 7}, {0, 8}, {0, 9},
				{1, 7},
				{2, 7},
				{3, 7},
				{4, 5}, {4, 6}, {4, 7},
			},
				ptrTo(1),
		},
		{
			
			{
				{1, 0}, {1, 1},
				{2, 0}, {2, 1},
			},
				ptrTo(2),
		},
		{
			
			{
				{1, 8}, {1, 9},
				{2, 8}, {2, 9},
			},
				ptrTo(0),
		},
		{
			{
				{3, 0}, {3, 1},
				{4, 0}, {4, 1},
				{5, 0}, {5, 1},
				{6, 0}, {6, 1},
				{7, 0}, {7, 1},
				{8, 0}, {8, 1},
			},
				nullptr
		},
		{
			{
				{3, 8}, {3, 9},
				{4, 8}, {4, 9},
				{5, 8}, {5, 9},
				{6, 8}, {6, 9},
				{7, 8}, {7, 9},
				{8, 8}, {8, 9},
			},
				ptrTo(4),
		},
		{
			{
				{5, 2}, {5, 3},
				{6, 2}, {6, 3},
			},
				ptrTo(1),
		},	
		{
			{
				{5, 4}, {5, 5},
				{6, 4}, {6, 5},
			},
				nullptr
		},
		{
			{
				{5, 6}, {5, 7},
				{6, 6}, {6, 7},
			},
				nullptr
		},
		{
			{
				{9, 0}, {9, 1},
				{10, 0}, {10, 1},
			},
				nullptr
		},
		{
			{
				{9, 8}, {9, 9},
				{10, 8}, {10, 9},
			},
				ptrTo(1),
		},
		{
			{
				{7, 2}, {7, 3}, {7, 4},
				{8, 2},
				{9, 2},
				{10, 2},
				{11, 0}, {11, 1}, {11, 2},
			},
				ptrTo(4),
		},
		{
			{
				{7, 5}, {7, 6}, {7, 7},
				{8, 7},
				{9, 7},
				{10, 7},
				{11, 7}, {11, 8}, {11, 9},
			},
				ptrTo(4),
		},
	},
};
	
Board testCase5 = {
	{
		{0, 8},
		{1, 8}, {1, 9}, {1, 10}, {1, 11}, {1, 12},
		{2, 7}, {2, 8}, {2, 9}, {2, 11}, {2, 12},
		{3, 5}, {3, 6}, {3, 7}, {3, 8}, {3, 9}, {3, 11}, {3, 12},
		{4, 3}, {4, 4}, {4, 5}, {4, 7}, {4, 8}, {4, 9}, {4, 12},
		{5, 2}, {5, 3}, {5, 4}, {5, 5}, {5, 7}, {5, 8}, {5, 12}, {5, 13},
		{6, 2}, {6, 7}, {6, 8}, {6, 11}, {6, 12}, {6, 13}, {6, 14},
		{7, 1}, {7, 2}, {7, 3}, {7, 13}, {7, 14},
		{8, 1}, {8, 2}, {8, 3}, {8, 4}, {8, 5}, {8, 6}, {8, 12}, {8, 13}, {8, 14},
		{9, 1}, {9, 2}, {9, 3}, {9, 4}, {9, 5}, {9, 12}, {9, 13},
		{10, 1}, {10, 2}, {10, 3}, {10, 4}, {10, 12},
		{11, 0}, {11, 1}, {11, 2}, {11, 11}, {11, 12}, {11, 13},
		{12, 1}, {12, 11}, {12, 12}, {12, 13}, {12, 14},
		{13, 1}, {13, 2}, {13, 3}, {13, 4}, {13, 5}, {13, 8}, {13, 11}, {13, 12}, {13, 13},
		{14, 1}, {14, 2}, {14, 3}, {14, 5}, {14, 6}, {14, 7}, {14, 8}, {14, 9}, {14, 10}, {14, 11}, {14, 12}, {14, 13},
		{15, 1}, {15, 2}, {15, 3}, {15, 6}, {15, 7}, {15, 8}, {15, 9}, {15, 10}, {15, 11}, {15, 12}, {15, 13},
		{16, 7}, {16, 10}, {16, 11}, {16, 12},
	},
	{
		{
			{
				{0, 8},
				{1, 8}, {1, 9}, {1, 10}, {1, 11}, {1, 12},
				{2, 8}, {2, 9}, {2, 11}, {2, 12},
				{3, 8}, {3, 9}, {3, 11}, {3, 12},
				{4, 8}, {4, 9}, {4, 12},
				{5, 8}, {5, 12}, {5, 13},
				{6, 8}, {6, 11}, {6, 12}, {6, 13}, {6, 14},
				{7, 13}, {7, 14},
				{8, 12}, {8, 13}, {8, 14},
			},
				ptrTo(9),
		},
		{
			
			{
				{2, 7},
				{3, 5}, {3, 6}, {3, 7},
				{4, 3}, {4, 4}, {4, 5}, {4, 7},
				{5, 2}, {5, 3}, {5, 4}, {5, 5}, {5, 7},
				{6, 2}, {6, 7},
				{7, 1}, {7, 2}, {7, 3},
				{8, 1}, {8, 2}, {8, 3}, {8, 4}, {8, 5}, {8, 6},
			},
			ptrTo(8),
		},
		{
			{
				{9, 1}, {9, 2}, {9, 3}, {9, 4}, {9, 5},
				{10, 1}, {10, 2}, {10, 3}, {10, 4},
				{11, 0}, {11, 1}, {11, 2},
				{12, 1},
				{13, 1}, {13, 2}, {13, 3}, {13, 4}, {13, 5},
				{14, 1}, {14, 2}, {14, 3}, {14, 5}, {14, 6}, {14, 7},
				{15, 1}, {15, 2}, {15, 3}, {15, 6}, {15, 7},
				{16, 7},
			},
				ptrTo(11),
		},
		{
			{
				{9, 12}, {9, 13},
				{10, 12},
				{11, 11}, {11, 12}, {11, 13},
				{12, 11}, {12, 12}, {12, 13}, {12, 14},
				{13, 8}, {13, 11}, {13, 12}, {13, 13},
				{14, 8}, {14, 9}, {14, 10}, {14, 11}, {14, 12}, {14, 13},
				{15, 8}, {15, 9}, {15, 10}, {15, 11}, {15, 12}, {15, 13},
				{16, 10}, {16, 11}, {16, 12},
			},
				ptrTo(13),
		},
	},
};



int main() {
    vector<Board> boards = {testCase1, testCase2, testCase3, testCase4, testCase5};
	cout << "Explanation: Black cells are marked with an 'x' and white cells are marked with a space." << endl;
	cout << "The speed of execution depends on the complexity of the playing board" << endl;
    for (size_t i = 0; i < boards.size(); i++) {
		cout << "Press Enter to get started solving board №" << i + 1;
		cin.get();
        boards[i].setNumbers();
		cout << "Initial board of the "<< i + 1 << " board";
		cout << boards[i].display();
        if (boards[i].isCorrect()) {
            cout << "Board structure is correct" << endl;;
        } else {
            cout << "Board structure is incorrect" << endl;
			cout << "This board cannot be solved" << endl;
			continue;
		}
		cout << "Solving..." << endl;;
        auto start = chrono::high_resolution_clock::now();

        boards[i].run();

        auto end = chrono::high_resolution_clock::now();
		chrono::duration<double> duration = end - start;
        cout << "Finish solving in " << duration.count() / 60.0 << " minutes" << endl;
    }
}