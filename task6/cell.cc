#include "cell.h"
#include <string>

std::string Cell::Coords() const {
	char buf[32]; 
	snprintf(buf, sizeof(buf), "%d %d", i, j);
	return std::string(buf);
}

bool Cell::NextTo(const Cell& cell) const {
	if (i == cell.i) {
		return j == cell.j - 1 || j == cell.j + 1;
	}
	if (j == cell.j) {
		return i == cell.i - 1 || i == cell.i + 1;
	}
	return false;
}

bool Cell::Equal(const Cell& cell) const {
	return i == cell.i && j == cell.j;
}

std::string Cell::String() const {
	if (filled) {
		return "|x";
	}
	if (NumberDisplay != nullptr) {
		char buffer[16];
		snprintf(buffer, sizeof(buffer), "|%d", *NumberDisplay);
		return std::string(buffer);
	}
	return "|_";
}  