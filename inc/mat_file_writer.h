#ifndef CURVEDETECT_MAT_FILE_WRITER_H
#define CURVEDETECT_MAT_FILE_WRITER_H

#include <cstdio>
#include <cstdint>

class MatFileWriter
{
public:
	static MatFileWriter* get(const char* path);

	MatFileWriter& matrix(const char *name, float *first, size_t rows, size_t cols = 1, bool bRowMajor = true);
	MatFileWriter& matrix(const char *name, double *first, size_t rows, size_t cols = 1, bool bRowMajor = true);

	void close();

private:
	FILE *outFile;

	void write_header();

	void write_data_element(uint32_t type, void *data, size_t dataItemSize, uint32_t nDataItems);
	void write_data_element_tag(uint32_t type, void *data, size_t dataItemSize, uint32_t nDataItems);
	void write_data_element_body(uint32_t type, void *data, size_t dataItemSize, uint32_t nDataItems);

};

#endif //CURVEDETECT_MAT_FILE_WRITER_H
