#define HEADER_LEN 8
#include <png.h>
#include "error_exit.h"
#include "base64.h"

typedef struct {
	png_bytep buffer;
	png_uint_32 bufsize;
} MEMORY_WRITER_STATE;
// This function will be used for writing png data 
static void write_data_memory(png_structp png_ptr, png_bytep data, png_uint_32 length) {
	printf("Memory length: %d\n", length);
	MEMORY_WRITER_STATE * p = png_get_io_ptr(png_ptr);
	png_uint_32 nsize = p->bufsize + length;

	if (p->buffer)
		p->buffer = (png_bytep)realloc(p->buffer, nsize);
	else
		p->buffer = (png_bytep)malloc(nsize);

	if (!p->buffer)
		png_error(png_ptr, "WriteError");
	memcpy(p->buffer + p->bufsize, data, length);
	p->bufsize += length;
}

// png_set_write_fn also require user's flush function. If we set just NULL to 
// png_set_write_fn, it will process its default flush function. So, it will be
// better to just declare like this.
void user_png_flush(png_structp png_ptr) {
}

void png_convert_test() {
	char * pngFilePath = "A:\\DropBox\\Pictures\\14361406913471.png";
	FILE *fp;
	int fnStatus;
	fnStatus = fopen_s(&fp, pngFilePath, "rb");
	if (fnStatus != 0) errorExit("fopen_s");
	char header_buff[HEADER_LEN];
	fread_s(header_buff, HEADER_LEN, 1, HEADER_LEN, fp);
	int is_png = !png_sig_cmp(header_buff, 0, HEADER_LEN);
	if (!is_png) errorExit("not a png");
	printf("header_buff: %s\n", header_buff);
	printf("is_png: %d\n", is_png);


	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) errorExit("png_create_read_struct");

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return errorExit("png_create_info_struct");
	}
	png_infop end_info = png_create_info_struct(png_ptr);

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, HEADER_LEN);

	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	png_bytep row_pointers = png_get_rows(png_ptr, info_ptr);

	int width, height, bit_depth, color_type, interlace_type, compression_type, filter_method;

	png_get_IHDR(
		png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method
	);

	char * base64ImagePrefix = "data:image/png;base64,";

	printf("png image size read: %d:%d\n", width, height);



	/*
	char * linearBuffer;
	linearBuffer = malloc(sizeof(png_bytep) * height);
	int base64BufferSize;
	char * base64Buffer = base64_encode(row_pointers, )
	*/

	getchar();
}

MEMORY_WRITER_STATE bitmapToPng(int width, int height, int bit_depth, char* bmp_source) {
	// 1. Create png struct pointer
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		errorExit("png_create_write_struct");
	}
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		errorExit("png_destroy_write_struct");
	}

	// 2. Set png info like width, height, bit depth and color type
	png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	MEMORY_WRITER_STATE memory_writer_state;
	memory_writer_state.buffer = NULL;
	memory_writer_state.bufsize = 0;
	png_set_write_fn(png_ptr, &memory_writer_state, write_data_memory, user_png_flush);

	// 3. Convert 1d array to 2d array to be suitable for png struct
//    I assumed the original array is 1d
	png_bytepp row_pointers = png_malloc(png_ptr, sizeof(png_bytepp) * height);
	for (int i = 0; i < height; i++) {
		row_pointers[i] = png_malloc(png_ptr, width);
	}
	for (int hi = 0; hi < height; hi++) {
		for (int wi = 0; wi < width; wi++) {
			// bmp_source is source data that we convert to png
			row_pointers[hi][wi] = bmp_source[wi + width * hi];
		}
	}
	// 4. Write png file
	printf("png height: %d width: %d\n", height, width);
	printf("png_write_info\n");
	png_write_info(png_ptr, info_ptr);
	printf("png_write_image\n");
	png_write_image(png_ptr, row_pointers);
	printf("png_write_end\n");
	png_write_end(png_ptr, info_ptr);
	printf("png_destroy_write_struct\n");
	png_destroy_write_struct(&png_ptr, &info_ptr);
	return memory_writer_state;
}