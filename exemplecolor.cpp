#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

int ROWS = 0;
int COLS = 0;
std::mutex mtx;
bool solutionFound = false;
std::mutex boardMutex;



// Structure qui va representer notre tile
struct Tile {
    char top, right, bottom, left;
};

// initialiser une matrice de Tile 
std::vector<std::vector<Tile>> board(COLS, std::vector<Tile>(ROWS));

// un vecteur de tiles qui va contenir les lignes de mon fichier.txt
std::vector<Tile> tiles;

// Function to print the board
/*void printBoard() {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            std::cout << board[j][i].top << " " << board[j][i].right << " " << board[j][i].bottom << " " << board[j][i].left << " | ";
        }
        std::cout << std::endl;
    }
} MY CODE !!*/    

//une resultat plus claire 
void printBoard() {
    for (int i = 0; i < ROWS; i++) {
        for (int row = 0; row < 3; row++) {
            for (int j = 0; j < COLS; j++) {
                if (row == 0) {
                    std::cout << " " << board[j][i].top << " ";
                } else if (row == 1) {
                    std::cout << board[j][i].left << " " << board[j][i].right ;
                   
                } else {
                    std::cout << " " << board[j][i].bottom << " ";
                }
            }
            std::cout << std::endl;
        }
         std::cout << std::endl;
    }
}

void readTilesFromFile() {
    std::ifstream inputFile("jeu.txt");
    if (!inputFile) {
        std::cerr << "file dosn't exist" << std::endl;
        return;
    }
    inputFile >> ROWS >> COLS;
    
    // une matrice de Tile avec COLS colonnes et ROWS Lignes
	board = std::vector<std::vector<Tile>>(COLS, std::vector<Tile>(ROWS));
    Tile tile;
    while (inputFile >> tile.top >> tile.right >> tile.bottom >> tile.left) {
        tiles.push_back(tile);
    }
    inputFile.close();
}


// une methode qui comporte les conditions necessaire pour que une tile doit etre placer dans la matrice
bool canPlace(int x, int y, const Tile& tile, char border_color, const std::vector<std::vector<Tile>>& localBoard) {
    if (x > 0 && tile.left != localBoard[x - 1][y].right)
        return false;
    if (y > 0 && tile.top != localBoard[x][y - 1].bottom)
        return false;
    if (x == 0) {
        if (tile.left != border_color)
            return false;
    }
    if (y == 0) {
        if (tile.top != border_color)
            return false;
    }
    if (x == COLS - 1) {
        if (tile.right != border_color)
            return false;
    }
    if (y == ROWS - 1) {
        if (tile.bottom != border_color)
            return false;
    }
    return true;
}


// methode de backtracking qui va remplir et afficher la matrice avec les tiles adequate si une solution a ete trouver si nn return false
bool solve(int x, int y, char border_color, const std::vector<Tile>& localTiles, std::vector<std::vector<Tile>>& localBoard) {
    if (x == COLS) {
        x = 0;
        y++;
    }

    if (y == ROWS) {
    	solutionFound=true;
        return true;
    }

    for (int tileIdx = 0; tileIdx < localTiles.size(); tileIdx++) {
    	if (solutionFound){
    		return false;
		}
        Tile currentTile = localTiles[tileIdx];
        if (canPlace(x, y, currentTile, border_color, localBoard)) {
        	localBoard[x][y] = currentTile; // Use the local board for placement
            std::vector<Tile> updatedTiles = localTiles; // Create a copy
            updatedTiles.erase(updatedTiles.begin() + tileIdx); // Remove the used tile
            if (solve(x + 1, y, border_color, updatedTiles, localBoard)) {
                return true;
            }
        }
    }

    return false;
}



void thread_function(char border_color) {
    std::cout << "Thread with border color " << border_color << " started." << std::endl;
    std::vector<std::vector<Tile>> threadBoard(COLS, std::vector<Tile>(ROWS)); // Create a local board for the thread
    std::vector<Tile> localTiles = tiles;  // creation d'une copie 
    if (solve(0, 0, border_color, localTiles, threadBoard)) {
	    {
	    	std::cout<<"soltuion found = "<<solutionFound ;
	        std::lock_guard<std::mutex> lock(mtx);
	            std::cout << "Solution found with border color " << border_color << std::endl;
	            board = threadBoard; // Update the global board with the local board
	            printBoard();
	    }
	    return;
    }
}


int main() {
    readTilesFromFile();
    std::cout << "Rows: " << ROWS << "    Cols: " << COLS << std::endl;
    std::vector<char> border_colors = {'R', 'B', 'G'};
    std::vector<std::thread> threads;

    for (char border_color : border_colors) {
        threads.emplace_back(thread_function, border_color);
    }

    for (std::thread& thread : threads) {
        thread.join();
    }

    if (!solutionFound) {
        std::cout << "No solution found with any border color." << std::endl;
    }
    return 0;
}
