/*
 * The systems color represented by "number" is set using the color component
 * intensities found in the "red", "grn", and "blu" variables.  A value of
 * 0 represents 0 intensity; a value of 255 represents 100% intensity.
 */
#include "cell.h"

int reset_color (int number, int red, int grn, int blu)
{
    Color_table[number][0] = red;
    Color_table[number][1] = grn;
    Color_table[number][2] = blu;

    return 0;
}
