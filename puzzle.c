
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

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

typedef struct
{
    int north;
    int west;
    piece_t *piece;
} cell_t;

typedef struct
{
    cell_t **cells;
    int numcols;
    int numrows;
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
            (col < grid->numcols) && (grid->cells[col][row].piece == NULL))
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

        }

        /* Go to the next grid cell in the direction given as a parameter. */

        row += row_inc[inc_index];
        col += col_inc[inc_index];
    }
}

/* This function is called when a new thread is created, and starts in a position
   dependent on the fill sturct contents */
void *puzzleThreadSolver(void *fill)
{
    printf("Testing thread\n");


    return NULL;
}

int
main( int argc, char **argv )
{

    /* Take in the threads from command line using argv and create
       that many threads */

    int numThreads = 2;

    // Define threads
    pthread_t puzzleThread;
    pthread_t puzzleThread2;

    int return_value = 0;
    piece_list_t piece_list;
    grid_t grid;
    int i;

    if (get_input( &grid, &piece_list ))
    {

        // If 1 thread
        if (numThreads >= 1)
        {
            printf("Creating thread 1\n");
            // Create fillStruct for this thread
            fill_t *fillStruct;

            // Allocate memory for fill_t
            fillStruct = (fill_t *) malloc( sizeof( fill_t ) );

            fillStruct->grid = &grid;
            fillStruct->piece_list = &piece_list;
            fillStruct->start_col = 0;
            fillStruct->start_row = 0;
            fillStruct->inc_index = GO_RIGHT_TO_LEFT;

            // Create a single puzzle thread to solve starting in top left
            if (pthread_create(&puzzleThread, NULL, puzzleThreadSolver, fillStruct))
            {
                fprintf(stderr, "Error creating thread\n");
            }
        }

        // If 2 threads
        if (numThreads >= 2)
        {
            printf("Creating thread 2\n");
               // Create fillStruct for this thread
            fill_t *fillStruct;

            // Allocate memory for fill_t
            fillStruct = (fill_t *) malloc( sizeof( fill_t ) );

            fillStruct->grid = &grid;
            fillStruct->piece_list = &piece_list;
            fillStruct->start_col = grid.numcols - 1;
            fillStruct->start_row = grid.numrows - 1;
            fillStruct->inc_index = GO_LEFT_TO_RIGHT;

            // Create a single puzzle thread to solve starting in bottom right
            if (pthread_create(&puzzleThread2, NULL, puzzleThreadSolver, fillStruct))
            {
                fprintf(stderr, "Error creating thread\n");
            }
        }


        /* If more than 8 threads loop rest */

        /* End Thread creation */

        /* Solve the puzzle row by row starting with the top row. */

        /*
        for (i = 0; i < grid.numrows; i++)
        {
            fill_any_dir(&grid, &piece_list, 0, i, GO_LEFT_TO_RIGHT);
        }
        */

        // Wait for puzzle threads to end
        if (pthread_join(puzzleThread, NULL))
        {
            fprintf(stderr, "Error joining thread\n");
            return_value = 2;
        }

        if (pthread_join(puzzleThread2, NULL))
        {
            fprintf(stderr, "Error joining thread\n");
            return_value = 2;
        }

        /* Show what the puzzle came out to be. */

        print_grid( &grid );

        release_memory( &grid, &piece_list );
    }

    return return_value;
}
