#include "zoid.h"

#define L1_SIZE	(32 * 1024)


char l1flusher[L1_SIZE] __attribute__ ((aligned(32)));


void flush_L1_all()
{
    char *p;

    for (p = l1flusher; p < l1flusher + L1_SIZE; p += L1_CACHE_LINE_SIZE)
	asm volatile("lwz 0,0(%0)\n" :: "r" (p) : "0");

    asm volatile("sync\n" ::: "memory");
}


void flush_L1_region(void *addr, unsigned int size)
{
    if (size < L1_SIZE) {
	char *p;

	for (p = addr; p < (char *) addr + size; p += L1_CACHE_LINE_SIZE)
	    asm volatile("dcbf 0,%0\n" :: "r" (p));

	asm volatile("sync\n" ::: "memory");
    } else {
	flush_L1_all();
    }
}


void flush_zoid_buf(struct zoid_buffer *buffer)
{
    int total_size = sizeof(struct zoid_buffer) + buffer->size;

    if(buffer->userbuf_in)
	total_size += buffer->userbuf_in_len;
    // XXX: why is there no such line for userbuf_out?

    if(total_size >= L1_SIZE) {
	flush_L1_all();
    } else {
	if(buffer->userbuf_in)
	    flush_L1_region(buffer->userbuf_in, buffer->userbuf_in_len);

	if(buffer->userbuf_out)
	    flush_L1_region(buffer->userbuf_out, buffer->total_len - 
			    buffer->result_len);
	
	/*flush_L1_region(buffer->data, buffer->size);
	flush_L1_region(buffer, sizeof(struct zoid_buffer));*/
	flush_L1_region(buffer, sizeof(struct zoid_buffer) + buffer->size);
    }
}
