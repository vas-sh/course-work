#pragma once
#include <memory>
#include <string>
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