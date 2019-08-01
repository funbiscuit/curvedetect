
#include "mat_file_writer.h"

#include <iostream>

#include <vector>

#include <imgui.h>

void write_test_file(FILE *fp)
{
	write_header(fp);

	std::vector<float> vect;

	vect.push_back(134.65f);
	vect.push_back(254.5f);
	vect.push_back(356.85f);
	vect.push_back(432.689f);
	vect.push_back(567.7f);
	vect.push_back(673.25f);

	std::vector<ImVec2> vect2;

	vect2.push_back(ImVec2(12.0f, 23.0f));
	vect2.push_back(ImVec2(35.0f, 42.0f));
	vect2.push_back(ImVec2(51.0f, 67.0f));

	//writeVectorToMatFile(fp, "TempArr34", &vect[0], vect.size());
	//write_matrix_to_file(fp, "TempArr34", &vect[0], vect.size()/2, 2, true);
	write_matrix_to_file(fp, "TempArr34", &(vect2[0].x), vect2.size(), 2);
}

void write_matrix_to_file(FILE *fp, const char *name, float *first, size_t rows, size_t cols /*= 1*/,
						  bool bRowMajor /*= true*/)
{
	//write data element
	uint32_t data_type = miMATRIX;
	uint32_t bytes_num = 0;

	//char ArrayName[] = "TempArr";
	//float data[4] = { 324.65f, 213.685f, 324.65f, 213.685f };

	int32_t dims[2]; //1xn - row; nx1 - column

	if (bRowMajor)
	{
		//dims[0] = cols;
		//dims[1] = rows;
	}
	//else
	{
		dims[0] = rows;
		dims[1] = cols;
	}


	uint32_t name_size = strlen(name) * sizeof(char);

	if (name_size % 8 > 0)
		name_size += 8 - name_size % 8;

	uint32_t data_size = rows * cols * sizeof(float);

	if (data_size % 8 > 0)
		data_size += 8 - data_size % 8;

	bytes_num = 16 //array flags subelement
		+ 16 //dimensions subelement
		+ name_size + 8 //name subelement
		+ data_size + 8; //data subelement




	fwrite(&data_type, sizeof(uint32_t), 1, fp);
	fwrite(&bytes_num, sizeof(uint32_t), 1, fp);


	//write subelements

	//write array flags block (8 bytes)
	data_type = miUINT32;
	bytes_num = 8;
	fwrite(&data_type, sizeof(uint32_t), 1, fp);
	fwrite(&bytes_num, sizeof(uint32_t), 1, fp);

	uint32_t flags = 0x00;
	//flags |= 0x04; //global
	flags <<= 8;  //left shift the flags by one byte
				  //flags |= mxUINT32_CLASS;
	flags |= mxSINGLE_CLASS;
	fwrite(&flags, sizeof(uint32_t), 1, fp);
	//write 4 bytes of undefined data to array flags block
	uint32_t TempUInt32 = 0x00;
	fwrite(&TempUInt32, sizeof(uint32_t), 1, fp);

	//write dimenstions

	write_data_element(fp, miINT32, dims, sizeof(int32_t), 2);

	//write array name

	write_data_element(fp, miINT8, (void *) name, sizeof(char), static_cast<uint32_t>(strlen(name)));


	//write array data
	write_data_element_tag(fp, miSINGLE, first, sizeof(float), static_cast<uint32_t>(rows * cols));
	if (!bRowMajor)
	{
		write_data_element_body(fp, miSINGLE, first, sizeof(float), static_cast<uint32_t>(rows * cols));
	}
	else
	{
		uint32_t    nBytes;
		uint32_t    paddingBytes = 0;
		uintptr_t   i;
		uint8_t     emptyChar = 0x00;

		//calculate the number of data bytes:
		nBytes = sizeof(float) * rows*cols;

		// write the data
		for (size_t col = 0; col < cols; col++)
		{
			for (size_t row = 0; row < rows; row++)
			{
				size_t offset = col + row * cols;
				fwrite(first + offset, sizeof(float), 1, fp);

			}
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
			fwrite(&emptyChar, sizeof(uint8_t), 1, fp);
		}
	}



}


void write_matrix_to_file(FILE *fp, const char *name, double *first, size_t rows, size_t cols /*= 1*/,
						  bool bRowMajor /*= true*/)
{
	//TODO combine matrix of different types in one function (a lot of repeating code)

	//write data element
	uint32_t data_type = miMATRIX;
	uint32_t bytes_num = 0;

	//char ArrayName[] = "TempArr";
	//float data[4] = { 324.65f, 213.685f, 324.65f, 213.685f };

	int32_t dims[2]; //1xn - row; nx1 - column

	if (bRowMajor)
	{
		//dims[0] = cols;
		//dims[1] = rows;
	}
	//else
	{
		dims[0] = rows;
		dims[1] = cols;
	}


	uint32_t name_size = strlen(name) * sizeof(char);

	if (name_size % 8 > 0)
		name_size += 8 - name_size % 8;

	uint32_t data_size = rows * cols * sizeof(double);

	if (data_size % 8 > 0)
		data_size += 8 - data_size % 8;

	bytes_num = 16 //array flags subelement
				+ 16 //dimensions subelement
				+ name_size + 8 //name subelement
				+ data_size + 8; //data subelement


	fwrite(&data_type, sizeof(uint32_t), 1, fp);
	fwrite(&bytes_num, sizeof(uint32_t), 1, fp);


	//write subelements

	//write array flags block (8 bytes)
	data_type = miUINT32;
	bytes_num = 8;
	fwrite(&data_type, sizeof(uint32_t), 1, fp);
	fwrite(&bytes_num, sizeof(uint32_t), 1, fp);

	uint32_t flags = 0x00;
	flags <<= 8;  //left shift the flags by one byte
	flags |= mxDOUBLE_CLASS;
	fwrite(&flags, sizeof(uint32_t), 1, fp);
	//write 4 bytes of undefined data to array flags block
	uint32_t TempUInt32 = 0x00;
	fwrite(&TempUInt32, sizeof(uint32_t), 1, fp);

	//write dimenstions
	write_data_element(fp, miINT32, dims, sizeof(int32_t), 2);

	//write array name
	write_data_element(fp, miINT8, (void *) name, sizeof(char), static_cast<uint32_t>(strlen(name)));


	//write array data
	write_data_element_tag(fp, miDOUBLE, first, sizeof(double), static_cast<uint32_t>(rows * cols));
	if (!bRowMajor)
	{
		write_data_element_body(fp, miDOUBLE, first, sizeof(double), static_cast<uint32_t>(rows * cols));
	}
	else
	{
		uint32_t    nBytes;
		uint32_t    paddingBytes = 0;
		uintptr_t   i;
		uint8_t     emptyChar = 0x00;

		//calculate the number of data bytes:
		nBytes = sizeof(double) * rows*cols;

		// write the data
		for (size_t col = 0; col < cols; col++)
		{
			for (size_t row = 0; row < rows; row++)
			{
				size_t offset = col + row * cols;
				fwrite(first + offset, sizeof(double), 1, fp);

			}
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
			fwrite(&emptyChar, sizeof(uint8_t), 1, fp);
		}
	}



}

void write_vector_to_file(FILE *fp, const char *name, float *first, size_t nItems)
{

	write_matrix_to_file(fp, name, first, nItems, 1);

}

void write_header(FILE *fp)
{

	const size_t HeaderSize = 116;	//matlab uses 116 bytes for header text
	const size_t SubsysSize = 8;	//and 8 bytes for sybsystem data offset

	char head[] = "File was created by Curve Digitizer v1.0";
	char subsysdata[SubsysSize];

	size_t text_len = strlen(head);

	char empty_char = ' ';

	//write header (128 bytes)

	//write text in header (116 bytes)
	for (size_t k = 0; k<HeaderSize; k++)
	{
		if (k < text_len)
		{
			fwrite(&head[k], sizeof(char), 1, fp);
		}
		else
		{
			fwrite(&empty_char, sizeof(char), 1, fp);
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
	fwrite(subsysdata, sizeof(char), SubsysSize, fp);
	//(version+endian 4 bytes)
	fwrite(&version, sizeof(uint16_t), 1, fp);
	fwrite(&endian, sizeof(uint16_t), 1, fp);

}

void write_data_element(FILE *outfile, uint32_t type, void *data, size_t dataItemSize, uint32_t nDataItems)
{
	write_data_element_tag(outfile, type, data, dataItemSize, nDataItems);
	write_data_element_body(outfile, type, data, dataItemSize, nDataItems);
}

void write_data_element_tag(FILE *outfile, uint32_t type, void *data, size_t dataItemSize, uint32_t nDataItems)
{
	uint32_t    nBytes;
	uint32_t    paddingBytes = 0;
	uint8_t     emptyChar = 0x00;


	//calculate the number of data bytes:
	nBytes = dataItemSize * nDataItems;
	if (type == mxCHAR_CLASS)
	{
		//mxCHAR_CLASS data is 8bit, but is written as 16bit uints
		type = miUINT16;
		nBytes *= 2;
		//write the datatype identifier
		fwrite(&type, sizeof(uint32_t), 1, outfile);
		type = mxCHAR_CLASS;
	}
	else {
		//write the datatype identifier
		fwrite(&type, sizeof(uint32_t), 1, outfile);
	}


	//write the number of data bytes to the data element's tag:
	fwrite(&nBytes, sizeof(uint32_t), 1, outfile);

}

void write_data_element_body(FILE *outfile, uint32_t type, void *data, size_t dataItemSize, uint32_t nDataItems)
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
			fwrite(tempPtr, sizeof(uint8_t), 1, outfile);
			tempPtr += 1;
			fwrite(&tempUInt8, sizeof(uint8_t), 1, outfile);
		}
	}
	else
	{
		fwrite(data, dataItemSize, nDataItems, outfile);
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
		fwrite(&emptyChar, sizeof(uint8_t), 1, outfile);
	}
}
