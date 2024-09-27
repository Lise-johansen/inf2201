/*
 * memory.c
 * Note:
 * There is no separate swap area. When a data page is swapped out,
 * it is stored in the location it was loaded from in the process'
 * image. This means it's impossible to start two processes from the
 * same image without screwing up the running. It also means the
 * disk image is read once. And that we cannot use the program disk.
 *
 * Best viewed with tabs set to 4 spaces.
 */

#include "common.h"
#include "interrupt.h"
#include "kernel.h"
#include "memory.h"
#include "scheduler.h"
#include "thread.h"
#include "tlb.h"
#include "usb/scsi.h"
#include "util.h"

inline uint32_t get_directory_index(uint32_t vaddr);
inline uint32_t get_table_index(uint32_t vaddr);
inline void table_map_present(uint32_t *table, uint32_t vaddr, uint32_t paddr, int user);
inline void directory_insert_table(uint32_t *directory, uint32_t vaddr, uint32_t *table, int user);
inline void directory_insert_table(uint32_t *directory, uint32_t vaddr, uint32_t *table, int user);

/* Debug-function.
 * Write all memory addresses and values by with 4 byte increment to output-file.
 * Output-file name is specified in bochsrc-file by line:
 * com1: enabled=1, mode=file, dev=serial.out
 * where 'dev=' is output-file.
 * Output-file can be changed, but will by default be in 'serial.out'.
 *
 * Arguments
 * title:		prefix for memory-dump
 * start:		memory address
 * end:			memory address
 * inclzero:	binary; skip address and values where values are zero
 */
static void
rsprintf_memory(char *title, uint32_t start, uint32_t end, uint32_t inclzero)
{
	uint32_t numpage, paddr;
	char *header;

	rsprintf("%s\n", title);

	numpage = 0;
	header = "========================== PAGE NUMBER %02d ==========================\n";

	for (paddr = start; paddr < end; paddr += sizeof(uint32_t))
	{

		/* Print header if address is page-aligned. */
		if (paddr % PAGE_SIZE == 0)
		{
			rsprintf(header, numpage);
			numpage++;
		}
		/* Avoid printing address entries with no value. */
		if (!inclzero && *(uint32_t *)paddr == 0x0)
		{
			continue;
		}
		/* Print:
		 * Entry-number from current page.
		 * Physical main memory address.
		 * Value at address.
		 */
		rsprintf("%04d - Memory Loc: 0x%08x ~~~~~ Mem Val: 0x%08x\n",
				 ((paddr - start) / sizeof(uint32_t)) % PAGE_N_ENTRIES,
				 paddr,
				 *(uint32_t *)paddr);
	}
}

//-------------------------------------------------------------- global data strucks / variables --------------------------------------------------------------//
/*Array with size of pages*/
physical_page_t physical_page[PAGEABLE_PAGES];

/*List of free pages*/
static physical_page_t *next_free_page;

/*List of free pages*/
static physical_page_t *next_used_page = NULL;

/*Pointer to the kernel page directory*/
static uint32_t *kernel_page_directory = NULL;

/*Lock*/
static lock_t paging_lock;

//-------------------------------------------------------------- Helper function --------------------------------------------------------------//
void init_frame(void)
{
	for (int i = 0; i < (PAGEABLE_PAGES - 1); i++)
	{
		/*Point to the start of frame*/
		physical_page[i].start = (MEM_START + (PAGE_SIZE * i));

		/* Set variables*/
		physical_page[i].pid = i;
		physical_page[i].pin = FALSE;
		physical_page[i].fault_addr = 0;
		physical_page[i].swap_loc = 0;
		physical_page[i].swap_size = 0;
		physical_page[i].start_pc = 0;
		physical_page[i].page_dir = 0;

		/*Connect all pyhsical page togheter*/
		physical_page[i].next = &physical_page[i + 1];
	}

	/*Set that the last page points to zero*/
	physical_page[PAGEABLE_PAGES - 1].next = NULL;

	/*Point to the start of frame*/
	physical_page[PAGEABLE_PAGES - 1].start = (MEM_START + (PAGE_SIZE * (PAGEABLE_PAGES - 1)));

	/* Set variables*/
	physical_page[PAGEABLE_PAGES - 1].pin = FALSE;
	physical_page[PAGEABLE_PAGES - 1].pid = (PAGEABLE_PAGES - 1);
	physical_page[PAGEABLE_PAGES - 1].fault_addr = 0;
	physical_page[PAGEABLE_PAGES - 1].swap_loc = 0;
	physical_page[PAGEABLE_PAGES - 1].swap_size = 0;
	physical_page[PAGEABLE_PAGES - 1].start_pc = 0;
	physical_page[PAGEABLE_PAGES - 1].page_dir = NULL;

	/*Next page*/
	next_free_page = &physical_page[0];
}

physical_page_t *page_alloc(void)
{
	/*If there is no free pages*/
	if (next_free_page == NULL)
	{
		// random_page_replacment();
		find_first_unpinned_page();
	}

	/* Allocate page and set the next free page*/
	physical_page_t *page = next_free_page;
	next_free_page = page->next;

	/*Set variabl*/
	page->pin = FALSE;

	/*Manage the used page list*/
	insert_page(page);

	scrprintf(24, 1, "Memory address: %x 	Page used %d /37", page->start, page->pid);
	return page;
}

physical_page_t *insert_page(physical_page_t *page)
{
	/*IF the list is empty, just insert the page*/
	if (next_used_page == NULL)
	{
		/*The page is insertet inside list and points to NULL*/
		next_used_page = page;
		page->next = NULL;
	}
	else
	{
		physical_page_t *tmp = next_used_page;

		/*Iterate through the list until the last page (the page point to NULL) */
		while (tmp->next != NULL)
		{
			/*To next page*/
			tmp = tmp->next;
		}

		/*
		 * The last pages pointer points no longer points
		 * to NULL but the new page, and the new page points to NULL.
		 */
		tmp->next = page;
		page->next = NULL;
	}
}

physical_page_t *random_page_replacment(void)
{
	while (1)
	{
		/*Get random index*/
		uint32_t rand_index = rand() % (PAGEABLE_PAGES - 1);
		physical_page_t *page = &physical_page[rand_index];
		scrprintf(24, 58, "Evicting page %d", page->pid);

		if (!page->pin)
		{
			/*Get the fault address and other relevant variables*/
			uint32_t fault_addr = page->fault_addr;
			uint32_t pd = get_directory_index(fault_addr);
			uint32_t *page_table = (uint32_t *)(page->page_dir[pd] & PE_BASE_ADDR_MASK);

			/*Unmap the virtual page using the physical page*/
			for (uint32_t vaddr = page->start_pc; vaddr < (page->swap_size * SECTOR_SIZE); vaddr += PAGE_SIZE)
			{
				int bit_to_set = 6;
				int mask = 0xfffffff8;

				uint32_t index = get_table_index(vaddr);
				page_table[index] = (vaddr & mask) | bit_to_set;
			}

			/*Get offset from bace address*/
			uint32_t offset = (((page->fault_addr - page->start_pc) / PAGE_SIZE) * SECTORS_PER_PAGE);
			int size = (page->swap_size - offset);
			scrprintf(1, 3, "Random: %d", offset);

			/*Determine to size to be read in either actually size or max size*/
			uint32_t read_size = (size > SECTORS_PER_PAGE) ? SECTORS_PER_PAGE : size;

			/*Write from RAM to disk, zero out page and retrun index*/
			scsi_write((page->swap_loc + offset), read_size, (char *)page->start);

			/*Flush cache*/
			flush_tlb_entry(fault_addr);

			/*Zero out page*/
			bzero((char *)page->start, PAGE_SIZE);
			return page;
		}
	}
}

physical_page_t *find_first_unpinned_page(void)
{
	while (1)
	{
		/*Allocate page and set the next used page*/
		physical_page_t *page = next_used_page;

		scrprintf(24, 58, "Evicting page %d", page->pid);
		scrprintf(23, 58, "%d", page->pid);

		if (!page->pin)
		{
			/*Get the fault address and other relevant variables*/
			uint32_t fault_addr = page->fault_addr;
			uint32_t pd = get_directory_index(fault_addr);
			uint32_t *page_table = (uint32_t *)(page->page_dir[pd] & PE_BASE_ADDR_MASK);

			/*Unmap the virtual page using the physical page*/
			for (uint32_t vaddr = page->start_pc; vaddr < (page->swap_size * SECTOR_SIZE); vaddr += PAGE_SIZE)
			{
				int bit_to_set = 6;
				int mask = 0xfffffff8;

				uint32_t index = get_table_index(vaddr);
				page_table[index] = (vaddr & mask) | bit_to_set;
			}

			/*Get offset for bace address*/
			uint32_t offset = (((page->fault_addr - page->start_pc) / PAGE_SIZE) * SECTORS_PER_PAGE);
			int size = (page->swap_size - offset);
			scrprintf(1, 3, "evict polly. offset: %d", offset);

			/*Determine to size to be read in either max size or extra*/
			uint32_t read_size = (size > SECTORS_PER_PAGE) ? SECTORS_PER_PAGE : size;

			/*Write from RAM to disk, zero out page and retrun index*/
			ms_delay(100000);
			scsi_write((page->swap_loc + offset), read_size, (char *)page->start);

			/*Flush cache*/
			flush_tlb_entry(fault_addr);

			/*Zero out page*/
			bzero((char *)page->start, PAGE_SIZE);

			/*
			 * Put the evictet page last in the used list
			 * Save frefrence the the evicted page in evicted_page.
			 * Save the next page in the used list in next_used_page.
			 * Insert the evicted page last in the used list.
			 */

			physical_page_t *evicted_page = page;
			next_used_page = page->next;
			insert_page(evicted_page);

			return page;
		}
		next_used_page = page->next;
	}
}

static void make_common_map(uint32_t *page_directory, int user)
{
	/*Variable*/
	uint32_t *page_table, addr;
	physical_page_t *tmp;

	/* Allocate memory for the page table, and set page table at the start of memory*/
	tmp = page_alloc();
	tmp->pin = TRUE;
	page_table = (uint32_t *)tmp->start;

	/* Identity map the first 640KB of base memory - Kernel*/
	for (addr = 0; addr < 640 * PAGE_N_ENTRIES; addr += PAGE_SIZE)
	{
		table_map_present(page_table, addr, addr, 0);
	}

	/* Identity map the video memory, from 0xb8000-0xb8fff. */
	table_map_present(page_table, (uint32_t)SCREEN_ADDR, (uint32_t)SCREEN_ADDR, user);

	/*If it is a thread, identity map the memory, else it is a process and to something else.*/
	if (!user)
	{
		/*
		 * Identity map in the rest of the physical memory
		 * so the kernel can access everything in memory directly.
		 */
		for (addr = MEM_START; addr < MAX_PHYSICAL_MEMORY; addr += PAGE_SIZE)
		{
			table_map_present(page_table, addr, addr, 0);
		}
	}
	else
	{
		/*
		 * Process know where kernel + vidoe buffer are.
		 */
		for (addr = MEM_START; addr < MAX_PHYSICAL_MEMORY; addr += PAGE_SIZE)
		{
			table_map_present(page_table, addr, addr, 0);
		}

		/*
		 * Allocate a stack table and pin a page for user stack.
		 * Map the virtual address of the user stack to the physical page.
		 */
		physical_page_t *stack_table = page_alloc();
		stack_table->pin = TRUE;
		directory_insert_table(page_directory, PROCESS_STACK, (uint32_t *)stack_table->start, 1);

		/* Allocate a page for the user stack. */
		physical_page_t *stack_page = page_alloc();
		stack_page->pin = TRUE;
		table_map_present((uint32_t *)stack_table->start, PROCESS_STACK, stack_page->start, 1);
	}

	/*
	 * Insert in page_directory an entry for virtual address 0,
	 * that points to physical address of page_table.
	 */
	directory_insert_table(page_directory, 0, page_table, user);
}

//-------------------------------------------------------------- NEED TO DO FROM PRECODE --------------------------------------------------------------//
void init_memory(void)
{
	/* Initialize the lock for the paging system.*/
	lock_init(&paging_lock);
	/*Call on memory and allocate a page for a temp value*/
	init_frame();
	physical_page_t *tmp = page_alloc();

	/*Set up kernel page directiery*/
	kernel_page_directory = (uint32_t *)tmp->start;

	/* Set variables*/
	tmp->pin = TRUE;

	/*Set up common map, make table, map it out, set it in page directory (last step by directory_insert_table)*/
	make_common_map(kernel_page_directory, 0);
}

void setup_page_table(pcb_t *p)
{
	uint32_t *page_directory, *page_table;

	lock_acquire(&paging_lock);

	if (p->is_thread)
	{
		/*Threads use the kernels page directory, so just set a pointer to that one and return.*/
		p->page_directory = kernel_page_directory;
		lock_release(&paging_lock);
		return;
	}
	else
	{
		/* Allocate a page directory, call on map.*/
		physical_page_t *process_directory = page_alloc();
		process_directory->pin = TRUE;
		page_directory = (uint32_t *)process_directory->start;
		make_common_map(page_directory, 1);

		/*
		 * Allocate a process table and pin a page for process table.
		 * Entry in page directroy points the page table (start of process table)
		 */
		physical_page_t *process_table = page_alloc();
		process_table->pin = TRUE;

		page_table = (uint32_t *)process_table->start;
		directory_insert_table(page_directory, p->start_pc, page_table, 1);

		/*
		 * Mark all pages belonging to process (code/data) as not present.
		 * int mask  = 6 -> Binary 110 -> not present, read/write, user
		 * Mask to get the needed bits -> 0xfffffff8
		 */
		for (uint32_t vaddr = p->start_pc; vaddr < (p->swap_size * SECTOR_SIZE); vaddr += PAGE_SIZE)
		{
			int bit_to_set = 6;
			int mask = 0xfffffff8;

			uint32_t index = get_table_index(vaddr);
			page_table[index] = (vaddr & mask) | bit_to_set;
		}

		/*Set page directory as process page directory*/
		p->page_directory = page_directory;

		lock_release(&paging_lock);
	}
}

void page_fault_handler(void)
{
	/*
	 * A program executing at user privilege level (3) attempted a
	 * read, resulting in a page table or page not present.
	 * error code 4 = 0b100
	 *
	 * A program executing at user privilege level (3) attempted a write, resulting in a page table or page not present.
	 * error code 6 = 0b110
	 *
	 * A program executing at supervisor privilege level (0, 1, or 2)
	 * attempted a read, resulting in a page table or page not
	 * present.
	 * error code 0 = 0b000
	 *
	 * A program executing at supervisor privilege level (0, 1, or 2)
	 * attempted a write, resulting in a page table or page not
	 * present.
	 * error code 2 = 0b010
	 *
	 * If everything is setup correctly this eror code should never happen.
	 */

	lock_acquire(&paging_lock);
	switch (current_running->error_code)
	{
	case 0b000:
	case 0b100:
	case 0b010:
	case 0b110:

		/*Increment the page fault count*/
		current_running->page_fault_count++;

		/*Get the fault address and other relevant variables*/
		uint32_t fault_addr = current_running->fault_addr;

		/*
		 * Get index from directory table to the fault address
		 * Get the page table of the faulting address using the directory index.
		 */
		uint32_t pd = get_directory_index(fault_addr);
		uint32_t *page_table = (uint32_t *)(current_running->page_directory[pd] & PE_BASE_ADDR_MASK);

		/* Allocate a physical page to map the faulting page into.*/
		physical_page_t *new_page = page_alloc();

		/*Pin the keyboard*/
		if (current_running->pid == 5)
		{
			new_page->pin = TRUE;
		}

		/*
		 *Assign the following values to the new page, uses this in the rep. polly.
		 *so that the page have access the needed information.
		*/
		new_page->fault_addr = current_running->fault_addr;
		new_page->swap_loc = current_running->swap_loc;
		new_page->swap_size = current_running->swap_size;
		new_page->start_pc = current_running->start_pc;
		new_page->page_dir = current_running->page_directory;

		/*
		 * Calculate the offset of the page within the process's virtual memory.
		 * Between start of process and fault address = offset.
		 * Align with start of the page = devide by page size.
		 * Multiplay the offset align with pages to a number of sectors = convert it to the number of sectors.
		 */
		uint32_t offset = (((current_running->fault_addr - current_running->start_pc) / PAGE_SIZE) * SECTORS_PER_PAGE);

		/*
		 * Finde the remainder of the process that do not fit in a page.
		 */
		int size = (current_running->swap_size - offset);
		scrprintf(2, 3, "pfh offet: %d", offset);

		/*
		 * If the size is larger than the number of sectors in a page, the size is the number of sectors in a page.
		 * Else the size is the remainder of the process, e.g. the left over of the process.
		 * Source for this idea: https://www.scaler.com/topics/c/ternary-operator-in-c/
		 */
		uint32_t read_size = (size > SECTORS_PER_PAGE) ? SECTORS_PER_PAGE : size;

		/*Read from disk to RAM */
		scsi_read((current_running->swap_loc + offset), read_size, (char *)new_page->start);

		/* Map the faulting page to the allocated physical page in the page table.*/
		table_map_present(page_table, current_running->fault_addr, new_page->start, 1);

		/*Release lock and flush the tlb entry*/
		lock_release(&paging_lock);
		break;

	default:
		lock_release(&paging_lock);
		break;
	}
}
