
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#define LABEL_LEN (12)
#define MAXLINELEN (8096)

/* Each puzzle piece is an array of 4 tabs ordered clockwise and starting
   at the top (north) tab. */

#define NORTH_TAB (0)
#define EAST_TAB (1)
#define SOUTH_TAB (2)
#define WEST_TAB (3)

#define NO_PIECE_INDEX (-1)

typedef struct
{
    int tab[4];
    char name[LABEL_LEN + 1];
} piece_t;

/* We'll want to keep all the pieces and the number of them together. */

typedef struct
{
    piece_t *pieces;
    int numpieces;
} piece_list_t;

/* A cell in the grid knows its north and west tabs.  Since this cell is
   expected to be in a grid, its east tab is the same as the west tab of the
   next cell to the right.  Its south tab is the same as the north tab of the
   cell immediately below.

    When we define our grid, cell entry (0, 0) will be the top left
    corner with the "y" values increasing as you go down the page.
    That's the opposite of typical geometry from high school but
    is not uncommon in graphics systems.
*/

/* Added a lock to the cell to keep things synced when solving with threads */
typedef struct
{
    sem_t threadLock;
    int north;
    int west;
    piece_t *piece;
} cell_t;

typedef struct
{
    cell_t **cells;
    int numcols;
    int numrows;
    int testnum;
} grid_t;

/* Create a sturct for all of the fill_any_dir arguments to pass into threads */
typedef struct
{
    grid_t *grid;
    piece_list_t *piece_list;
    int start_col;
    int start_row;
    int inc_index;
} fill_t;


/* Display the names of all the pieces in the grid. */

void
print_grid( grid_t *grid )
{
    int i, j;

    for (j = 0; j < grid->numrows; j++)
    {
        for (i = 0; i < grid->numcols; i++)
        {
            if (grid->cells[i][j].piece == NULL)
            {
                printf (". ");
            }
            else
            {
                printf ("%s ", grid->cells[i][j].piece->name);
            }
        }
        printf ("\n");
    }
}

/* Display the set of tabs of the puzzle. */

void
print_edges( grid_t *grid )
{
    int i, j;

    for (j = 0; j < grid->numrows; j++)
    {
        for (i = 0; i < grid->numcols; i++)
        {
            printf ("   %3d", grid->cells[i][j].north);
        }
        printf ("\n");
        for (i = 0; i <= grid->numcols; i++)
        {
            printf ("%3d   ", grid->cells[i][j].west);
        }
        printf ("\n");
    }
    for (i = 0; i < grid->numcols; i++)
    {
        printf ("   %3d", grid->cells[i][grid->numrows].north);
    }
    printf ("\n");
}

/* Retrieve the puzzle configuration from stdin. */

int
get_input( grid_t *grid, piece_list_t *piece_list )
{
    int return_value = 0;
    char line[MAXLINELEN + 1];
    cell_t *space;
    char *token;
    char *context;
    int i;
    int *cols = &(grid->numcols);
    int *rows = &(grid->numrows);
    piece_t **piece = &(piece_list->pieces);

    /* Get the grid size. */

    fgets( line, MAXLINELEN, stdin );
    line[MAXLINELEN] = '\0';

    sscanf( line, "%d %d", cols, rows );

    /* Use a "trick" for two dimensional array space management.  Allocate
       the entire 2d array as a sing sequence of cells and then build up
       the 2d index by pointing into parts of that space.  The trick means
       that we can release the whole 2d array with just two calls to "free". */

    space = (cell_t *) malloc( (*rows + 1) * (*cols + 1) * sizeof( cell_t ) );
    grid->cells = (cell_t **) malloc(      (*cols + 1) * sizeof( cell_t *) );

    if ((space != NULL) && (grid->cells != NULL))
    {
        /* Initialize the space. */

        for (i = 0; i < *cols + 1; i++)
        {
            grid->cells[i] = space + i * (*rows + 1);
        }

        for (i = 0; i < (*rows + 1) * (*cols + 1); i++)
        {
            space[i].north = NO_PIECE_INDEX;
            space[i].west = NO_PIECE_INDEX;
            space[i].piece = NULL;

            // Set up lock for this cell when the pieces are being set up
            sem_init(&space[i].threadLock, 0, 1);
        }

        /* Get the top. */

        fgets( line, MAXLINELEN, stdin );
        context = NULL;
        token = strtok_r( line, " \n", &context);
        for (i = 0; i < *cols; i++)
        {
            grid->cells[i][0].north = atoi( strtok_r( NULL, " \n", &context ));
        }

        /* Get the bottom. */

        fgets( line, MAXLINELEN, stdin );
        context = NULL;
        token = strtok_r( line, " \n", &context );
        for (i = 0; i < *cols; i++)
        {
            grid->cells[i][*rows].north = atoi( strtok_r( NULL, " \n", &context ));
        }

        /* Get the left side. */

        fgets( line, MAXLINELEN, stdin );
        context = NULL;
        token = strtok_r( line, " \n", &context );
        for (i = 0; i < *rows; i++)
        {
            grid->cells[0][i].west = atoi( strtok_r( NULL, " \n", &context ));
        }

        /* Get the right. */

        fgets( line, MAXLINELEN, stdin );
        context = NULL;
        token = strtok_r( line, " \n", &context );
        for (i = 0; i < *rows; i++)
        {
            grid->cells[*cols][i].west = atoi( strtok_r( NULL, " \n", &context ));
        }

        /* Get the pieces now. */

        *piece = (piece_t *)malloc( *rows **cols * sizeof( piece_t ) );
        piece_list->numpieces = *rows **cols;
        if (*piece != NULL)
        {
            for (i = 0; i < *rows **cols; i++)
            {
                fgets( line, MAXLINELEN, stdin );
                sscanf( line, "%s %d %d %d %d", (*piece)[i].name,
                        &((*piece)[i].tab[NORTH_TAB]), &((*piece)[i].tab[EAST_TAB]),
                        &((*piece)[i].tab[SOUTH_TAB]), &((*piece)[i].tab[WEST_TAB]) );

            }
        }

        return_value = 1;
    }

    return return_value;
}

/* Free up the memory that get_input allocates. */

void
release_memory( grid_t *grid, piece_list_t *piece_list )
{
    /* Get rid of all the pieces. */

    free( piece_list->pieces );
    piece_list->pieces = NULL;

    /* Get rid of the puzzle grid. */

    free( grid->cells[0] );
    free( grid->cells );
    grid->cells = NULL;
}


/* Have a function that traverses a row or a column, trying to fill in
   pieces.  Only puzzle grid spots that have at least two tabs defined
   are candidates to be filled in.

   There are four possible directions to travel:
     GO_LEFT_TO_RIGHT -- assumes that north and west of current cell are defined
                         (like the top row)
     GO_TOP_TO_BOTTOM -- assumes that north and east of current cell are defined
                         (like the rightmost column)
     GO_RIGHT_TO_LEFT -- assumes that east and south of current cell are defined
                         (like the bottom row)
     GO_BOTTOM_TO_TOP -- assumes that south and west of current cell are defined
                         (like the leftmost column)

   These four directions essentially let you go clockwise around the inside
   of the puzzle boundary if you want.
*/

#define GO_LEFT_TO_RIGHT (0)
#define GO_TOP_TO_BOTTOM (1)
#define GO_RIGHT_TO_LEFT (2)
#define GO_BOTTOM_TO_TOP (3)

void
fill_any_dir( grid_t *grid, piece_list_t *piece_list,
              int start_col, int start_row, int inc_index )
{
    int j;
    int found;
    int row, col;
    int col_inc[] = {1, 0, -1, 0};
    int row_inc[] = {0, 1, 0, -1};
    int count;

    row = start_row;
    col = start_col;

    /* Loop through the entire column / row and stop when we hit an edge or a
       puzzle grid cell that is already filled in. */

    while ((row >= 0) && (col >= 0) && (row < grid->numrows) &&
            (col < grid->numcols))
    {
        // Wait for piece to unlock and then solve it if not solved
        sem_wait(&grid->cells[col][row].threadLock);

        // If solved skip and unlock, else solve
        if (grid->cells[col][row].piece == NULL)
        {

            /* Ensure that we're ready for the piece by making sure that at least
               two tabs are defined. */

            count = 0;
            if (grid->cells[col][row].north != NO_PIECE_INDEX) count++;
            if (grid->cells[col][row].west != NO_PIECE_INDEX) count++;
            if (grid->cells[col][row + 1].north != NO_PIECE_INDEX) count++;
            if (grid->cells[col + 1][row].west != NO_PIECE_INDEX) count++;

            if (count >= 2)
            {
                /* Search the set of pieces for what will go in this grid position. */

                found = NO_PIECE_INDEX;
                for (j = 0; (j < grid->numcols * grid->numrows) && (found == NO_PIECE_INDEX); j++)
                {

                    /* I will find the first piece whose tabs match the defined tabs of
                       the grid cell.  This will find the unique pieces _if_ the grid
                       cell has at least two adjacent tabs that are not -1. */

                    if (
                        ((grid->cells[col][row].north == NO_PIECE_INDEX) ||
                         (grid->cells[col][row].north == piece_list->pieces[j].tab[NORTH_TAB])) &&
                        ((grid->cells[col + 1][row].west == NO_PIECE_INDEX) ||
                         (grid->cells[col + 1][row].west == piece_list->pieces[j].tab[EAST_TAB])) &&
                        ((grid->cells[col][row + 1].north == NO_PIECE_INDEX) ||
                         (grid->cells[col][row + 1].north == piece_list->pieces[j].tab[SOUTH_TAB])) &&
                        ((grid->cells[col][row].west == NO_PIECE_INDEX) ||
                         (grid->cells[col][row].west == piece_list->pieces[j].tab[WEST_TAB]))
                    )
                    {
                        found = j;
                    }
                }

                /* When we get the piece, fit it into the grid and update the tabs of
                   the grid for all surrounding grid cells. */

                if (found != NO_PIECE_INDEX)
                {
                    grid->cells[col][row].piece = &(piece_list->pieces[found]);
                    grid->cells[col][row].north = piece_list->pieces[found].tab[NORTH_TAB];
                    grid->cells[col + 1][row].west = piece_list->pieces[found].tab[EAST_TAB];
                    grid->cells[col][row + 1].north = piece_list->pieces[found].tab[SOUTH_TAB];
                    grid->cells[col][row].west = piece_list->pieces[found].tab[WEST_TAB];
                }
                else
                {
                    printf("Error piece not found!!!\n");
                }
            }

            // Unlock after solving the piece
            sem_post(&grid->cells[col][row].threadLock);

        }
        // Unlock if piece is solved after getting access to the critical section
        else
        {
            sem_post(&grid->cells[col][row].threadLock);
        }

        /* Go to the next grid cell in the direction given as a parameter. */
        row += row_inc[inc_index];
        col += col_inc[inc_index];

    }
}

/* This function is called when a new thread is created, and starts in a position
   dependent on the fill sturct contents */
void *puzzleThreadSolver(void *temp)
{
    // Set temp to a fill_t struct
    fill_t *fill = (fill_t *)temp;

    // Define variables to pass into fill_in_dir
    piece_list_t *piece_list;
    grid_t *grid;
    int i;
    int start_col;
    int start_row;
    int inc_index;

    // Get info to pass into fill_to_dir
    grid = fill->grid;
    piece_list = fill->piece_list;
    start_col = fill->start_col;
    start_row = fill->start_row;
    inc_index = fill->inc_index;

    /* Logic for running each thread and which corner and direction */
    // Call fill_to_dir based on inc_index and start row

    /* Left to right and right to left */
    // Top left
    if (inc_index == GO_LEFT_TO_RIGHT && start_row == 0)
    {
        for (i = start_row; i < grid->numrows; i++)
        {
            //printf("Test1 Col:%d Row%d\n", start_col, i);
            fill_any_dir(grid, piece_list, start_col, i, GO_LEFT_TO_RIGHT);
        }
    }

    // Bottom right
    if (inc_index == GO_RIGHT_TO_LEFT && start_row == grid->numrows - 1)
    {
        for (i = start_row; i >= 0; i--)
        {
            //printf("Test2 Col:%d Row%d\n", start_col, i);
            fill_any_dir(grid, piece_list, start_col, i, GO_RIGHT_TO_LEFT);
        }
    }

    //Top right
    if (inc_index == GO_RIGHT_TO_LEFT && start_row == 0)
    {
        for (i = start_row; i < grid->numrows; i++)
        {
            //printf("Test3 Col:%d Row%d\n", start_col, i);
            fill_any_dir(grid, piece_list, start_col, i, GO_RIGHT_TO_LEFT);
        }
    }

    //Bottom left
    if (inc_index == GO_LEFT_TO_RIGHT && start_row == grid->numrows - 1)
    {
        for (i = start_row; i >= 0; i--)
        {
            //printf("Test4 Col:%d Row%d\n", start_col, i);
            fill_any_dir(grid, piece_list, start_col, i, GO_LEFT_TO_RIGHT);
        }
    }

    /* Top to bottom and bottom to top */

    // Top left
    if (inc_index == GO_TOP_TO_BOTTOM && start_col == 0)
    {
        for (i = start_col; i < grid->numcols; i++)
        {
            //printf("Test5 Col:%d Row%d\n", i, start_row);
            fill_any_dir(grid, piece_list, i, start_row, GO_TOP_TO_BOTTOM);
        }
    }

    // Bottom right
    if (inc_index == GO_BOTTOM_TO_TOP && start_col == grid->numcols - 1)
    {
        for (i = start_col; i >= 0; i--)
        {
            //printf("Test6 Col:%d Row%d\n", i, start_row);
            fill_any_dir(grid, piece_list, i, start_row, GO_BOTTOM_TO_TOP);
        }
    }

    //Top right
    if (inc_index == GO_TOP_TO_BOTTOM && start_col == grid->numcols - 1)
    {
        for (i = start_col; i >= 0; i--)
        {
            //printf("Test7 Col:%d Row%d\n", i, start_row);
            fill_any_dir(grid, piece_list, i, start_row, GO_TOP_TO_BOTTOM);
        }
    }

    //Bottom left
    if (inc_index == GO_BOTTOM_TO_TOP && start_col == 0)
    {
        for (i = start_col; i < grid->numcols; i++)
        {
            //printf("Test8 Col:%d Row%d\n", i, start_row);
            fill_any_dir(grid, piece_list, i, start_row, GO_BOTTOM_TO_TOP);
        }
    }

    return NULL;
}

int
main( int argc, char **argv )
{

    /* Take in the threads from command line using argv and create
       that many threads */
    int numThreads;
    if (argc >= 2)
    {
        numThreads = atoi(argv[1]);
    }
    else
    {
        printf("Please put the number of threads you want as an argument");
        return 1;
    }

    // Define threads array
    pthread_t puzzleThread[numThreads];
    fill_t fillArray[numThreads];

    // Define values to get from input for grid and piece list
    int return_value = 0;
    piece_list_t piece_list;
    grid_t grid;
    int i;

    // Get input from STDIN for piece list and grid
    if (get_input( &grid, &piece_list ))
    {

        /* Create all of the structs to pass in with the threads */
        for (i = 0; i < numThreads; i++)
        {
            // Create fillStruct for this thread
            fill_t fillStruct;

            fillStruct.grid = &grid;
            fillStruct.piece_list = &piece_list;

            // Pick which corner to put the thread in, and to go which direction
            if (i % 8 == 0) // Top left
            {
                fillStruct.start_col = 0;
                fillStruct.start_row = 0;
                fillStruct.inc_index = GO_LEFT_TO_RIGHT;
            }
            else if (i % 8 == 1) // Bottom right
            {
                fillStruct.start_col = grid.numcols - 1;
                fillStruct.start_row = grid.numrows - 1;
                fillStruct.inc_index = GO_RIGHT_TO_LEFT;
            }
            else if (i % 8 == 2) // Top right
            {
                fillStruct.start_col = grid.numcols - 1;
                fillStruct.start_row = 0;
                fillStruct.inc_index = GO_RIGHT_TO_LEFT;
            }
            else if ( i % 8 == 3) // Bottom left
            {
                fillStruct.start_col = 0;
                fillStruct.start_row = grid.numrows - 1;
                fillStruct.inc_index = GO_LEFT_TO_RIGHT;
            }
            else if ( i % 8 == 4) // Top left top-bottom
            {
                fillStruct.start_col = 0;
                fillStruct.start_row = 0;
                fillStruct.inc_index = GO_TOP_TO_BOTTOM;
            }
            else if ( i % 8 == 5) // Bottom right bottom-top
            {
                fillStruct.start_col = grid.numcols - 1;
                fillStruct.start_row = grid.numrows - 1;
                fillStruct.inc_index = GO_BOTTOM_TO_TOP;
            }
            else if ( i % 8 == 6) // Top right top-bottom
            {
                fillStruct.start_col = grid.numcols - 1;
                fillStruct.start_row = 0;
                fillStruct.inc_index = GO_TOP_TO_BOTTOM;
            }
            else if ( i % 8 == 7) // Bottom left bottom-top
            {
                fillStruct.start_col = 0;
                fillStruct.start_row = grid.numrows - 1;
                fillStruct.inc_index = GO_BOTTOM_TO_TOP;
            }

            // Put the fillStruct into the array of fill_t
            fillArray[i] = fillStruct;
        }

        /* Create all of the threads at once */
        for (i = 0; i < numThreads; i++)
        {
            // Create a single puzzle thread to solve starting in top left
            if (pthread_create(&puzzleThread[i], NULL, &puzzleThreadSolver, &fillArray[i]))
            {
                fprintf(stderr, "Error creating thread\n");
            }
        }

        /* End Thread creation */

        // Wait for threads to finish that are created and join them to main
        for (i = 0; i < numThreads; i++)
        {
            // Wait for puzzle threads to end
            if (pthread_join(puzzleThread[i], NULL))
            {
                fprintf(stderr, "Error joining thread\n");
                return_value = 2;
            }
        }

        /* Show what the puzzle came out to be. */

        print_grid( &grid );

        release_memory( &grid, &piece_list );
    }

    // Exit the program with return value
    return return_value;
}
