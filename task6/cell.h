#pragma once
#include <memory>
#include <string>

struct Cell {
	int i = 0, j = 0;  
    std::shared_ptr<int> NumberDisplay = nullptr;
    bool filled = false;
    
	std::string Coords() const;
	bool NextTo(const Cell& cell) const;
	bool Equal(const Cell& cell) const;
	std::string String() const;
};