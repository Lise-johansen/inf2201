#ifndef MEMORY_H
#define MEMORY_H

#include "kernel.h"
#include "tlb.h"
#include "util.h"

#define PROCESS_STACK_SIZE 4096

enum
{
	/* physical page facts */
	PAGE_SIZE = 4096,
	PAGE_N_ENTRIES = (PAGE_SIZE / sizeof(uint32_t)),
	SECTORS_PER_PAGE = (PAGE_SIZE / SECTOR_SIZE),

	PTABLE_SPAN = (PAGE_SIZE * PAGE_N_ENTRIES),

	/* page directory/table entry bits (PMSA p.235 and p.240) */
	PE_P = 1 << 0,					/* present */
	PE_RW = 1 << 1,					/* read/write */
	PE_US = 1 << 2,					/* user/supervisor */
	PE_PWT = 1 << 3,				/* page write-through */
	PE_PCD = 1 << 4,				/* page cache disable */
	PE_A = 1 << 5,					/* accessed */
	PE_D = 1 << 6,					/* dirty */
	PE_BASE_ADDR_BITS = 12,			/* position of base address */
	PE_BASE_ADDR_MASK = 0xfffff000, /* extracts the base address */

	/* Constants to simulate a very small physical memory. */
	MEM_START = 0x100000, /* 1MB */
	PAGEABLE_PAGES = 37,
	MAX_PHYSICAL_MEMORY = (MEM_START + PAGEABLE_PAGES * PAGE_SIZE),

	/* number of kernel page tables */
	N_KERNEL_PTS = 1,

	PAGE_DIRECTORY_BITS = 22,		  /* position of page dir index */
	PAGE_TABLE_BITS = 12,			  /* position of page table index */
	PAGE_DIRECTORY_MASK = 0xffc00000, /* page directory mask, most significant bit, first 10 = directory index/ offset */
	PAGE_TABLE_MASK = 0x003ff000,	  /* page table mask, middle significant bit, middle 10 = table index/ offset*/
	PAGE_MASK = 0x00000fff,			  /* page offset mask, least significant bit, last 10 = page index/ offset*/
	/* used to extract the 10 lsb of a page directory entry */
	MODE_MASK = 0x000003ff,

	PAGE_TABLE_SIZE = (1024 * 4096 - 1) /* size of a page table in bytes */
};

/* Struct*/
/*keep track of the pages, specifically the linked list of page tables.*/
typedef struct physical_page physical_page_t;

struct physical_page
{
	uint32_t pin;	/*ID to page*/
	uint32_t pid;	/* TRUE if page is pinned and can not be altered, and FALSE if not pinned can be altered*/
	uint32_t start; /*Start of the physical memory*/

	struct physical_page *next;		 /*Pointer to the next page*/
	struct physical_page *next_used; /*Pointer to the next used page*/

	uint32_t fault_addr; /*Address of the fault*/
	uint32_t swap_loc;	 /*Location of the swap*/
	uint32_t swap_size;	 /*Size of the swap*/
	uint32_t start_pc;	 /*Start address of process*/
	uint32_t *page_dir;	 /*Pointer to a page directory*/
};

/*
 * Make a frame in physical memory and set var vailus.
 * Input: void
 * Output: void
 * Inspired from project 4 kernel.c file init_pcb_table(), line 472-485.
 */
void init_frame(void);

/*
 * Allocate a free physical page, zero it out, and return a pointer to it
 * Input: void
 * Output: physical_page_t *page
 * Inspired from project 4 kernel.c file alloc_pcb(), line 565-580.
 */
physical_page_t *page_alloc(void);

/*
 * Get the random index and evicte that page from page list.
 * Input: void
 * Output: physical_page_t *page
 */
physical_page_t *random_page_replacment(void);

/*
 * Get the fist unpinned page and evicte that page from page list.
 * Input: void
 * Output: physical_page_t *page
 */
physical_page_t *find_first_unpinned_page(void);

/*
 * Insert a page at the end of used page list.
 * Input: physical_page_t *page
 * Output: void
 */
physical_page_t *insert_page(physical_page_t *page);

/*
 * Note:
 * - we identity map the pages, so that physical address is
 *   the same as the virtual address.
 *
 * - The user processes need access video memory directly, so we set
 *   the USER bit for the video page if we make this map in a user
 *   directory.
 *
 * Input: int is_thread, uint32_t *page_directory, int user
 * Output: void
 * This function is from project 4 paging.c file., line 175-202. with some changes.
 */
static void make_common_map(uint32_t *page_directory, int user);

/* Prototypes */
/*
 * innit_memory()
 * Initialize the memory system, called from kernel.c: _start()
 * You need to set up the virtual memory map for the kernel here.
 *
 * Input: void
 * Output: void
 */
void init_memory(void);

/*
 * Set up a page directory and page table for the process and fill in any necessary information in the pcb.
 * Called by create_process() in scheduler.c, every time a new process/thread is loaded in.
 *
 * Input: pcb_t *p
 * output: void
 * Important: Inspired from project 4 paging.c file make_page_directory(), line 251-289.
 */
void setup_page_table(pcb_t *p);

/*
 * Page fault handler, called from interrupt.c: exception_14().
 * Should handle demand paging
 * Interrupts are on when calling this function.
 * This function is called when a page fault occurs.  The faulting page is not in memory (memory-mapped file needed).
 * Input: void
 * Output: void
 */
void page_fault_handler(void);

/* Small helper functions */

/*
 * Use virtual address to get index in page directory.
 * Input: uint32_t vaddr
 * Output: uint32_t
 */
inline uint32_t get_directory_index(uint32_t vaddr)
{
	return (vaddr & PAGE_DIRECTORY_MASK) >> PAGE_DIRECTORY_BITS;
}

/*
 * Use virtual address to get index in a page table.
 * input: uint32_t vaddr
 * output: uint32_t
 */
inline uint32_t get_table_index(uint32_t vaddr)
{
	return (vaddr & PAGE_TABLE_MASK) >> PAGE_TABLE_BITS;
}

/*
 * Maps a page as present in the page table.
 *
 * 'vaddr' is the virtual address which is mapped to the physical
 * address 'paddr'.
 *
 * If user is nonzero, the page is mapped as accessible from a user
 * application.
 *
 * input: uint32_t *table, uint32_t vaddr, uint32_t paddr, int user
 * output: void
 *
 * Important: Function from project 4 paging.c file., line 122-131.
 */
inline void table_map_present(uint32_t *table, uint32_t vaddr, uint32_t paddr, int user)
{
	int access = PE_RW | PE_P, index = get_table_index(vaddr);

	if (user)
		access |= PE_US;

	table[index] = (paddr & ~PAGE_MASK) | access;
}

/*
 * Information: Make an entry in the page directory pointing to the given page
 * table.  vaddr is the virtual address the page table start with
 * table is the physical address of the page table
 *
 * If user is nonzero, the page is mapped as accessible from a user
 * application.
 *
 * input: uint32_t *directory, uint32_t vaddr, uint32_t *table, int user
 * output: void
 *
 * Important: Function from project 4 paging.c file, line 141-153.
 */
inline void directory_insert_table(uint32_t *directory, uint32_t vaddr, uint32_t *table, int user)
{
	int access = PE_RW | PE_P, index = get_directory_index(vaddr);
	uint32_t taddr;

	if (user)
		access |= PE_US;

	taddr = (uint32_t)table;

	directory[index] = (taddr & ~PAGE_MASK) | access;
}

#endif /* !MEMORY_H */
