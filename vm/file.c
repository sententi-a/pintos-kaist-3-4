/* file.c: Implementation of memory backed file object (mmaped object). */

#include "vm/vm.h"
/*################Newly added in Project 3###################*/
/*-------------------Memory Mapped Files---------------------*/
#include "threads/vaddr.h"
#include "include/lib/debug.h"
#include "threads/mmu.h"
#include "filesys/file.h"
#include "filesys/inode.h"
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

	/*################Newly added in Project 3###################*/
	/*-------------------Memory Mapped Files---------------------*/
	/* Plant a file resource that is backing the memory */
	struct resource *resource = (struct resource *) page->uninit.aux;

	struct file_page *file_page = &page->file;

	if (resource) {
		file_page->resource = resource;
	}

	return true;
	/*###########################################################*/
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
	/*###################Newly added in Project3##################*/
	/*--------------------Memory Mapped Files---------------------*/
	// free(file_page->resource);
	/*############################################################*/
}

/*###############################Newly added in Project 3################################*/
/*-------------------------------Memory Mapped Files-------------------------------------*/

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
/*########################################################################################*/

/* Do the mmap */
void *
do_mmap (void *addr, size_t length, int writable,
		struct file *file, off_t offset) {
	
	addr = pg_round_down (addr);

	struct supplemental_page_table *spt = &thread_current()->spt;

	size_t read_bytes = length;
	size_t zero_bytes = read_bytes % PGSIZE == 0 ? 0 : pg_round_up(read_bytes) - read_bytes;

	void *upage = addr;
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
		resource->mmap_addr = addr;
		
		/* If the range of pages mapped overlaps any existing set of mapped pages, 
		including the stack or pages mapped at executable load time, it must fail */
		if (!vm_alloc_page_with_initializer(VM_FILE, upage, 
					writable, lazy_load_segment_file_backed, resource)) {
			
			free (resource);

			while (upage != addr) {
				spt_remove_page (spt, spt_find_page (spt, upage));
				upage -= PGSIZE;
			}
		
			return NULL;
		}

		offset += page_read_bytes;
		read_bytes -= page_read_bytes;
		zero_bytes -= page_zero_bytes;
		upage += PGSIZE;
	}

	/* If successful, returns the virtual address where the file is mapped */
	return addr;
}

/* Do the munmap */
void
do_munmap (void *addr) {
	//dirty flag 확인 후 write-back to the file (while 돌면서 inode가 일치할 때까지?, 페이지가 있을 때까지)

	void *uaddr = addr; // addr for iteration
	void *kaddr;

	struct supplemental_page_table *spt = &thread_current()->spt;
	uint64_t *pml4 = thread_current()->pml4;
	struct page *page;
	// struct file *backing_file = 

	while (page = spt_find_page(spt, uaddr)) {

		/* Out of mmaped page range */
		// uninitialized 페이지인 경우에는 ?
		if (page_get_type(page) != VM_FILE || page->file.resource->mmap_addr != addr) {
			break;
		}

		/* If the page is not mapped to a frame */
		if (! (kaddr = pml4_get_page (pml4, uaddr))) {
			spt_remove_page (spt, page);
			continue;
		}

		/* If it's dirty */
		else {
			spt_remove_page(spt, page);
		}

		uaddr += PGSIZE;

		/* If it's not dirty *
		/
		/* If page is modified by process, write it back to the file in the disk */
		// if (pml4_is_dirty (pml4, uaddr)) {
		// 	file_write_at ()
		// 	/* If there's zeroing, just discard it */
			
		// 	// zero_bytes도 생각해야 함 
		// 	// zero_bytes가 >0 이면, 그거 빼고 file_write? -> 그냥 read_bytes만큼씩 계속 write해주면 될듯
		// 	//file_seek() 후 file_write() 해야 할듯 
		// }

		// /* If page is */
		// else {

		// }

		// struct inode *inode = file->inode;
		// struct inode *inode = backing_file->inode;
	}

	return;
	//pml4_clear_page 후 spt_remove_page도 해야할 듯 

	/*You should use the file_reopen function 
	to obtain a separate and independent reference 
	to the file for each of its mappings.*/
	// file_reopen();
	
}
/*########################################################################################*/