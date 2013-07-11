Puzzle
======

Overview
--------
The program fills in the puzzle one row at a time, starting in the
top left corner.  When starting at a grid cell, the program searches
through all of the puzzle pieces to find a match for the grid cell.
The key function is fill_any_dir, which is designed to fill the
grid along any one of the sides of the grid.  However, this puzzle
program only uses one direction.

Installation
------------

1. Copy files into working directory.
2. run `make` command in terminal

How To Run
----------

Run the command `time ./puzzle n < yyy > /dev/null` where yyy is the filename of
the puzzle generated to try and solve.

If you do not have a puzzle generated, refer to Generate Documentation below.

You can remove the time and /dev/null if
you want to see the output of the puzzle and no time.

Method Descriptions
------------------

#### generate.c - Puzzle Generator ####

This file generates the puzzle and solved puzzle for use in the main puzzle
program. Refer to Generate documentation below for more information.

#### puzzle.c - Main File ####

This takes in a puzzle from STDIN and solves it using multiple threads, then
outputs the solved puzzle to STDOUT.

#### Puzzle Functions ####

int main(int argc, char **argv );

	- This main function reads in the number of threads as an argument when starting your program. It
	also creates the different threads depending on the number read in, all with slightly different directions
	and starting in different corners. It then joins the threads once they are all created, prints out the completed
	puzzle, and exits the program.

void *puzzleThreadSolver(void *temp);

	- This function is called whenever a thread is created. It handles the logic on how each thread
	will solve the puzzle, depending on its direction and starting position.

void fill_any_dir( grid_t *grid, piece_list_t *piece_list,
              int start_col, int start_row, int inc_index );

    - This function actually solves the puzzle row or column it is currently on. It has locking
    on individual pieces and checks for if a piece is being solved or is solved.

void release_memory( grid_t *grid, piece_list_t *piece_list );

	- This function frees up memory for the grid and piece_list

int get_input( grid_t *grid, piece_list_t *piece_list );

	- This function gets the input from the file in STDIN and stores it in the
	grid and piece list structs. It also initializes the semaphores in each cell.

void print_edges( grid_t *grid );

	- This displays the set of tabs of the puzzle

void print_grid( grid_t *grid );

	- This prints out the grid of the puzzle

#### Puzzle Structs ####

piece_t
	- This is the struct for the piece of the puzzle

piece_list_t
	- This is the struct for the list of pieces of the puzzle

cell_t
	- This is the struct for a cell, it has a semaphore lock added to it

grid_t
	- This is a struct for entire grid

fill_t
	- This is a struct that threads pass in on creation, to be used in the fill_in_dir function.



Data structures
---------------

All the pieces are stored in a one-dimensional array.

The grid is stored as a two-dimensional array.  Although puzzle
piece tabs are common across neighbouring cells, we only want to
store them once.  So, a cell of the grid stores the puzzle piece
tab number for the tab going up (north) and to the left (west) of
the cell.  To get the tab below, we need to get the "north" tab
from the grid cell below.  To get the tab on the right of the cell,
we need to get the "west" tab from the grid cell immediately right
of the current cell.  A consequence of this organization is that
the grid ends up storing one extra row and one extra column of data
to give the right and bottom boundaries of the grid.






Generate
========

Overview
--------
Sample puzzles can be constructed by hand, but it is troublesome.
The "generate" program will create a random puzzle for you, with a
size that you can select.  The program ensures that any pair of
neighbouring tabs on puzzle pieces in the same puzzle is unique
within the puzzle.  Consequently, when solving the puzzle, if a
piece has two neighbouring tabs that match the grid then you have
the right piece for the puzzle grid location.

The program prints both the solution to the puzzle and the input
file that you would supply to the "puzzle" program.  The solution
is sent to stderr while the input file is sent to stderr.  By sending
the output to two different streams, you can capture the data in
different files.  Since we can ask for big puzzles, the solution
is only printed if the puzzle has fewer than 100 columns.

Puzzle pieces are given a name that corresponds to their locations
in the grid to make identifying a solution easier.  The name is of
the form colxrow where col is the column number where the piece
belongs and row is the row where the piece belongs, with row 0 being
the top row.

Operation
---------
The program accepts three parameters.  In order, they are the number
of columns for the grid, the number of rows for the grid, and a
seed for the random number generator.  Although we use a random
number generator, you can re-create a grid repeatedly if you use
the same seed and grid dimensions.

Given that output goes to two streams, you can capture the streams
to different files on bluenose as follows:

  ./generate 3 5 10 > file1 2> file2

This command creates a grid with 3 columns and 5 rows.  It uses 10
as the seed for the random number generator.  The input for the
"puzzle" program goes to "file1" while the solution to the puzzle
goes to "file2".

The puzzle pieces are printed in the same order as in the puzzle.
That's not an ideal order for testing, but it makes it easier to
ensure that all the pieces are printed.  To shuffle up the piece order,
store the pieces to a file (called "xxx" below) and then do the following
UNIX commands (the one with %% is not a UNIX command):

  head -5 xxx > xxx.head
  %% use an editor to edit file xxx and remove the first 5 lines
  sort -n -k 3 xxx > xxx.sorted
  cat xxx.head xxx.sorted > xxx
  rm xxx.head xxx.sorted

Data Structures
---------------

We use two data structures in the program.  The first data structure
is a grid that matches the grid itself.  Each cell stores all four
tab values for the puzzle piece that goes in that grid cell.

The second data structure is a 2d grid of essentially boolean values
(but modeled as ints).  The 2d grid has a row and a column for every
possible tab value that a puzzle piece can have.  The grid tracks
which sequences of tab values have been used in pieces already.
This array is used to ensure that we don't re-use tab sequences.

Test Cases
==========

See testCases.txt for test cases done

Citations
=========

The non multithreaded portion of the puzzle.c and the entire generate.c was
created by Mike McAllister of Dalhousie University

