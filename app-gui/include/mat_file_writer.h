#ifndef CURVEDETECT_MAT_FILE_WRITER_H
#define CURVEDETECT_MAT_FILE_WRITER_H

#include <cstdio>
#include <cstdint>


/**
 * MatFileWriter creates Matlab file (version 5) with matrices/vectors (only 2d) of all numeric types
 * For example usage open MatFileWriter::test_writer(path)
 */
class MatFileWriter
{
	/**
     * should use get() instead of manual creation
     */
	MatFileWriter(const MatFileWriter&);
	MatFileWriter& operator=(MatFileWriter&);
	MatFileWriter() = default;
	/**
     * should call to close() instead of deleting directly
     */
	~MatFileWriter() = default;
public:
	enum NumberType
	{
		FLOAT = 1,
		DOUBLE = 2,
		INT8 = 3,
		UINT8 = 4,
		INT16 = 5,
		UINT16 = 6,
		INT32 = 7,
		UINT32 = 8,
		INT64 = 9,
		UINT64 = 10,
		CHAR =11
	};

	static MatFileWriter* get(const char* path);

	/**
	 * Create a test mat file with matrices of all supported data types
	 * @param path - path to file
	 */
	static void test_writer(const char* path);

	MatFileWriter& matrix(const char *name, const float *first, int rows, int cols = 1, bool bRowMajor = true);
	MatFileWriter& matrix(const char *name, const double *first, int rows, int cols = 1, bool bRowMajor = true);

	MatFileWriter& matrix(const char *name, const int8_t *first, int rows, int cols = 1, bool bRowMajor = true);
	MatFileWriter& matrix(const char *name, const uint8_t *first, int rows, int cols = 1, bool bRowMajor = true);

	MatFileWriter& matrix(const char *name, const int16_t *first, int rows, int cols = 1, bool bRowMajor = true);
	MatFileWriter& matrix(const char *name, const uint16_t *first, int rows, int cols = 1, bool bRowMajor = true);

	MatFileWriter& matrix(const char *name, const int32_t *first, int rows, int cols = 1, bool bRowMajor = true);
	MatFileWriter& matrix(const char *name, const uint32_t *first, int rows, int cols = 1, bool bRowMajor = true);

	MatFileWriter& matrix(const char *name, const int64_t *first, int rows, int cols = 1, bool bRowMajor = true);
	MatFileWriter& matrix(const char *name, const uint64_t *first, int rows, int cols = 1, bool bRowMajor = true);

	MatFileWriter& matrix(const char *name, const char *first, int rows, int cols = 1, bool bRowMajor = true);

	void close();

private:
	FILE *outFile;

	MatFileWriter& matrix(const char *name, const void* first, NumberType dataType, int rows, int cols, bool bRowMajor);

	void write_header();

	void write_data_element(int type, const void *data, int dataItemSize, int nDataItems, bool charClass);

	uint8_t* transpose(const void* data, int dataItemSize, int rows, int cols);
	static inline uint32_t get_padding(uint32_t size);
};

#endif //CURVEDETECT_MAT_FILE_WRITER_H
