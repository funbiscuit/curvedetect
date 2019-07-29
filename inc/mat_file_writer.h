#ifndef CURVEDETECT_MAT_FILE_WRITER_H
#define CURVEDETECT_MAT_FILE_WRITER_H

#include <stdio.h>
#include <stdint.h>

enum miTYPE
{
	miINT8 = 1,
	miUINT8 = 2,
	miINT16 = 3,
	miUINT16 = 4,
	miINT32 = 5,
	miUINT32 = 6,
	miSINGLE = 7,
	miDOUBLE = 9,
	miINT64 = 12,
	miUINT64 = 13,
	miMATRIX = 14,
};

enum mxCLASS
{
	mxCHAR_CLASS = 4,
	mxDOUBLE_CLASS = 6,
	mxSINGLE_CLASS = 7,
	mxINT8_CLASS = 8,
	mxUINT8_CLASS = 9,
	mxINT16_CLASS = 10,
	mxUINT16_CLASS = 11,
	mxINT32_CLASS = 12,
	mxUINT32_CLASS = 13
};




void writeTestMatFile(FILE* fp);


void writeMatrixToMatFile(FILE* fp, const char* name, float* first, size_t rows, size_t cols = 1, bool bRowMajor = true);
void writeMatrixToMatFile(FILE* fp, const char* name, double* first, size_t rows, size_t cols = 1, bool bRowMajor = true);

void writeVectorToMatFile(FILE* fp, const char* name, float* first, size_t nItems);

void writeHeader(FILE* fp);

void writeDataElement(FILE* outfile, uint32_t type, void* data, size_t dataItemSize, uint32_t nDataItems);

void writeDataElementTag(FILE* outfile, uint32_t type, void* data, size_t dataItemSize, uint32_t nDataItems);
void writeDataElementBody(FILE* outfile, uint32_t type, void* data, size_t dataItemSize, uint32_t nDataItems);

#endif //CURVEDETECT_MAT_FILE_WRITER_H
