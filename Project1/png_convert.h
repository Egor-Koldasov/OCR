#include <png.h>

void png_convert_test();
typedef struct {
	png_bytep buffer;
	png_uint_32 bufsize;
} MEMORY_WRITER_STATE;
MEMORY_WRITER_STATE bitmapToPng(int width, int height, int bit_depth, char * bmp_source);
