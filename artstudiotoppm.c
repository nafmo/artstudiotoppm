// artstudiotoppm, a program that converts Art Studio (C64, highres only)
// pictures to the portable pixmap (ppm) format
// Copyright © 1998 Peter Karlsson
//
// $Id$
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include <stdio.h>

// Struct defining Art Studio pictures
typedef struct
{
	unsigned char	loadaddress[2];		// highbit, lowbit
	unsigned char	image[8000];		// pixmap image
//	unsigned char	border;				// border colour (not supported here)
	unsigned char	colour[1000];		// colourmap
} artstudio_t;

// Struct used to define colour mapping
typedef struct
{
	int		r, g, b;			// Colourmap entry
} colour_t;

// RGB colour codes for the C64 colours. These are borrowed from Vice.
const colour_t c64colours[16] =
{
	{   0,   0,   0 },	// Black
	{ 255, 255, 255 },	// White
	{ 189,  24,  33 },	// Red
	{  49, 231, 198 },	// Cyan
	{ 181,  24, 231 },	// Purple
	{  24, 214,  24 },	// Green
	{  33,  24, 173 },	// Blue
	{ 222, 247,   8 },	// Yellow
	{ 189,  66,   0 },	// Orange
	{ 107,  49,   0 },	// Brown
	{ 255,  74,  82 },	// Light red
	{  66,  66,  66 },	// Gray 1
	{ 115, 115, 107 },	// Gray 2
	{  90, 255,  90 },	// Light green
	{  90,  82, 255 },	// Light blue
	{ 165, 165, 165 }	// Gray 3
};

// Function headers
int convert(const char *fname1, const char *fname2);

// main
int main(int argc, char *argv[])
{
	if (argc < 1 || argc > 3)
	{
		printf("Usage: %s [infile [outfile]]\n", argv[0]);
		return 1;
	}

	if (1 == argc)
		return convert(NULL, NULL);
	else if (2 == argc)
		return convert(argv[1], NULL);
	else if (3 == argc)
		return convert(argv[1], argv[2]);
	else
		return 1; // Shouldn't happen!
}

// Conversion function
int convert(const char *fname1, const char *fname2)
{
	FILE		*input, *output;
	artstudio_t	image;
	int			x, y, pixel, index, colourindex, mask, r, g, b;

	if (fname1)
	{
		// Open input file
		input = fopen(fname1, "rb");
		if (!input)	return 1;
	}
	else
		input = stdin;

	if (fname2)
	{
		// Open output file
		output = fopen(fname2, "wb");
		if (!output)	return 1;
	}
	else
		output = stdout;

	// Load the Art Studio image
	if (9002 != fread(&image, 1, sizeof(image), input))
	{
		fprintf(stderr, "Input ended prematurely\n");
		return 1;
	}
	
	// Close the file
	fclose(input);

	if (0 != image.loadaddress[0] || 0x20 != image.loadaddress[1])
	{
		fprintf(stderr, "Load address mismatch.\n");
	}
	
	// Create the PPM output

	// Header
	fputs("P6\n# Created by artstudiotoppm\n320 200 255\n", output);

	// Image
	for (y = 0; y < 200; y ++)
	{
		for (x = 0; x < 320; x ++)
		{
			// Get value of pixel at (x,y)
			index = (x / 8) * 8 + (y % 8) + (y / 8) * 320;
			colourindex = (x / 8) + (y / 8) * 40;
			pixel = (image.image[index] & (1 << (7 -(x % 8)))) >> (7 -(x % 8));

			// Retrieve RGB values
			switch (pixel)
			{
			case 0: // Colour 1
				r = c64colours[image.colour[colourindex] & 0xf].r;
				g = c64colours[image.colour[colourindex] & 0xf].g;
				b = c64colours[image.colour[colourindex] & 0xf].b;
				break;
				
			case 1: // Colour 2
				r = c64colours[image.colour[colourindex] >> 4].r;
				g = c64colours[image.colour[colourindex] >> 4].g;
				b = c64colours[image.colour[colourindex] >> 4].b;
				break;

			default:
				fprintf(stderr, "Internal error\n");
				break;
			};

			// Print to PPM file
			fputc(r, output);
			fputc(g, output);
			fputc(b, output);
		}
	}
	
	fclose(output);
}
