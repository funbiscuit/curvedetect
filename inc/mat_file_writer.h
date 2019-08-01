#ifndef CURVEDETECT_MAT_FILE_WRITER_H
#define CURVEDETECT_MAT_FILE_WRITER_H

#include <cstdio>
#include <cstdint>


/**
 * Example usage:
 *
    int32_t vals[]={1,2,3,4,5,6};
    float vals2[]={1.f,2.f,3.f,4.f,5.f,6.f};
    double vals3[]={1.0,2.0,3.0,4.0,5.0,6.0};

    auto writer = MatFileWriter::get("test.mat");	//path to mat file to be written

    if (writer)
    {
        writer->matrix("float_vec", vals2, 3, 2)
                .matrix("double_vec", vals3, 3, 2)
                .matrix("int32_vec", vals, 3, 2)
                .close();	//call to close is required to close opened file and flush all data!
    }
 */
class MatFileWriter
{
public:
	enum NumberType
	{
		FLOAT = 1,
		DOUBLE = 2,
		INT32 = 3
	};

	static MatFileWriter* get(const char* path);

	MatFileWriter& matrix(const char *name, int32_t *first, int rows, int cols = 1, bool bRowMajor = true);
	MatFileWriter& matrix(const char *name, float *first, int rows, int cols = 1, bool bRowMajor = true);
	MatFileWriter& matrix(const char *name, double *first, int rows, int cols = 1, bool bRowMajor = true);

	void close();

private:
	FILE *outFile;

	MatFileWriter& matrix(const char *name, void* first, NumberType dataType, int rows, int cols, bool bRowMajor);

	void write_header();

	void write_data_element(int type, void *data, int dataItemSize, int nDataItems);
	void write_data_element_tag(int type, int dataItemSize, int nDataItems);
	void write_data_element_body(int type, void *data, int dataItemSize, int nDataItems);

	void transpose(void* data, int dataItemSize, int rows, int cols);
};

#endif //CURVEDETECT_MAT_FILE_WRITER_H
