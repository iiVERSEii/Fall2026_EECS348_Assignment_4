#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cctype>
#include <cstddef>

class SudokuGrid {
private:
    int grid[9][9];          // 0 means empty
    std::string sourceName;  // file name

public:
    SudokuGrid() : sourceName("") {
        for (int r = 0; r < 9; ++r)
            for (int c = 0; c < 9; ++c)
                grid[r][c] = 0;
    }

    void setSourceName(const std::string &name) {
        sourceName = name;
    }

    const std::string &getSourceName() const {
        return sourceName;
    }

    bool loadFromFile(const std::string &filename) {
        setSourceName(filename);
        std::ifstream in(filename);
        if (!in) return false;

        for (int r = 0; r < 9; ++r) {
            for (int c = 0; c < 9; ++c) {
                grid[r][c] = 0;
            }
        }

        std::vector<std::string> rows;
        std::string line;
        while (std::getline(in, line)) {
            rows.push_back(line);
        }

        if (rows.size() != 9) {
            return false;
        }

        for (int r = 0; r < 9; ++r) {
            int col = 0;
            for (char ch : rows[r]) {
                if (std::isspace(static_cast<unsigned char>(ch))) {
                    continue;
                }

                if (col >= 9) {
                    return false;
                }

                if (ch == '_' || ch == '.' || ch == '0') {
                    grid[r][col] = 0;
                } else if (ch >= '1' && ch <= '9') {
                    grid[r][col] = ch - '0';
                } else {
                    return false;
                }

                ++col;
            }

            if (col != 9) {
                return false;
            }
        }

        return true;
    }

    void print(std::ostream &out) const {
        for (int r = 0; r < 9; ++r) {
            if (r > 0 && r % 3 == 0) {
                out << "------+-------+------\n";
            }
            for (int c = 0; c < 9; ++c) {
                if (c > 0 && c % 3 == 0) out << "| ";
                if (grid[r][c] == 0) out << "_";
                else out << grid[r][c];
                if (c % 3 != 2) out << " ";
            }
            out << "\n";
        }
    }

    int get(int r, int c) const {
        return grid[r][c];
    }

    void set(int r, int c, int val) {
        grid[r][c] = val;
    }

    bool isRowValid(int r, int val) const {
        for (int c = 0; c < 9; ++c)
            if (grid[r][c] == val) return false;
        return true;
    }

    bool isColValid(int c, int val) const {
        for (int r = 0; r < 9; ++r)
            if (grid[r][c] == val) return false;
        return true;
    }

    bool isBlockValid(int r, int c, int val) const {
        int br = (r / 3) * 3;
        int bc = (c / 3) * 3;
        for (int rr = br; rr < br + 3; ++rr)
            for (int cc = bc; cc < bc + 3; ++cc)
                if (grid[rr][cc] == val) return false;
        return true;
    }

    bool isCellEmpty(int r, int c) const {
        return grid[r][c] == 0;
    }

    bool findEmptyCell(int &r, int &c) const {
        for (int i = 0; i < 9; ++i)
            for (int j = 0; j < 9; ++j)
                if (grid[i][j] == 0) {
                    r = i;
                    c = j;
                    return true;
                }
        return false;
    }

    bool isComplete() const {
        for (int r = 0; r < 9; ++r)
            for (int c = 0; c < 9; ++c)
                if (grid[r][c] == 0) return false;
        return true;
    }
};

class SudokuSolver {
private:
    SudokuGrid initial;
    std::vector<SudokuGrid> solutions;
    std::size_t solutionCount = 0;
    bool countOnly = false;

    bool canPlace(const SudokuGrid &g, int r, int c, int val) const {
        return g.isRowValid(r, val) &&
               g.isColValid(c, val) &&
               g.isBlockValid(r, c, val);
    }

    bool search(SudokuGrid &current, bool stopAfterFirst) {
        int r, c;
        if (!current.findEmptyCell(r, c)) {
            if (current.isComplete()) {
                ++solutionCount;
                if (!countOnly) {
                    solutions.push_back(current);
                }
                return true;
            }
            return false;
        }

        bool foundSolution = false;
        for (int val = 1; val <= 9; ++val) {
            if (canPlace(current, r, c, val)) {
                current.set(r, c, val);
                bool found = search(current, stopAfterFirst);
                current.set(r, c, 0); // backtrack

                if (found) {
                    if (stopAfterFirst) {
                        return true;
                    }
                    foundSolution = true;
                }
            }
        }

        return foundSolution;
    }

public:
    SudokuSolver(const SudokuGrid &g) : initial(g) {}

    void solve(bool stopAfterFirst = false, bool countOnly = false) {
        solutions.clear();
        solutionCount = 0;
        this->countOnly = countOnly;
        SudokuGrid working = initial;
        search(working, stopAfterFirst);
    }

    const std::vector<SudokuGrid> &getSolutions() const {
        return solutions;
    }

    void printReport(std::ostream &out) const {
        out << "============================================================\n";
        out << "Puzzle file: " << initial.getSourceName() << "\n";
        out << "============================================================\n";
        out << "Puzzle from file:\n";
        initial.print(out);
        out << "------------------------------------------------------------\n";

        if (solutionCount == 0) {
            out << "No solution found\n";
        } else if (countOnly) {
            out << "Total solutions found: " << solutionCount << "\n";
        } else {
            int idx = 1;
            for (const auto &sol : solutions) {
                out << "Solution " << idx++ << ":\n";
                sol.print(out);
                out << "------------------------------------------------------------\n";
            }
        }
        out << "============================================================\n\n";
    }
};

int main() {
    int option;
    std::cout << "Choose a solving option:\n"
              << "1. Find the first solution\n"
              << "2. Count all solutions\n"
              << "3. Print all solutions\n"
              << "Enter option (1-3): ";

    if (!(std::cin >> option) || option < 1 || option > 3) {
        std::cerr << "Invalid option. Enter 1, 2, or 3.\n";
        return 1;
    }

    std::vector<std::string> files = {
        "puzzle1.txt",
        "puzzle2.txt",
        "puzzle3.txt",
        "puzzle4.txt",
        "puzzle5.txt"
    };

    for (const auto &fname : files) {
        SudokuGrid grid;
        if (!grid.loadFromFile(fname)) {
            std::cout << "============================================================\n";
            std::cout << "Puzzle file: " << fname << "\n";
            std::cout << "============================================================\n";
            std::cout << "Could not load puzzle: file is missing or does not contain "
                         "exactly 9 rows of 9 valid symbols.\n";
            std::cout << "============================================================\n\n";
            continue;
        }

        SudokuSolver solver(grid);
        switch(option) {
            case 1:
                solver.solve(true, false); // Find first solution
                break;
            case 2:
                solver.solve(false, true); // Count all solutions
                break;
            case 3:
                solver.solve(false, false); // Find all solutions
                break;
        }
        solver.printReport(std::cout);
    }

    return 0;
}
