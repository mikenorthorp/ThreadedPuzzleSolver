
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define COLMULT (2)
#define ROWMULT (2)
#define LABEL_LEN (12)

#define NO_TAB (-1)
#define MAX_OUT_LINE_LEN (1000)
#define TMP_STRING_LEN (80)

#define UNUSED (-1)
#define USED (1)

#define BIGGEST_GRID_TO_PRINT (100)

typedef struct { 
  int north, east, south, west;
  char name[LABEL_LEN+1];
} cell_t;

/* Fill in the tab values for one cell with random tabs while ensuring that
   the sequence of tabs is unique and is consistent with the neighbouring
   cells.  We assume that we're going left-to-right and top-to-bottom
   in the puzzle grid so we're just looking for the east and south tab
   entries. */

void
set_cell( cell_t *cell, int **used, int range, int next_north )
{
  int east;
  int south;
  int found = 0;
  int i;

  /* Mark the current cell sequence as used. */

  used[cell->west][cell->north] = USED;

  /* Pick an actual number now. */

  do {
    east = random() % range;
    south = random() % range;
  } while ( (used[cell->north][east] == USED) ||
            (used[east][south] == USED) ||
            (used[south][cell->west] == USED) ||
            (used[east][next_north] == USED) || 
            ((east == south) && (cell->west == next_north)) ||
            (south == next_north)
          );

  /* Use the east and south values for the cell. */

  cell->east = east;
  cell->south = south;

  /* Inidicate that the new sequences of tabs are no longer available for
     other puzzle pieces. */

  used[cell->north][east] = USED;
  used[east][south] = USED;
  used[south][cell->west] = USED;
}

int
main( int argc, char ** argv )
{
  int return_value = 0;
  int rows, cols;
  char line[TMP_STRING_LEN];
  int **used;
  int *space;
  int i, j;
  cell_t **grid;
  cell_t *space2;
  int numrange;
  int seed;
  int next_top;
  char top[MAX_OUT_LINE_LEN];
  char mid[MAX_OUT_LINE_LEN];
  char bot[MAX_OUT_LINE_LEN];
  char sep[MAX_OUT_LINE_LEN];
  char add[TMP_STRING_LEN];


  if (argc < 4) {
    printf ("enter number of rows\n");
    fgets( line, TMP_STRING_LEN, stdin );
    rows = atoi( line );
  
    printf ("enter number of columns\n");
    fgets( line, TMP_STRING_LEN, stdin );
    cols = atoi( line );
  
    printf ("enter random seed \n");
    fgets( line, TMP_STRING_LEN, stdin );
    seed = atoi( line );
  } else {
    cols = atoi( argv[1] );
    rows = atoi( argv[2] );
    seed = atoi( argv[3] );
  }
  srandom( seed );

  numrange  = (rows+1) * (cols+1)/2;
  numrange  = (int)sqrt(10.0 * (rows+1) * (cols+1));
  if (numrange < 10) {
    numrange *= 2;
  }
  fprintf (stderr, "cols %d, rows %d, seed %d range %d\n", cols, rows, seed, numrange );

  /* Make space for the allocated numbers.  Use a trick for allocating 
     2d arrays.  The trick is to allocate a 1d array for the entire 2d array
     content and then index each row into this 1d array.  This way, we 
     have just two calls to malloc and can free all the space with
     two calls to free. */

  space = (int *) malloc( sizeof(int) * numrange * numrange );
  if (space != NULL) {
    used = (int **) malloc( sizeof( int * ) * numrange );
    for (i = 0; i < numrange; i++) {
      used[i] = space + i*numrange;
      for (j = 0; j < numrange; j++) {
        used[i][j] = UNUSED;
      }
    }
  }

  space2 = (cell_t *) malloc( sizeof(cell_t) * rows * cols );
  if (space != NULL) {
    grid = (cell_t **) malloc( sizeof( cell_t * ) * cols );
    for (i = 0; i < cols; i++) {
      grid[i] = space2 + i*rows;
    }
 
    /* Initialize the grid as not having any tab values yet. 
       For debugging convenience, use the grid coordinates as the
       cell name. */
 
    for (i = 0; i < cols; i++) {
      for (j = 0; j < rows; j++) {
        grid[i][j].north = NO_TAB;
        grid[i][j].east = NO_TAB;
        grid[i][j].south = NO_TAB;
        grid[i][j].west = NO_TAB;
        sprintf( grid[i][j].name, "%02dx%02d", i, j);
      }
    }
  }

  if ((space != NULL) && (space2 != NULL)) {

    /* Fill in the cells now. */

    next_top = random() % numrange;

    for (j = 0; j < rows; j++) {
      for (i = 0; i < cols; i++) {

        /* Get the top and left side of the cell from predecessors. */

        do {
          if (j == 0) {
            grid[i][0].north = next_top;
            next_top = random() % numrange;
          } else {
            grid[i][j].north = grid[i][j-1].south;
          }
          if (i == 0) {
            grid[0][j].west = random() % numrange;
          } else {
            grid[i][j].west = grid[i-1][j].east;
          }
        } while ( used[grid[i][j].west][grid[i][j].north] == USED);
  
        used[grid[i][j].west][grid[i][j].north] = USED;
  
        set_cell( &(grid[i][j]), used, numrange, (j == 0 ? next_top: (i+1 < cols ? grid[i+1][j-1].south : 0)) );
      }
    }

    /* Strcat can't handle strings that are too long. */

    if (cols <= BIGGEST_GRID_TO_PRINT) {
      /* Print out the grid solution to check on its validity. */
  
      for (j = 0; j < rows; j++) {
        top[0] = '\0'; mid[0] = '\0'; bot[0] = '\0'; sep[0]='\0';
        for (i = 0; i < cols; i++) {
          sprintf( add, "  %2d  |", grid[i][j].north );
          strcat( top, add );
          sprintf( add, "%2d  %2d|", grid[i][j].west, grid[i][j].east );
          strcat( mid, add );
          sprintf( add, "  %2d  |", grid[i][j].south );
          strcat( bot, add );
          sprintf( add, "-------" );
          strcat( sep, add );
        }
        fprintf (stderr, "%s\n%s\n%s\n%s\n", top, mid, bot, sep);
      }
    }

    /* Print the grid size and the four boundaries. */

    printf ("%d %d\n", cols, rows );

    printf ("top ");
    for (i = 0; i < cols; i++) {
      printf ("%d ", grid[i][0].north );
    }
    printf ("\n");
  
    printf ("bottom ");
    for (i = 0; i < cols; i++) {
      printf ("%d ", grid[i][rows-1].south );
    }
    printf ("\n");

    printf ("left ");
    for (j = 0; j < rows; j++) {
      printf ("%d ", grid[0][j].west );
    }
    printf ("\n");

    printf ("right ");
    for (j = 0; j < rows; j++) {
      printf ("%d ", grid[cols-1][j].east );
    }
    printf ("\n");

    /* Print out each piece. */

    for (j = 0; j < rows; j++) {
      for (i = 0; i < cols; i++) {
        printf ("%s %d %d %d %d\n",
           grid[i][j].name,
           grid[i][j].north,
           grid[i][j].east,
           grid[i][j].south,
           grid[i][j].west );
      }
    }

    free( space );
    free( used );
    free( space2 );
    free( grid );
  }

  return return_value;
}

