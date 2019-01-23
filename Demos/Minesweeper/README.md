# Minesweeper recreation using Crayon

A recreation of the original Windows 2000 and XP versions of minesweeper for the Dreamcast using KallistiOS and my Crayon library

### Data structures

uint16_t \*coordGrid
+ Stores the screen coordinates for each tile, function populates this to make the grid

uint16_t \*frameGrid
+ Stores the frame coordinates for each tile, this basically tells it what each tile should appear as (Flag, 1, red mine, etc)

uint8_t \*logicGrid
+ Stored in an unsigned 8 bit variable (Format DBBX TTTT)
+ 4 rightmost bits (T bits) responsible for the tile's value (0 to 8 + mine = 10 combos = 4 bits)
+ Leftmost bit (D bit) determines if it has been "discovered" yet. initially 0, but when left/A clicked it changes to 1
+ The B bits are responsible for flagging/question marks. Left bit is true when flagged and right bit is true when question marked. They're never both true at the same time
+ The X bit is unused

uint8_t gridX && uint8_t gridY
+ This is how many tiles on the X and Y axes (Default 30, 20. Expert mode)

uint16_t gridStartX && uint16_t gridStartY
+ This is the top left coordinate to start drawing the grid from (Default 80, 80)

int cursorPos[8]
+ keeps track of the 4 player's cursor coordinates

int clickedCursorPos[8]
+ If player X is holding down A, elements 2 \* (X - 1) and (2 \* (X - 1)) + 1 (Assuming X starts at 1) will contain the cursor coordinates when the player first held down the A button. Used for making sure the player is still hovering over the right tile when releasing the A button. When A isn't held, these elements equal -1, -1 (Off screen)

int heldB[4]
+ Used to help with B button presses since MAPLE is designed for HOLD actions, not PRESS actions

Each tile is 16x16 texels
Total of 9 unique colours (Alot of colours are shared) (Not including icon, window, cursor, etc)
Total of 16 different tiles

{blank, blank pressed, unpressed/flagged mine, left clicked mine, wrongly flagged spot, flag, question mark, 1, 2, 3, 4, 5, 6, 7, 8, 9}

Record is saved depending on how many players are active
