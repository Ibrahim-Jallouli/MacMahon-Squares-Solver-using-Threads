#include <iostream>
#include <vector>
#include <fstream> 

int ROWS = 0;
int COLS = 0;

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



// une methode qui comporte les conditions necessaire pour que une tile doit etre placer dans la matrice
bool canPlace(int x, int y, const Tile& tile,char border_color) {
	
    if (x > 0 && tile.left != board[x - 1][y].right)
        return false;
    if (y > 0 && tile.top != board[x][y - 1].bottom)
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
bool solve(int x, int y,char border_color) {
    if (x == COLS) {
        x = 0;
        y++;
    }

    if (y == ROWS) {
        return true;
    }
    
    for (int tileIdx = 0; tileIdx < tiles.size(); tileIdx++) {
       Tile currentTile = tiles[tileIdx];
        if (canPlace(x, y, currentTile, border_color)) {
           board[x][y] = currentTile;
		   tiles.erase(tiles.begin() + tileIdx);
           if (solve(x + 1, y, border_color))
                return true;
           tiles.insert(tiles.begin() + tileIdx, currentTile);
        }
    }

    return false;
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

int main() {
    // Initialize the available tiles 
    readTilesFromFile();
    std::cout << "Rows: " << ROWS <<"    Cols: " << COLS << std::endl;
    
    // avoir un vector qui comporte toutes les couleurs disponible et supposer chaque fois que la solution exacte est la couleur i
    std::vector<char> border_colors = {'R', 'B', 'G'};

    for (char border_color : border_colors) {
        std::cout << "border color is: " << border_color << std::endl;
			board = std::vector<std::vector<Tile>>(COLS, std::vector<Tile>(ROWS)); //reInitialiser la board chaque fois une coleur n'est pas la sol 
        if (solve(0, 0, border_color)) {
            std::cout << "Solution found:" << std::endl;
                printBoard();
            break;
        } else {
            std::cout << "No solution found with border color " << border_color << std::endl;
        }
    }

    return 0;
}








