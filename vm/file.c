/* file.c: Implementation of memory backed file object (mmaped object). */

#include "vm/vm.h"
/*################Newly added in Project 3###################*/
/*-------------------Memory Mapped Files---------------------*/
#include "threads/vaddr.h"
#include "include/lib/debug.h"
/*###########################################################*/

static bool file_backed_swap_in (struct page *page, void *kva);
static bool file_backed_swap_out (struct page *page);
static void file_backed_destroy (struct page *page);

/* DO NOT MODIFY this struct */
static const struct page_operations file_ops = {
	.swap_in = file_backed_swap_in,
	.swap_out = file_backed_swap_out,
	.destroy = file_backed_destroy,
	.type = VM_FILE,
};

/* The initializer of file vm */
void
vm_file_init (void) {
}

/* Initialize the file backed page */
bool
file_backed_initializer (struct page *page, enum vm_type type, void *kva) {
	/* Set up the handler */
	page->operations = &file_ops;

	struct file_page *file_page = &page->file;
}

/* Swap in the page by read contents from the file. */
static bool
file_backed_swap_in (struct page *page, void *kva) {
	struct file_page *file_page UNUSED = &page->file;
}

/* Swap out the page by writeback contents to the file. */
static bool
file_backed_swap_out (struct page *page) {
	struct file_page *file_page UNUSED = &page->file;
}

/* Destory the file backed page. PAGE will be freed by the caller. */
static void
file_backed_destroy (struct page *page) {
	struct file_page *file_page UNUSED = &page->file;
}


bool
lazy_load_segment_file_backed (struct page *page, void *aux) {
	struct resource *resource = (struct resource *) aux;

	/* Get a frame mapped to page */
	uint8_t *kpage = page->frame->kva;

	if (!kpage) {
		return false;
	}
	
	/* Read segment from file and load it to memory*/
	file_seek (resource->file, resource->offset);

	if (file_read(resource->file, kpage, resource->read_bytes) != (int) resource->read_bytes) {
		return false;
	}
	/* zeroing */
	memset(kpage + resource->read_bytes, 0, resource->zero_bytes);

	return true;
}

/* Do the mmap */
void *
do_mmap (void *addr, size_t length, int writable,
		struct file *file, off_t offset) {
			
	/*###############################Newly added in Project 3################################*/
	/*-------------------------------Memory Mapped Files-------------------------------------*/
	struct supplemental_page_table *spt = &thread_current()->spt;

	size_t read_bytes = length;
	size_t zero_bytes = read_bytes % PGSIZE == 0 ? 0 : pg_round_up(read_bytes) - read_bytes;

	addr = pg_round_down(addr);
	void *upage = addr;
	// addr = pg_round_down(addr);
	// open했는데 파일 크기가 0이면 ?

	/* Set up uninitialized pages */
	while (read_bytes > 0 || zero_bytes > 0) {
		size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
		size_t page_zero_bytes = PGSIZE - page_read_bytes;

		struct resource * resource = (struct resource *) malloc (sizeof(struct resource));
		resource->file = file;
		resource->offset = offset;
		resource->read_bytes = page_read_bytes;
		resource->zero_bytes = page_zero_bytes;

		void *aux = resource;
		
		/* If the range of pages mapped overlaps any existing set of mapped pages, 
		including the stack or pages mapped at executable load time, it must fail */
		if (!vm_alloc_page_with_initializer(VM_FILE, upage, 
					writable, lazy_load_segment_file_backed, aux)) {
			
			while (upage != addr) {
				spt_remove_page (spt, spt_find_page(spt, upage));
				upage -= PGSIZE;
			}
		
			return;
		}

		offset += page_read_bytes;

		read_bytes -= page_read_bytes;
		zero_bytes -= page_zero_bytes;
		upage += PGSIZE;
	}

	/*########################################################################################*/
}

/* Do the munmap */
void
do_munmap (void *addr) {
}
