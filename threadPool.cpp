#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <fstream> 
#include <atomic>
#include <chrono>


int ROWS = 0;
int COLS = 0;
std::mutex solutionMutex;
std::atomic<bool> solutionFound(false);   // doit etre atomic pour qu'on soit sur qu'une seul thread peu la modifier a la fois
std::mutex mtx;
std::atomic<bool> stopThreads(false);


class ThreadPool {
public:
    ThreadPool(int numThreads);
    ~ThreadPool();

    template <typename Function>
    void addTask(Function&& task);

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable cv;
    bool stop;
};

ThreadPool::ThreadPool(int numThreads) : stop(false) {
    for (int i = 0; i < numThreads; ++i) {
        workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(queueMutex);
                    cv.wait(lock, [this] { return stop || !tasks.empty(); });
                    if (stop && tasks.empty()) {
                        return;
                    }
                    task = std::move(tasks.front());
                    tasks.pop();
                }
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    cv.notify_all();
    for (std::thread& worker : workers) {
        worker.join();
    }
}

template <typename Function>
void ThreadPool::addTask(Function&& task) {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        tasks.emplace(std::forward<Function>(task));
    }
    cv.notify_one();
}


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
    // Chaque thread travaille avec une copie du tableau et des tuiles
    std::vector<std::vector<Tile>> boardCopy = board;
    std::vector<Tile> tilesCopy = tiles;
    
    // Place the assigned tile in the 0,0 of the local board copy
    boardCopy[0][0] = tilesCopy[threadId];
    tilesCopy.erase(tilesCopy.begin() + threadId);
    
    // Try to find a solution with the assigned tile
    if (solve(1, 0, border_color, boardCopy, tilesCopy)) {
        // If a solution is found, => changement du valeur du solution found et finalement affecter la board local a la board global pour l'affichage
        std::lock_guard<std::mutex> lock(solutionMutex);
        board=boardCopy;
        printBoard();
		return;
    }

}

//  std::cout << "solution found  with border color " << border_color << std::endl;

int main() {
    // Initialize the available tiles 
    readTilesFromFile();
    std::cout << "Rows: " << ROWS << "    Cols: " << COLS << std::endl;

    // Create a ThreadPool with 4 threads
    ThreadPool threadPool(4);

    // avoir un vector qui comporte toutes les couleurs disponible et supposer chaque fois que la solution exacte pour le bordure est la couleur i
    std::vector<char> border_colors = {'B', 'R', 'G'};
    for (char border_color : border_colors) {
        std::cout << "border color is: " << border_color << std::endl;

        // Loop through tiles and add tasks to the ThreadPool
        for (int i = 0; i < tiles.size(); ++i) {
            threadPool.addTask([i, border_color]() {
                workerThread(i, border_color);
            });
        }
        if (solutionFound) {
            return 0;
        }
        std::cout << "No solution found with border color " << border_color << std::endl;
    }
    return 0;
}
