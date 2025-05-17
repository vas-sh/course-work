#include "board.h"
#include "initalBoards.h"
#include <chrono>
#include <iostream>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <vector>
using namespace std;

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
        cout << "Finish solving in " << duration.count() / 60.0 << " minutes\n" << endl;
    }
}