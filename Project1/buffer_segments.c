#include "buffer_segments.h"

buffer_segments readBufferSegments(char * buffer, int bufferSize) {
	buffer_segments buff_segs;
	buff_segs.len = 0;
	buff_segs.size = 0;
	buff_segs.list = malloc(0);

	buffer_segment curr_buff_seg;
	curr_buff_seg.buffer = &buffer[0];
	curr_buff_seg.bufferSize = 0;
	short isNullChar;
	for (int ib = 0; ib < bufferSize; ib++) {
		curr_buff_seg.bufferSize++;
		isNullChar = buffer[ib] == '\0';
		if (isNullChar) {
			buffer_segment buff_seg;
			buff_seg.buffer = curr_buff_seg.buffer;
			buff_seg.bufferSize = curr_buff_seg.bufferSize;

			buff_segs.size += buff_seg.bufferSize;
			buff_segs.list = realloc(buff_segs.list, buff_segs.size * sizeof(buffer_segment));
			buff_segs.list[buff_segs.len] = buff_seg;
			buff_segs.len++;

			curr_buff_seg.buffer = &buffer[curr_buff_seg.bufferSize];
			curr_buff_seg.bufferSize = 0;
		}
	}
	return buff_segs;
}