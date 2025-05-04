#include <vector>
#include <memory> 
#include <map>
#include <iostream>
#include <unordered_map>

using namespace std;

struct Cell {
	int i = 0, j = 0;  
    
    string Coords() const {
        char buf[32]; 
        snprintf(buf, sizeof(buf), "%d %d", i, j);
        return string(buf);
    }
};

struct Sector {
	vector<Cell> Cells;
	shared_ptr<int> Number = nullptr;
};

class Board {
    public:
        vector<Cell> Cells;
	    vector<Sector> Sectors;

        bool isCorrect();
    };

shared_ptr<int> ptrTo(int i) {
    return make_shared<int>(i);
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

void printBoard(const Board& board) {
    vector<vector<string>> output(10, vector<string>(10, "_"));
    for (const Sector& sector : board.Sectors) {
        if (sector.Number) {
            int i = sector.Cells[0].i;
            int j = sector.Cells[0].j;
            output[i][j] = to_string(*sector.Number);
        }
    }
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            cout << "|" << output[i][j];
        }
        cout << "|" << endl;
    }
}
 


int main() {
    printBoard(testCase1);
    cout << endl;
    if (testCase1.isCorrect()) {
        cout << "Board structure is correct" << endl;;
    } else {
        cout << "Board structure is incorrect" << endl;;
    }
}