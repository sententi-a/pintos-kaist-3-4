/* vm.c: Generic interface for virtual memory objects. */

#include "threads/malloc.h"
#include "vm/vm.h"
#include "vm/inspect.h"
/*#############################Newly added in Project 3##################################*/
#include "threads/mmu.h"
#include "include/lib/kernel/hash.h"
#include "userprog/process.h"

unsigned page_hash (const struct hash_elem *p_, void *aux UNUSED);
bool page_less (const struct hash_elem *a_, const struct hash_elem *b_, void *aux UNUSED);
void page_destroy (struct hash_elem *e, void *aux);
/*#######################################################################################*/

/* Initializes the virtual memory subsystem by invoking each subsystem's
 * intialize codes. */
void
vm_init (void) {
	vm_anon_init ();
	vm_file_init ();
#ifdef EFILESYS  /* For project 4 */
	pagecache_init ();
#endif
	register_inspect_intr ();
	/* DO NOT MODIFY UPPER LINES. */
	/* TODO: Your code goes here. */
	// 아마 ... maybe 프레임 테이블 
}

/* Get the type of the page. This function is useful if you want to know the
 * type of the page after it will be initialized.
 * This function is fully implemented now. */
enum vm_type
page_get_type (struct page *page) {
	int ty = VM_TYPE (page->operations->type);
	switch (ty) {
		case VM_UNINIT:
			return VM_TYPE (page->uninit.type);
		default:
			return ty;
	}
}

/* Helpers */
static struct frame *vm_get_victim (void);
static bool vm_do_claim_page (struct page *page);
static struct frame *vm_evict_frame (void);

/* Create the pending page object with initializer. If you want to create a
 * page, do not create it directly and make it through this function or
 * `vm_alloc_page`. */
bool
vm_alloc_page_with_initializer (enum vm_type type, void *upage, bool writable,
		vm_initializer *init, void *aux) {

	ASSERT (VM_TYPE(type) != VM_UNINIT)

	struct supplemental_page_table *spt = &thread_current ()->spt;

	/* Check wheter the upage is already occupied or not. */
	//printf("vm_alloc_page_with_initializer(): spt_find_page 부릅니다.\n");
	if (spt_find_page (spt, upage) == NULL) {
		/* FIXME: Create the page, fetch the initialier according to the VM type,
		 * FIXME: and then create "uninit" page struct by calling uninit_new. You
		 * FIXME: should modify the field after calling the uninit_new. */
		struct page *page = (struct page *) malloc (sizeof(struct page));
		// page->va = upage;
		//printf("vm_alloc_page_with_initializer(): spt_find_page 불렀습니다.\n");

		/*Fetch the initializer according to the VM type*/
		if (type == VM_ANON || type & VM_MARKER_0){
			uninit_new(page, upage, init, type, aux, anon_initializer);
		}
		else if (type == VM_FILE){
			uninit_new(page, upage, init, type, aux, file_backed_initializer);
		}
		/* setting writable (이건 Load할 때 프로그램 헤더 보고 알아냄) */
		page->writable = writable;

		/* setting resource */
		//page->resource = (struct resource *) aux;

		/* FIXME: Insert the page into the spt. */
		if (!spt_insert_page(spt, page)) {
			free(page);
			goto err;
		}
		// spt_insert_page(spt, page);
	}

	/* If the upage is already occupied... */
	// else {
	// 	goto err;
	// }

	return true;

/*언제 err로 가게 될까?*/
err:
	return false;
}

/* Find VA from spt and return page. On error, return NULL. */
struct page *
spt_find_page (struct supplemental_page_table *spt UNUSED, void *va UNUSED) {
	// struct page *page = NULL;
	/* FIXME: Fill this function. */
	/*#######################Newly added in Project 3######################*/
	/* Create a dummy page*/
	//struct page *page;
	struct page *page = (struct page *) malloc (sizeof(struct page));
	page->va = pg_round_down(va);
	
	/* Find hash_elem */
	struct hash_elem *e = hash_find(&spt->hash, &page->hash_elem);

	free(page);

	/* If not found, just return NULL*/
	if (!e) {
		//printf("spt_find_page(): spt에서 페이지를 찾지 못했습니다.\n");
		return NULL;
	}

	struct page *page_found= hash_entry(e, struct page, hash_elem);
	
	/*#####################################################################*/

	return page_found;
}

/* Insert PAGE into spt with validation. */
bool
spt_insert_page (struct supplemental_page_table *spt UNUSED,
		struct page *page UNUSED) {
	int succ = false;
	/* FIXME: Fill this function. */
	/*#####################Newly added in Project 3######################*/
	// printf("spt_insert_page(): hash_find 부르기 전\n");
	struct hash_elem *hash_elem = hash_find(&spt->hash, &page->hash_elem);
	/*If page already exists in SPT, return false*/
	if (hash_elem) {
		return succ;
	}
	
	/* If page does not exist, insert it into SPT */
	succ = true;
	hash_insert(&spt->hash, &page->hash_elem);
	// printf("spt_insert_page(): hash_insert 부른 후\n");
	/*###################################################################*/
	return succ;
}

void
spt_remove_page (struct supplemental_page_table *spt, struct page *page) {
	vm_dealloc_page (page);
	return true;
}

/* Get the struct frame, that will be evicted. */
static struct frame *
vm_get_victim (void) {
	struct frame *victim = NULL;
	 /* TODO: The policy for eviction is up to you. */

	return victim;
}

/* Evict one page and return the corresponding frame.
 * Return NULL on error.*/
static struct frame *
vm_evict_frame (void) {
	struct frame *victim UNUSED = vm_get_victim ();
	/* TODO: swap out the victim and return the evicted frame. */

	return NULL;
}

/* palloc() and get frame. If there is no available page, evict the page
 * and return it. This always return valid address. That is, if the user pool
 * memory is full, this function evicts the frame to get the available memory
 * space.*/
static struct frame *
vm_get_frame (void) {
	//struct frame *frame;
	struct frame *frame = (struct frame *) calloc (1, sizeof(struct frame));
	/*#############################Newly added in Project3############################*/
	/*------------------------------Frame Management----------------------------------*/
	/* FIXME: Fill this function. */
	frame->kva = palloc_get_page(PAL_USER);

	/* FIXME: Eviction */
	if (!frame->kva) {
		PANIC("todo");
	}
	//printf("frame 페이지의 주소값: 0x%x\n", frame->page);
	/* TODO: ADD to frame table */
	/*##############################################################################*/
	ASSERT (frame != NULL);
	ASSERT (frame->page == NULL);

	return frame;
}

/* Growing the stack. */
static void
vm_stack_growth (void *addr UNUSED) {
	
	if(!vm_alloc_page (VM_ANON | VM_MARKER_0, addr, 1)) return;
	if(!vm_claim_page (addr)) return;
	// printf("///////////vm_stack_grow////////////////");
}

/* Handle the fault on write_protected page */
static bool
vm_handle_wp (struct page *page UNUSED) {
}

/* Return true on success */
bool
vm_try_handle_fault (struct intr_frame *f UNUSED, void *addr UNUSED,
		bool user UNUSED, bool write UNUSED, bool not_present UNUSED) {
	struct supplemental_page_table *spt UNUSED = &thread_current ()->spt;
	struct page *page = NULL;
	/*###########################Newly added in Project 3##############################*/
	/*----------------------------------Lazy Loading------------------------------------*/
	/* FIXME: Validate the fault */
	/* FIXME: Your code goes here */
	struct page* stk_grow = pg_round_down(addr);
	uint8_t *stack_count = 1;

	//[GITBOOK] Finally, modify vm_try_handle_fault function to resolve the page struct 
	//corresponding to the faulted address by consulting to the supplemental page table through spt_find_page
	page = spt_find_page (spt, pg_round_down (addr));

	/* Valid page fault : accesses invalid */
	if (!page && !is_kernel_vaddr (addr)) { //CHECK: 조건식 변경
		
		/* stack_growth */
		void *stack_bottom = is_kernel_vaddr(f->rsp) ? f->rsp : thread_current()->rsp;	
		if(pg_round_down(f->rsp) == pg_round_down(addr - PGSIZE)){
			vm_stack_growth(stk_grow);
		}

		return false;
	}

	/* Bogus fault : (1) Lazy Load */
	if (not_present) {
		return vm_do_claim_page (page);
	}

	/* Bogus fault : (2) swaped-out page*/
	
	/* Bogus fault : (3) write-protected page (COW) */

	/*#################################################################################*/

	//return vm_do_claim_page (page);
}

/* Free the page.
 * DO NOT MODIFY THIS FUNCTION. */
void
vm_dealloc_page (struct page *page) {
	destroy (page);
	free (page);
}

/* Claim the page that allocate on VA. */
bool
vm_claim_page (void *va UNUSED) {
	struct page *page;
	/*###################Newly added in Project 3#####################*/
	/*-------------------------Frame Management-------------------------*/
	va = pg_round_down(va);

	
	/* FIXME: Fill this function */
	//spt_insert_page (&thread_current ()->spt, page);
	/* 없다면 만들어주고, 있다면 그냥 바로 vm_do_claim_page? */
	// spt에 먼저 페이지 넣은 후에 매핑?! (아직 spt에 페이지가 없을 때)
	// page의 다른 멤버 변수는 어떻게 처리?

	/* Find a allocated page(but not cached) that starts with va */
	page = spt_find_page (&thread_current ()->spt, va);

	/* Page not found */
	if (!page) {
		//printf("vm_claim_page(): spt에서 페이지를 찾지 못했습니다.\n");
		return false;
	}
	/*################################################################*/

	return vm_do_claim_page (page);
}

/* Claim the PAGE and set up the mmu. */
static bool
vm_do_claim_page (struct page *page) {
	struct frame *frame = vm_get_frame (); //get a free frame

	/* Set links */
	frame->page = page;
	page->frame = frame;

	/* FIXME: Insert page table entry to map page's VA to frame's PA. */
	/*##########################Newly added in Project 3#########################*/
	/*------------------------------Frame Management-----------------------------*/
	bool result = pml4_set_page(thread_current()->pml4, page->va, frame->kva, 1);

	/*If the operation was not successful...*/
	if (!result) {
		return false;
	}
	/*##########################################################################*/

	return swap_in (page, frame->kva);
}

/* Initialize new supplemental page table */
void
supplemental_page_table_init (struct supplemental_page_table *spt UNUSED) {
	/*#######################Newly added in Project 3#########################*/
	/*-----------------------Supplemental Page Table--------------------------*/
	hash_init(&spt->hash, page_hash, page_less, NULL);
	/*########################################################################*/
}

/* Copy supplemental page table from src to dst */
bool
supplemental_page_table_copy (struct supplemental_page_table *dst UNUSED,
		struct supplemental_page_table *src UNUSED) {
	/*FIXME: This is used when a child needs to inherit the execution context of its parent.
	  FIXME: Iterate through each page in the src's supplemental page table and make a exact copy of the entry in the dst's supplemental page table.
	  FIXME: You will need to allocate uninit page and claim them immediately.
	*/

	//hash_init (&dst->hash, page_hash, page_less, NULL); (밖에서 해준다고 함)
	struct hash_iterator hash_iter;

	/* Points to the first hash_elem in hash */	
	hash_first (&hash_iter, &src->hash);
	
	/* Insert all pages in src SPT into dst SPT */
	while (hash_next (&hash_iter)) {
		struct hash_elem *e = hash_cur (&hash_iter); // hash_iter.elem
		struct page *page = hash_entry (e, struct page, hash_elem);
		// page set up
		// struct resource는 어떻게 가져오죠 (o)
		enum vm_type type = page->operations->type;

		/* Type (1) : VM_UNINIT */
		if (type == VM_UNINIT) {
			bool success = vm_alloc_page_with_initializer(page->uninit.type, page->va, page->writable, page->uninit.init, page->uninit.aux);
			if (!success) {
				return false;
			}
			continue;
		}

		/* Type (2) : VM_MARKER_0 (stack) */
		// MARKER_0 (stack)일 때 (setup_stack ?-> 다음 루프로)
		// 인터럽트 프레임 어떻게 가져오죠 .. 
		type = page_get_type(page);

		if (type & VM_MARKER_0) {
			if (!vm_alloc_page(VM_ANON | VM_MARKER_0, page->va, page->writable)) {
				return false;
			}
			if (!vm_claim_page(page->va)) {
				return false;
			}
			// memcpy(new_page->frame->kva, page->frame->kva, PGSIZE);
			// setup_stack(&thread_current()->if_);
		}

		/* Type (3) : VM_ANON or VM_FILE */
		else {
			if (!vm_alloc_page(type, page->va, page->writable)) {
				return false;
			}
			if (!vm_claim_page(page->va)) {
				return false;
			}
		}

		struct page *new_page = spt_find_page(dst, page->va);
		memcpy(new_page->frame->kva, page->frame->kva, PGSIZE);
	}

	return true; 
}


/* Free the resource hold by the supplemental page table */
void
supplemental_page_table_kill (struct supplemental_page_table *spt UNUSED) {
	/* FIXME: Destroy all the supplemental_page_table hold by thread and
	 * FIXME: writeback all the modified contents to the storage. */
	 // iterate through the page entries in SPT! and just call destroy (page)
	 
	// struct hash_iterator hash_iter;

	// hash_first (&hash_iter, &spt->hash);

	// while (hash_next (&hash_iter)) {
	// 	struct hash_elem *e = hash_cur (&hash_iter);
	// 	// hash_delete (); 이걸 여기에? 아니면 anon_destroy에?

	// 	struct page *page = hash_entry (e, struct page, hash_elem);

	// 	destroy (page);
	// }

	hash_clear(&spt->hash, page_destroy);
	ASSERT(hash_empty(&spt->hash));
	// hash_clear나 hash_destroy를 써야 하나?
}

/*###################Newly added in Project 3######################*/
/*--------------------Supplemental Page Table----------------------*/
/* Hash Function*/
unsigned
page_hash (const struct hash_elem *p_, void *aux UNUSED) {
  const struct page *p = hash_entry (p_, struct page, hash_elem);
  return hash_bytes (&p->va, sizeof p->va);
}

/* Returns true if page a precedes page b. */
bool
page_less (const struct hash_elem *a_,
           const struct hash_elem *b_, void *aux UNUSED) {
  const struct page *a = hash_entry (a_, struct page, hash_elem);
  const struct page *b = hash_entry (b_, struct page, hash_elem);

  return a->va < b->va;
}

/*-------------------------Page Cleanup----------------------------*/
// void
// hash_clear (struct hash *h, hash_action_func *destructor) {
// 	size_t i;

// 	for (i = 0; i < h->bucket_cnt; i++) {
// 		struct list *bucket = &h->buckets[i];

// 		if (destructor != NULL)
// 			while (!list_empty (bucket)) {
// 				struct list_elem *list_elem = list_pop_front (bucket);
// 				struct hash_elem *hash_elem = list_elem_to_hash_elem (list_elem);
// 				destructor (hash_elem, h->aux);
// 			}

// 		list_init (bucket);
// 	}

// 	h->elem_cnt = 0;
// }
void
page_destroy (struct hash_elem *e, void *aux) {
	struct page *page = hash_entry(e, struct page, hash_elem);
	vm_dealloc_page(page);
}
/*################################################################*/
