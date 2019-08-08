
#include "mat_file_writer.h"
#include <cstring>
#include <cfloat>
#include <algorithm>


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
	mxUINT32_CLASS = 13,
	mxINT64_CLASS = 14,
	mxUINT64_CLASS = 15
};

MatFileWriter& MatFileWriter::matrix(const char *name, const float *first, int rows, int cols, bool bRowMajor)
{
	return matrix(name, first, FLOAT, rows, cols, bRowMajor);
}

MatFileWriter& MatFileWriter::matrix(const char *name, const double *first, int rows, int cols, bool bRowMajor)
{
	return matrix(name, first, DOUBLE, rows, cols, bRowMajor);
}

MatFileWriter& MatFileWriter::matrix(const char *name, const int8_t *first, int rows, int cols, bool bRowMajor)
{
	return matrix(name, first, INT8, rows, cols, bRowMajor);
}

MatFileWriter& MatFileWriter::matrix(const char *name, const uint8_t *first, int rows, int cols, bool bRowMajor)
{
	return matrix(name, first, UINT8, rows, cols, bRowMajor);
}

MatFileWriter& MatFileWriter::matrix(const char *name, const int16_t *first, int rows, int cols, bool bRowMajor)
{
	return matrix(name, first, INT16, rows, cols, bRowMajor);
}

MatFileWriter& MatFileWriter::matrix(const char *name, const uint16_t *first, int rows, int cols, bool bRowMajor)
{
	return matrix(name, first, UINT16, rows, cols, bRowMajor);
}

MatFileWriter& MatFileWriter::matrix(const char *name, const int32_t *first, int rows, int cols, bool bRowMajor)
{
	return matrix(name, first, INT32, rows, cols, bRowMajor);
}

MatFileWriter& MatFileWriter::matrix(const char *name, const uint32_t *first, int rows, int cols, bool bRowMajor)
{
	return matrix(name, first, UINT32, rows, cols, bRowMajor);
}

MatFileWriter& MatFileWriter::matrix(const char *name, const int64_t *first, int rows, int cols, bool bRowMajor)
{
	return matrix(name, first, INT64, rows, cols, bRowMajor);
}

MatFileWriter& MatFileWriter::matrix(const char *name, const uint64_t *first, int rows, int cols, bool bRowMajor)
{
	return matrix(name, first, UINT64, rows, cols, bRowMajor);
}

MatFileWriter& MatFileWriter::matrix(const char *name, const char *first, int rows, int cols, bool bRowMajor)
{
	return matrix(name, first, CHAR, rows, cols, bRowMajor);
}

MatFileWriter& MatFileWriter::matrix(const char *name, const void* first, NumberType dataType, int rows, int cols, bool bRowMajor)
{
	//write data element
	uint32_t data_type = miMATRIX;
	uint32_t bytes_num = 0;

	int32_t dims[2]={rows, cols}; //1xn - row; nx1 - column

	int dataItemSize=sizeof(uint8_t);
	int dataItemType=miUINT8;
	int dataItemClass=mxUINT8_CLASS;

	switch (dataType)
	{
		case FLOAT:
			dataItemSize=sizeof(float);
			dataItemType=miSINGLE;
			dataItemClass=mxSINGLE_CLASS;
			break;
		case DOUBLE:
			dataItemSize=sizeof(double);
			dataItemType=miDOUBLE;
			dataItemClass=mxDOUBLE_CLASS;
			break;

		case INT8:
			dataItemSize=sizeof(int8_t);
			dataItemType=miINT8;
			dataItemClass=mxINT8_CLASS;
			break;
		case UINT8:
			dataItemSize=sizeof(uint8_t);
			dataItemType=miUINT8;
			dataItemClass=mxUINT8_CLASS;
			break;

		case INT16:
			dataItemSize=sizeof(int16_t);
			dataItemType=miINT16;
			dataItemClass=mxINT16_CLASS;
			break;
		case UINT16:
			dataItemSize=sizeof(uint16_t);
			dataItemType=miUINT16;
			dataItemClass=mxUINT16_CLASS;
			break;

		case INT32:
			dataItemSize=sizeof(int32_t);
			dataItemType=miINT32;
			dataItemClass=mxINT32_CLASS;
			break;
		case UINT32:
			dataItemSize=sizeof(uint32_t);
			dataItemType=miUINT32;
			dataItemClass=mxUINT32_CLASS;
			break;

		case INT64:
			dataItemSize=sizeof(int64_t);
			dataItemType=miINT64;
			dataItemClass=mxINT64_CLASS;
			break;
		case UINT64:
			dataItemSize=sizeof(uint64_t);
			dataItemType=miUINT64;
			dataItemClass=mxUINT64_CLASS;
			break;

		case CHAR:
			dataItemSize=sizeof(int8_t);
			//data type will be set to miUINT16 automatically later
			//because char array is written as array of uint16
			dataItemType=miUINT16;
			dataItemClass=mxCHAR_CLASS;
			break;
	}

	const void* data=first;
	uint8_t* transposed=nullptr;

	//since matlab expects matrices in column major order, we need to transpose it first
	if (bRowMajor)
		transposed=transpose(first, dataItemSize, rows, cols);
	if(transposed)
		data=transposed;

	uint32_t name_size = strlen(name) * sizeof(char);

	//add padding bytes
	name_size += get_padding(name_size);

	auto dataCount = static_cast<uint32_t>(rows * cols);
	uint32_t dataTotalSize = dataCount * dataItemSize;
	//chars will be written as uint16, so size is twice as big
	if(dataType==CHAR)
		dataTotalSize <<= 1;

	dataTotalSize += get_padding(dataTotalSize);

	bytes_num = 16 //array flags subelement
				+ 16 //dimensions subelement
				+ name_size + 8 //name subelement (8 for tag)
				+ dataTotalSize + 8; //data subelement (8 for tag)


	fwrite(&data_type, sizeof(uint32_t), 1, outFile);
	fwrite(&bytes_num, sizeof(uint32_t), 1, outFile);


	//write subelements

	//write array flags block (8 bytes)
	data_type = miUINT32;
	bytes_num = 8;
	fwrite(&data_type, sizeof(uint32_t), 1, outFile);
	fwrite(&bytes_num, sizeof(uint32_t), 1, outFile);

	uint32_t flags = 0x00;
	flags |= dataItemClass;//we use only data class flag
	fwrite(&flags, sizeof(uint32_t), 1, outFile);
	//write 4 bytes of undefined (0 in this case) data to array flags block
	for(int i=0;i<4;++i)
		putc(0x00, outFile);

	//write dimensions
	write_data_element(miINT32, dims, sizeof(int32_t), 2, false);

	//write array name and data
	write_data_element(miINT8, (void *) name, sizeof(char), static_cast<uint32_t>(strlen(name)), false);
	write_data_element(dataItemType, data, dataItemSize, dataCount, dataType==CHAR);

	//delete transposed data
	if (transposed)
		delete[](transposed);

	return *this;
}

void MatFileWriter::test_writer(const char* path)
{
	//by default most libraries store data in row major format, so matrix
	//[0 1 2
	// 3 4 5
	// 6 7 8]
	//will be stored in array
	//[0 1 2 3 4 5 6 7 8]
	//the same matrix in column major format will be stored as
	//[0 3 6 1 4 7 2 5 8]
	//when you call writer->matrix() you need to send false to last parameter if
	//your matrix is stored in column major. By default it will be row major format
	//all test matrices will have 4 rows and 2 columns
	char chars[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};

	int8_t vals8i[] = {1, 2, INT8_MIN, 4, 5, 6, INT8_MAX, 8};
	uint8_t vals8ui[] = {1, 2, 3, 4, 5, 6, UINT8_MAX, 8};

	int16_t vals16i[] = {1, 2, INT16_MIN, 4, 5, 6, INT16_MAX, 8};
	uint16_t vals16ui[] = {1, 2, 3, 4, 5, 6, UINT16_MAX, 8};

	int32_t vals32i[] = {1, 2, INT32_MIN, 4, 5, 6, INT32_MAX, 8};
	uint32_t vals32ui[] = {1, 2, 3, 4, 5, 6, UINT32_MAX, 8};

	int64_t vals64i[] = {1, 2, INT64_MIN, 4, 5, 6, INT64_MAX, 8};
	uint64_t vals64ui[] = {1, 2, 3, 4, 5, 6, UINT64_MAX, 8};

	float valsf[] = {1.f, 0.f, FLT_MIN, -FLT_MIN, 5.f, 6.f, FLT_MAX, -FLT_MAX};
	double valsd[] = {1.0, 0.0, DBL_MIN, -DBL_MIN, 5.0, 6.0, DBL_MAX, -DBL_MAX};

	//also test the same input in column major format
	char chars_cm[] = {'a', 'c', 'e', 'g', 'b', 'd', 'f', 'h'};

	int8_t vals8i_cm[] = {1, INT8_MIN, 5, INT8_MAX, 2, 4, 6, 8};
	uint8_t vals8ui_cm[] = {1, 3, 5, UINT8_MAX, 2, 4, 6, 8};

	int16_t vals16i_cm[] = {1, INT16_MIN, 5, INT16_MAX, 2, 4, 6, 8};
	uint16_t vals16ui_cm[] = {1, 3, 5, UINT16_MAX, 2, 4, 6, 8};

	int32_t vals32i_cm[] = {1, INT32_MIN, 5, INT32_MAX, 2, 4, 6, 8};
	uint32_t vals32ui_cm[] = {1, 3, 5, UINT32_MAX, 2, 4, 6, 8};

	int64_t vals64i_cm[] = {1, INT64_MIN, 5, INT64_MAX, 2, 4, 6, 8};
	uint64_t vals64ui_cm[] = {1, 3, 5, UINT64_MAX, 2, 4, 6, 8};

	float valsf_cm[] = {1.f, FLT_MIN, 5.f, FLT_MAX, 2.f, -FLT_MIN, 6.f, -FLT_MAX};
	double valsd_cm[] = {1.0, DBL_MIN, 5.0, DBL_MAX, 2.0, -DBL_MIN, 6.0, -DBL_MAX};

	auto writer = MatFileWriter::get(path);

	if (writer)
	{
		//write row major matrices (don't need the last argument)
		writer->matrix("chars", chars, 4, 2)
				.matrix("vals8i", vals8i, 4, 2)
				.matrix("vals8ui", vals8ui, 4, 2)
				.matrix("vals16i", vals16i, 4, 2)
				.matrix("vals16ui", vals16ui, 4, 2)
				.matrix("vals32i", vals32i, 4, 2)
				.matrix("vals32ui", vals32ui, 4, 2)
				.matrix("vals64i", vals64i, 4, 2)
				.matrix("vals64ui", vals64ui, 4, 2)
				.matrix("valsf", valsf, 4, 2)
				.matrix("valsd", valsd, 4, 2)
				.matrix("literal", "Literal can also be written!", 4, 7)
				.matrix("literal2", "Literal can also be written!", 7, 4)
						//write column major matrices
				.matrix("chars_cm", chars_cm, 4, 2, false)
				.matrix("vals8i_cm", vals8i_cm, 4, 2, false)
				.matrix("vals8ui_cm", vals8ui_cm, 4, 2, false)
				.matrix("vals16i_cm", vals16i_cm, 4, 2, false)
				.matrix("vals16ui_cm", vals16ui_cm, 4, 2, false)
				.matrix("vals32i_cm", vals32i_cm, 4, 2, false)
				.matrix("vals32ui_cm", vals32ui_cm, 4, 2, false)
				.matrix("vals64i_cm", vals64i_cm, 4, 2, false)
				.matrix("vals64ui_cm", vals64ui_cm, 4, 2, false)
				.matrix("valsf_cm", valsf_cm, 4, 2, false)
				.matrix("valsd_cm", valsd_cm, 4, 2, false)
				.matrix("literal_cm", "Literal can also be written!", 4, 7, false)
				.matrix("literal2_cm", "Literal can also be written!", 7, 4, false)
				.close();
	}
}

void MatFileWriter::write_header()
{
	//write header (128 bytes)

	const size_t maxTextLen = 124;	//matlab uses 124 bytes for header text and 4 bytes for version+endian

	const char head[] = "Version 5 MAT-file, created by Curve Detect";
	size_t text_len = strlen(head);

	//write text in header (124 bytes max)
	fwrite(head, sizeof(char), std::min(maxTextLen, text_len), outFile);
	for (size_t k = text_len; k<maxTextLen; k++)
		putc(' ', outFile);

	uint16_t version = 0x0100;
	uint16_t endian = 'M';
	endian <<= 8;
	endian |= 'I';

	//(version+endian 4 bytes)
	fwrite(&version, sizeof(uint16_t), 1, outFile);
	fwrite(&endian, sizeof(uint16_t), 1, outFile);
}

void MatFileWriter::write_data_element(int type, const void* data, int dataItemSize, int nDataItems, bool charClass)
{
	//write data element tag (8 bytes)

	//calculate the number of data bytes:
	uint32_t N = dataItemSize * nDataItems;
	if (charClass)
	{
		//mxCHAR_CLASS data is 8bit, but is written as 16bit uints
		type = miUINT16;
		N <<= 1;
	}

	//write the datatype identifier
	fwrite(&type, sizeof(uint32_t), 1, outFile);
	//write the number of data bytes to the data element's tag:
	fwrite(&N, sizeof(uint32_t), 1, outFile);

	//write data element body (N bytes)
	// write data
	if (charClass)
	{
		//we have to write one char at a time,
		//each followed by a null byte
		auto *chars = reinterpret_cast<const char*>(data);

		for (int i = 0; i<nDataItems; ++i)
		{
			putc(*chars, outFile);
			++chars;
			putc(0x00, outFile);
		}
	}
	else
		fwrite(data, dataItemSize, nDataItems, outFile);

	/*
	* padding is required to ensure 64bit boundaries between
	* data elements.
	*/
	uint32_t paddingBytes = get_padding(N);

	for (int i = 0; i<paddingBytes; i++)
		putc(0x00, outFile);
}

MatFileWriter* MatFileWriter::get(const char* path)
{
	FILE* fp=fopen(path, "wb");
	MatFileWriter* writer = nullptr;
	if(path)
	{
		writer = new MatFileWriter();
		writer->outFile = fp;
		writer->write_header();
	}
	return writer;
}

void MatFileWriter::close()
{
	fclose(outFile);
	delete(this);
}

uint8_t* MatFileWriter::transpose(const void *data, int dataItemSize, int rows, int cols)
{
	//transposed result will be the same
	if(rows==1 || cols==1)
		return nullptr;

	auto* tr = new uint8_t[rows*cols*dataItemSize];
	auto* src = (uint8_t*) data;

	for(int i=0;i<cols;++i)
		for(int j=0;j<rows;++j)
			memcpy(&tr[(i*rows+j)*dataItemSize], &src[(j*cols+i)*dataItemSize], (size_t)dataItemSize);

	return tr;
}

uint32_t MatFileWriter::get_padding(uint32_t size)
{
	//short way to write (8 - (size % 8)) % 8;
	return uint32_t(-size%8);
}
