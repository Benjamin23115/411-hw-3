#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define MAX_N 1000 // Define a reasonable maximum size for n

// Initialize the grid with randomized 0 or 1's
void initialize(int grid[MAX_N][MAX_N], int n)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            grid[i][j] = rand() % 2;
        }
    }
}

// Print the grid
void printGrid(int grid[MAX_N][MAX_N], int n)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            printf("%d ", grid[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

// Calculate live neighbors of a cell
int calculateLiveNeighbors(int grid[MAX_N][MAX_N], int x, int y, int n)
{
    int liveNeighbors = 0;
    // Check the 8 neighbors of the cell (up, down, left, right, and diagonals)
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            int newX = x + i;
            int newY = y + j;
            if (newX >= 0 && newX < n && newY >= 0 && newY < n)
            {
                liveNeighbors += grid[newX][newY];
            }
        }
    }
    // Subtract cell itself (don't include the cell itself in the amount of live neighbors)
    return liveNeighbors - grid[x][y];
}

int main(int argc, char *argv[])
{
    if (argc != 4) {
        printf("Usage: %s <n> <timesteps> <thread_num>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int t = atoi(argv[2]);
    int num_threads = atoi(argv[3]);

    if (n <= num_threads || n % num_threads != 0) {
        printf("Error: n must be greater than the number of threads and divisible by the number of threads.\n");
        printf("Example: srun ./golOMP 8 4 4.\n");
        return 1;
    }

    int grid[MAX_N][MAX_N];
    int nextGrid[MAX_N][MAX_N];

    // Initialize the grid
    srand(time(NULL));
    initialize(grid, n);

    omp_set_num_threads(num_threads);

    double startTime = omp_get_wtime();
    for (int step = 0; step < t; step++)
    {
        #pragma omp parallel
        {
            // Parallelize the computation of the next state
            #pragma omp for
            for (int i = 0; i < n; i++)
            {
                for (int j = 0; j < n; j++)
                {
                    int liveNeighbors = calculateLiveNeighbors(grid, i, j, n);
                    if (grid[i][j] == 1)
                    {
                        // If cell is alive check if it has less than 2 or more than 3 live neighbors and update its state accordingly
                        nextGrid[i][j] = (liveNeighbors < 3 || liveNeighbors > 5) ? 0 : 1;
                    }
                    else
                    {
                        // If cell is dead check if it has 3 neighbors and update its state accordingly
                        nextGrid[i][j] = (liveNeighbors >= 3 && liveNeighbors <= 5) ? 1 : 0;
                    }
                }
            }

            // Copy the nextGrid to grid
            #pragma omp for
            for (int i = 0; i < n; i++)
            {
                for (int j = 0; j < n; j++)
                {
                    grid[i][j] = nextGrid[i][j];
                }
            }
        }
    }
    double endTime = omp_get_wtime();

    printf("Final Grid:\n");
    printGrid(grid, n);

    printf("total time taken: %f \n", endTime - startTime);

    return 0;
}
