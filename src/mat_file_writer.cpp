
#include "mat_file_writer.h"
#include <cstring>
#include <iostream>


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

MatFileWriter& MatFileWriter::matrix(const char *name, int32_t *first, int rows, int cols, bool bRowMajor)
{
    return matrix(name, first, INT32, rows, cols, bRowMajor);
}

MatFileWriter& MatFileWriter::matrix(const char *name, float *first, int rows, int cols, bool bRowMajor)
{
	return matrix(name, first, FLOAT, rows, cols, bRowMajor);
}

MatFileWriter& MatFileWriter::matrix(const char *name, double *first, int rows, int cols, bool bRowMajor)
{
	return matrix(name, first, DOUBLE, rows, cols, bRowMajor);
}

MatFileWriter& MatFileWriter::matrix(const char *name, void* first, NumberType dataType, int rows, int cols, bool bRowMajor)
{
	//write data element
	uint32_t data_type = miMATRIX;
	uint32_t bytes_num = 0;

	int32_t dims[2]={rows, cols}; //1xn - row; nx1 - column

	int dataItemSize;
	int dataItemClass;
	int dataItemType;

	switch (dataType)
	{
        case INT32:
            dataItemSize=sizeof(int32_t);
            dataItemType=miINT32;
            dataItemClass=mxINT32_CLASS;
            break;
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
		default:
			dataItemSize=sizeof(uint8_t);
			dataItemType=miINT8;
			dataItemClass=mxINT8_CLASS;
			break;
	}

	//since matlab stores matrices in column major order, we need to transpose it first
	if (bRowMajor)
		transpose(first, dataItemSize, rows, cols);


	uint32_t name_size = strlen(name) * sizeof(char);

	if (name_size % 8 > 0)
		name_size += 8 - name_size % 8;

	auto dataCount = static_cast<uint32_t>(rows * cols);
	uint32_t dataTotalSize = dataCount * dataItemSize;

	if (dataTotalSize % 8 > 0)
		dataTotalSize += 8 - dataTotalSize % 8;

	bytes_num = 16 //array flags subelement
				+ 16 //dimensions subelement
				+ name_size + 8 //name subelement
				+ dataTotalSize + 8; //data subelement


	fwrite(&data_type, sizeof(uint32_t), 1, outFile);
	fwrite(&bytes_num, sizeof(uint32_t), 1, outFile);


	//write subelements

	//write array flags block (8 bytes)
	data_type = miUINT32;
	bytes_num = 8;
	fwrite(&data_type, sizeof(uint32_t), 1, outFile);
	fwrite(&bytes_num, sizeof(uint32_t), 1, outFile);

	uint32_t flags = 0x00;
	flags <<= 8;  //left shift the flags by one byte
	flags |= dataItemClass;
	fwrite(&flags, sizeof(uint32_t), 1, outFile);
	//write 4 bytes of undefined data to array flags block
	uint32_t TempUInt32 = 0x00;
	fwrite(&TempUInt32, sizeof(uint32_t), 1, outFile);

	//write dimenstions
	write_data_element(miINT32, dims, sizeof(int32_t), 2);

	//write array name and data
	write_data_element(miINT8, (void *) name, sizeof(char), static_cast<uint32_t>(strlen(name)));
	write_data_element(dataItemType, first, dataItemSize, dataCount);

	//data we received might be needed outside, so transpose it back
	if (bRowMajor)
		transpose(first, dataItemSize, cols, rows);

	return *this;
}

void MatFileWriter::write_header()
{

	const size_t HeaderSize = 116;	//matlab uses 116 bytes for header text
	const size_t SubsysSize = 8;	//and 8 bytes for sybsystem data offset

	char head[] = "File was created by Curve Detect";
	char subsysdata[SubsysSize];

	size_t text_len = strlen(head);

	char empty_char = ' ';

	//write header (128 bytes)

	//write text in header (116 bytes)
	for (size_t k = 0; k<HeaderSize; k++)
	{
		if (k < text_len)
		{
			fwrite(&head[k], sizeof(char), 1, outFile);
		}
		else
		{
			fwrite(&empty_char, sizeof(char), 1, outFile);
		}
	}


	for (size_t k = 0; k<SubsysSize; k++)
	{
		subsysdata[k] = 0;
	}

	uint16_t version = 0x0100;
	uint16_t endian = 'M';
	endian <<= 8;
	endian |= 'I';

	//(text+subsystem data offset 124 bytes)
	fwrite(subsysdata, sizeof(char), SubsysSize, outFile);
	//(version+endian 4 bytes)
	fwrite(&version, sizeof(uint16_t), 1, outFile);
	fwrite(&endian, sizeof(uint16_t), 1, outFile);

}

void MatFileWriter::write_data_element(int type, void *data, int dataItemSize, int nDataItems)
{
	write_data_element_tag(type, dataItemSize, nDataItems);
	write_data_element_body(type, data, dataItemSize, nDataItems);
}

void MatFileWriter::write_data_element_tag(int type, int dataItemSize, int nDataItems)
{
	//calculate the number of data bytes:
	uint32_t nBytes = dataItemSize * nDataItems;
	if (type == mxCHAR_CLASS)
	{
		//mxCHAR_CLASS data is 8bit, but is written as 16bit uints
		type = miUINT16;
		nBytes *= 2;
		//write the datatype identifier
		fwrite(&type, sizeof(uint32_t), 1, outFile);
		type = mxCHAR_CLASS;
	}
	else {
		//write the datatype identifier
		fwrite(&type, sizeof(uint32_t), 1, outFile);
	}


	//write the number of data bytes to the data element's tag:
	fwrite(&nBytes, sizeof(uint32_t), 1, outFile);

}

void MatFileWriter::write_data_element_body(int type, void *data, int dataItemSize, int nDataItems)
{
	uint8_t     tempUInt8;
	uint32_t    nBytes;
	uint32_t    paddingBytes = 0;
	uintptr_t   i;
	uint8_t     emptyChar = 0x00;

	//calculate the number of data bytes:
	nBytes = dataItemSize * nDataItems;
	if (type == mxCHAR_CLASS)
	{
		//mxCHAR_CLASS data is 8bit, but is written as 16bit uints
		nBytes *= 2;
	}

	// write the data
	if (type == mxCHAR_CLASS)
	{
		//we have to write one char
		//at a time, each followed by a null byte
		uint8_t     *tempPtr = reinterpret_cast<uint8_t*>(data);
		char     *charPtr = reinterpret_cast<char*>(data);
		tempUInt8 = 0x00;

		for (uintptr_t iChar = 0; iChar<strlen(charPtr); iChar++)
		{
			fwrite(tempPtr, sizeof(uint8_t), 1, outFile);
			tempPtr += 1;
			fwrite(&tempUInt8, sizeof(uint8_t), 1, outFile);
		}
	}
	else
	{
		fwrite(data, dataItemSize, nDataItems, outFile);
	}

	/*
	* padding may be required to ensure 64bit boundaries between
	* data elements.
	*/
	if (nBytes % 8 > 0) {
		paddingBytes = 8 - nBytes % 8;   //This could probably be neatly rewritten with the ternary operator
	}

	for (i = 0; i<paddingBytes; i++)
	{
		fwrite(&emptyChar, sizeof(uint8_t), 1, outFile);
	}
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

void MatFileWriter::transpose(void *data, int dataItemSize, int rows, int cols)
{
	//transposed result will be the same
	if(rows==1 || cols==1)
		return;

	auto* temp = new uint8_t[rows*cols*dataItemSize];

	auto* src = (uint8_t*) data;

	for(int i=0;i<cols;++i)
		for(int j=0;j<rows;++j)
			memcpy(&temp[(i*rows+j)*dataItemSize], &src[(j*cols+i)*dataItemSize], (size_t)dataItemSize);

	memcpy(data, temp, (size_t)rows*cols*dataItemSize);

	delete[](temp);
}
