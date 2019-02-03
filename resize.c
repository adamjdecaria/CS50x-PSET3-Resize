/*
*
*   CS50x 2019
*   Problem Set 3 -> Resize.c
*
*   Resize an image
*
*   Adam DeCaria
*/

#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        fprintf(stderr, "Usage: resize n infile outfile\n");
        return 1;
    }

    // remember filenames
    char *infile = argv[2];
    char *outfile = argv[3];

    int resizeValue = atoi(argv[1]);

    if (resizeValue < 1 || resizeValue > 100)
    {
        printf("Please use a positive number less than 100.");
        return 1;
    }

    // open input file
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 2;
    }

    // open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 3;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }

    // create BITPMAPFILEHEADER and BITMAPINFOHEADER for resized file
    BITMAPFILEHEADER resizedBF;
    BITMAPINFOHEADER resizedBI;

    resizedBF = bf;
    resizedBI = bi;

    resizedBI.biHeight = bi.biHeight * resizeValue;
    resizedBI.biWidth = bi.biWidth * resizeValue;

    // determine padding for scanlines
    int padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    int resizedPadding = (4 - (resizedBI.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    resizedBI.biSizeImage = ((resizedBI.biWidth * sizeof(RGBTRIPLE)) + resizedPadding) * abs(resizedBI.biHeight);
    resizedBF.bfSize = resizedBI.biSizeImage + 54;

    // write outfile's BITMAPFILEHEADER
    fwrite(&resizedBF, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&resizedBI, sizeof(BITMAPINFOHEADER), 1, outptr);

    // iterate over infile's scanlines
    for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i++)
    {
        // iterate over pixels in scanline
        for (int l = 0; l < resizeValue; l++)
        {
            // copy each RGBTRIPLE and write to file "resize" times
            for (int j = 0; j < bi.biWidth; j++)
            {
                // temporary storage
                RGBTRIPLE triple;

                // read RGB triple from infile
                fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

                for (int m = 0; m < resizeValue; m++)
                {
                    fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
                }
            }

            // skip over padding, if any
            fseek(inptr, padding, SEEK_CUR);

            // then add it back (to demonstrate how)
            for (int k = 0; k < resizedPadding; k++)
            {
                fputc(0x00, outptr);
            }

            // move the cursor back to the start to continue copying/resizing
            if (l < resizeValue - 1)
            {
                long cursor = bi.biWidth * sizeof(RGBTRIPLE) + padding;
                fseek(inptr, -cursor, SEEK_CUR);
            }
        }
    }

    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // success
    return 0;
}
