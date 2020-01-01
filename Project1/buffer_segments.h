typedef struct {
	char * buffer;
	int bufferSize;
} buffer_segment;
typedef struct {
	buffer_segment * list;
	int len;
	int size;
} buffer_segments;

buffer_segments readBufferSegments(char * buffer, int bufferSize);
