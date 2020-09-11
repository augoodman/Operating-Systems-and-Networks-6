/**
* A utility functions for doing pixel transformations.
*
* Completion time: 1 minutes
*
* @author Vatrcia Edgar
* @version 1.1
* (typedef added by Goodman)
*/

#ifndef PixelProcessor_H
#define PixelProcessor_H 1
typedef struct Pixel{
	unsigned char red;
	unsigned char green;
	unsigned char blue;
}Pixel;

//NOT NEEDED FOR THREADING HW.
void colorShiftPixels(struct Pixel** pArr, int width, int height, int rShift, int gShift, int bShift);
#endif