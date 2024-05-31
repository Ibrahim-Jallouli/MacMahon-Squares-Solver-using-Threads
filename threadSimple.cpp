#include <iostream>
#include <vector>
#include <fstream> 
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>


int ROWS = 0;
int COLS = 0;
std::mutex solutionMutex;
std::atomic<bool> solutionFound(false);   // doit etre atomic pour qu'on soit sur qu'une seul thread peu la modifier a la fois
std::mutex mtx;


// Structure qui va representer notre tile
struct Tile {
    char top, right, bottom, left;
};

// initialiser une matrice de Tile 
std::vector<std::vector<Tile>> board(COLS, std::vector<Tile>(ROWS));

// un vecteur de tiles qui va contenir les lignes de mon fichier.txt
std::vector<Tile> tiles;

//Affichage du Board global
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
    
    // remplir le board
	board = std::vector<std::vector<Tile>>(COLS, std::vector<Tile>(ROWS));

    Tile tile;
    while (inputFile >> tile.top >> tile.right >> tile.bottom >> tile.left) {
        tiles.push_back(tile);
    }
    inputFile.close();
}


// une methode qui comporte les conditions necessaire pour que une tile doit etre placer dans la matrice
bool canPlace(int x, int y, const Tile& tile, char border_color, const std::vector<std::vector<Tile>>& boardCopy) {
    if (x > 0 && tile.left != boardCopy[x - 1][y].right)
        return false;
    if (y > 0 && tile.top != boardCopy[x][y - 1].bottom)
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
bool solve(int x, int y, char border_color,  std::vector<std::vector<Tile>>& boardCopy, std::vector<Tile>& tilesCopy) {
    if (x == COLS) {
        x = 0;
        y++;
    }

    if (y == ROWS) {
    	solutionFound=true;
        return true;
    }
    
     for (int tileIdx = 0; tileIdx < tilesCopy.size(); tileIdx++) {
     	if (solutionFound){
    		return false;
		}
    	Tile currentTile = tilesCopy[tileIdx];
        if (canPlace(x, y, currentTile, border_color, boardCopy)) {
           boardCopy[x][y] = currentTile;
		   tilesCopy.erase(tilesCopy.begin() + tileIdx);
           if (solve(x + 1, y, border_color,boardCopy,tilesCopy))
                return true;
           tilesCopy.insert(tilesCopy.begin() + tileIdx, currentTile);
        }
    }
    return false;
}

void workerThread(int threadId,char border_color) {
    // Chaque thread avec une copie du tableau et des tuiles
    std::vector<std::vector<Tile>> boardCopy = board;
    std::vector<Tile> tilesCopy = tiles;
    
    // Place the assigned tile in the 0,0 of the local board copy
    boardCopy[0][0] = tilesCopy[threadId];
    tilesCopy.erase(tilesCopy.begin() + threadId);
    
    // Try to find a solution with the assigned tile
    if (solve(1, 0, border_color, boardCopy, tilesCopy)) {
        // If a solution is found, => changement du valeur du solution found et finalement affecter la board local a la board global pour l'affichage
        std::lock_guard<std::mutex> lock(solutionMutex);
        std::cout << "Thread " << threadId << " found a solution:" << std::endl;
        board=boardCopy;
        printBoard();
		return;
    }

}




int main() {
    // Initialize the available tiles 
    readTilesFromFile();
    std::cout << "Rows: " << ROWS <<"    Cols: " << COLS << std::endl;
    
    // avoir un vector qui comporte toutes les couleurs disponible et supposer chaque fois que la solution exacte pour le bordure est la couleur i
    std::vector<char> border_colors = {'R', 'B', 'G'};
	for (char border_color : border_colors) {
		std::cout << "border color is: " << border_color << std::endl;
        std::vector<std::thread> threads;

	    for (int i = 0; i < tiles.size(); ++i) {
	        threads.emplace_back(workerThread, i,border_color);
	    }
	
	    // Le thread principal doit attendre jusqu'a ce que les threads aient termine leurs taches
	    for (auto& thread : threads) {
	        thread.join();
	    }
	    if(solutionFound){
	    	return 0;
		}
    	std::cout << "No solution found with border color " << border_color << std::endl;

	}
    return 0;
}








