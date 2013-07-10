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

