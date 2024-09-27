#include "fs.h"

#ifdef LINUX_SIM
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#endif /* LINUX_SIM */

#include "block.h"
#include "common.h"
#include "fs_error.h"
#include "inode.h"
#include "kernel.h"
#include "superblock.h"
#include "thread.h"
#include "util.h"

#define BITMAP_ENTRIES 256

#define INODE_TABLE_ENTRIES 20

static char inode_bmap[BITMAP_ENTRIES]; // INODE_TABLE_ENTRIES
static char dblk_bmap[BITMAP_ENTRIES];	// INODE_TABLE_ENTRIES * INODE_NDIRECT

int min(int a, int b);
int dir_blk(int offset);
void file_descripter_init(void);
int initialize_bitmap(char *bitmap);
void create_root_directory(void);
int make_file(const char *filename, int mode);
void insert_entry(inode_t new_index, struct dirent array[DIRENTS_PER_BLK], const char *filename);
void insert_dir_entry(inode_t curr_inode, inode_t parent_inode, struct dirent array[DIRENTS_PER_BLK]);
void remove_entry(inode_t inode, struct dirent array[DIRENTS_PER_BLK]);

static int get_free_entry(unsigned char *bitmap);
static int free_bitmap_entry(int entry, unsigned char *bitmap);
static inode_t name2inode(char *name);
static blknum_t ino2blk(inode_t ino);
static blknum_t idx2blk(int index);

struct disk_superblock fs_disk;
struct mem_superblock fs_mem;
struct mem_inode fs_inode_table[INODE_TABLE_ENTRIES];

/*Init the file system*/
void fs_init(void)
{
	/*Initialize the device*/
	block_init();

	/*The file system exist - Mount super block by reading superblock*/
	block_read_part(S_SUP_BLK, 0, sizeof(struct disk_superblock), &fs_mem.d_super);

	/*If file system do not exist - Make and mount it*/
	if (fs_mem.d_super.s_magic_nr != S_MAGIC)
	{
		fs_mkfs();
	}

	/*Connect the bitmap in disk to mem superblocks bitmap fields.*/
	block_read_part(S_DISK_BITMAP_POS, 0, sizeof(inode_bmap), &inode_bmap);
	block_read_part(S_DISK_BITMAP_POS, BITMAP_ENTRIES, sizeof(dblk_bmap), &dblk_bmap);

	fs_mem.dirty = 'F';
}

/* Make a new file system.*/
void fs_mkfs(void)
{
	/*Write new superblock - Formating*/
	fs_disk.s_magic_nr = S_MAGIC;
	fs_disk.max_filesize = S_FILE;								// 8 *512 = 4096
	fs_disk.ndata_blks = (INODE_TABLE_ENTRIES * INODE_NDIRECT); // 20 inodes * 8 blocks per inode = 160 data blocks
	fs_disk.ninodes = INODE_TABLE_ENTRIES;
	fs_disk.root_inode = 0;

	/*Mark indoes and data block as free*/
	initialize_bitmap((inode_bmap));
	initialize_bitmap((dblk_bmap));

	/*Write inode to disk and init starting values*/
	for (int i = 0; i < INODE_TABLE_ENTRIES; i++)
	{
		/*Value to the mem/runntime inode - Usless just for peace of minde*/
		fs_inode_table[i].dirty = 'F';
		fs_inode_table[i].open_count = 0;
		fs_inode_table[i].inode_num = i;
		fs_inode_table[i].pos = 0;

		/*Set defualt value to disk indoe and write it down to disk*/
		fs_inode_table[i].d_inode.nlinks = 0;
		fs_inode_table[i].d_inode.size = 0;
		block_modify(S_INODE_POS + i, 0, &fs_inode_table[i], sizeof(struct disk_inode));
	}

	/*creat root directory (/)*/
	create_root_directory();

	/*Inizilize file discriptor table*/
	file_descripter_init();

	/*Write the bitmaps to disk*/
	block_modify(S_DISK_BITMAP_POS, 0, &inode_bmap, sizeof(inode_bmap));
	block_modify(S_DISK_BITMAP_POS, BITMAP_ENTRIES, &dblk_bmap, sizeof(dblk_bmap));

	/*write the superblock to disk*/
	block_modify(S_SUP_BLK, 0, &fs_disk, sizeof(struct disk_superblock));

	/*Mount file system*/
	fs_mem.d_super = fs_disk;
}

int fs_open(const char *filename, int mode)
{
	if ((strlen(filename) - 1) > MAX_FILENAME_LEN)
		return FSE_NAMETOLONG;

	/*Check if file exist, if not make one*/
	inode_t inode = name2inode((char *)filename);
	if (inode == -1)
	{
		if (mode == MODE_RDONLY)
			return FSE_NOTEXIST; // -5

		/* Create the file and return file pointer */
		int fd = make_file(filename, mode);
		return fd;
	}
	
	/*Check if i am cating a directory - check do not work*/
	if((mode == CAT) && (fs_inode_table[inode].d_inode.type == INTYPE_DIR))
		return FSE_ERROR; // -1

	/*Check if file is already open, if not set values and return file pointer*/
	for (int file_ptr = 0; file_ptr < MAX_OPEN_FILES; file_ptr++)
	{
		if (current_running->filedes[file_ptr].mode == MODE_UNUSED)
		{
			current_running->filedes[file_ptr].mode = mode;
			current_running->filedes[file_ptr].idx = inode;
			fs_inode_table[inode].open_count += 1;
			fs_inode_table[inode].pos = 0;
			return file_ptr; 
		}
	}
	return FSE_OK;
}

int fs_close(int fd)
{
	if (fd < 0 || fd > MAX_OPEN_FILES)
		return FSE_INVALIDHANDLE; // -6

	current_running->filedes[fd].mode = MODE_UNUSED;
	fs_inode_table[current_running->filedes[fd].idx].open_count--;
	current_running->filedes[fd].idx = -1;
	return FSE_OK;
}

int fs_read(int fd, char *buffer, int size)
{
	if (size <= 0)
		return FSE_OK;
	if (fd < 0 || fd > MAX_OPEN_FILES)
		return FSE_INVALIDHANDLE;
	if (current_running->filedes[fd].mode == MODE_WRONLY)
		return FSE_INVALIDMODE;

	/*Get the inode number from the file descriptor of the current running.*/
	inode_t inode = current_running->filedes[fd].idx;
	/*Sore the current position (offset) within the file, curresponding inode*/
	int offset = fs_inode_table[inode].pos;

	/* Read the inode data from disk into memory.
	 * It reads the disk block containging the needed indoe and retrives the corresponding disk node*/
	struct mem_inode from_mem_inode; 
	block_read_part(ino2blk(inode), 0, sizeof(struct disk_inode), &from_mem_inode.d_inode);

	int total_read_bytes = 0;

	/* Read the data that spane multiple blocks.
	 * Read data until either the remaning size to read are zero or the offset exceeds the size of the file*/
	while (size > 0 && offset < from_mem_inode.d_inode.size)
	{
		/* Get the data block associated with the inode.
		 * Obtains the block number from the direct array, using the helper function dir_blk(offset) to determine the correct index.*/
		int current_block = from_mem_inode.d_inode.direct[dir_blk(offset)];

		/*Calculate the remaining bytes to read*/
		int remaining_block_bytes = BLOCK_SIZE - (offset % BLOCK_SIZE);

		/*Only the needed data is read - minimum value between the remaining size to read and block size*/
		int bytes_to_read = min(size, remaining_block_bytes);

		/*Read data from the current data block*/
		block_read_part(current_block, offset % BLOCK_SIZE, bytes_to_read, buffer); 

		/*Ensure that data is read from the correct position - update offset, buffer, and remaining size*/
		offset += bytes_to_read;
		buffer += bytes_to_read;
		size -= bytes_to_read;
		total_read_bytes += bytes_to_read;

		/* When data block is fully read, check if there is more data block by retrieving the block number.
		 * If the block number is 0, no more data block. If not, reset the offset to 0 to read the next data block.*/
		if (offset == BLOCK_SIZE)
		{
			int next_block = from_mem_inode.d_inode.direct[dir_blk(offset)];
			if (next_block == 0)
				break;
			current_block = next_block;
			offset = 0;
		}
	}
	/* Update the pos to inode in the inode table.*/
	fs_inode_table[inode].pos = offset;

	/*Returns the number of bytes read.*/
	return total_read_bytes;
}

int fs_write(int fd, char *buffer, int size)
{
	if (size <= 0)
		return FSE_ERROR;
	if (fd < 0 || fd > MAX_OPEN_FILES)
		return FSE_INVALIDHANDLE;
	if (current_running->filedes[fd].mode == MODE_RDONLY)
		return FSE_INVALIDMODE;

	/*Get the inode number from the file descriptor*/
	inode_t inode = current_running->filedes[fd].idx;

	/*Get the offset within the file corresponding to the inode*/
	int offset = fs_inode_table[inode].pos;

	/*Write the inode data from disk into memory*/
	struct mem_inode inode_from_table;
	block_read_part(ino2blk(inode), 0, sizeof(struct disk_inode), &inode_from_table.d_inode);

	/*Get the data block associated with the inode*/
	int write_data = inode_from_table.d_inode.direct[dir_blk(offset)];

	if (offset == inode_from_table.d_inode.size)
	{
		/* Increase the file size */
		inode_from_table.d_inode.size += size;
	}

	/*Write data to the file*/
	block_modify(write_data, offset % BLOCK_SIZE, buffer, size); 

	/*Update size and offset*/
	offset += size;
	fs_inode_table[inode].pos = offset;
	fs_inode_table[inode].d_inode.size += size;

	/*Write data to the file*/
	block_modify(ino2blk(inode), 0, &inode_from_table.d_inode, sizeof(struct disk_inode)); // mem_inode.inode_num

	/*Return the number of writen bytes*/
	return size;
}

int fs_lseek(int fd, int offset, int whence)
{
	if (fd < 0 || fd >= MAX_OPEN_FILES)
		return FSE_INVALIDHANDLE; // -6

	/*Get the inode index from the file descriptor table*/
	inode_t inode = current_running->filedes[fd].idx;

	/*Check if the file is opne*/
	if (inode == -1)
		return FSE_NOTEXIST; // -5

	/*Get the current file position from the inode*/
	int curr_pos = fs_inode_table[inode].pos;

	/*Read up inode from disk - To get size*/
	block_read_part(ino2blk(inode), 0, sizeof(struct disk_inode), &fs_inode_table[inode].d_inode);

	/*What is the new file position, using the whence parameter*/
	int new_pos = 0;
	switch (whence)
	{
	case SEEK_SET:
		new_pos = offset;
		break;
	case SEEK_CUR:
		new_pos = curr_pos + offset;
		break;
	case SEEK_END:
		new_pos = fs_inode_table[inode].d_inode.size + offset;
		break;
	default:
		return FSE_INVALIDOFFSET;
	}

	/*Validate the new file pos*/
	if (new_pos < 0)
		return FSE_INVALIDOFFSET; // -7

	/*Update the file position in the indoe*/
	fs_inode_table[inode].pos = new_pos;

	/*Return new offset location in file*/
	return new_pos;
}

int fs_mkdir(char *dirname)
{
	/*Check if the directory exists*/
	inode_t dir_exist = name2inode(dirname);
	if (dir_exist != -1)
		return FSE_EXIST;

	/*Allocate a new indoe for the directory*/
	inode_t dir_inode = get_free_entry((unsigned char *)inode_bmap);
	struct mem_inode dir_mem_inode; 
	if (dir_inode == -1)
		return FSE_INODETABLEFULL;

	/*Set values for the inode - disk*/
	dir_mem_inode.d_inode.type = INTYPE_DIR;
	dir_mem_inode.d_inode.size = sizeof(struct dirent) * 2;
	dir_mem_inode.d_inode.nlinks = 0;

	/*Get a free data block for the directory*/
	int dir_blk = get_free_entry((unsigned char *)dblk_bmap);
	dir_mem_inode.d_inode.direct[0] = idx2blk(dir_blk);

	/*Set values for the inode - mem*/
	dir_mem_inode.open_count = 0;
	dir_mem_inode.pos = 0;
	dir_mem_inode.inode_num = dir_inode;
	dir_mem_inode.dirty = 'F';

	/*Write the new disk_inode to disk - update disk*/
	block_modify(ino2blk(dir_inode), 0, &dir_mem_inode.d_inode, sizeof(struct disk_inode));

	/*Create a new directory entry for the directory and write dir entry ("." and "..") to disk*/
	struct dirent dir_entry[DIRENTS_PER_BLK];
	insert_dir_entry(dir_inode, current_running->cwd, dir_entry); // Correct?
	block_modify(dir_mem_inode.d_inode.direct[0], 0, &dir_entry, sizeof(struct dirent) * DIRENTS_PER_BLK);

	/*Read up current working directory and update size and write down size*/
	block_read_part(ino2blk(current_running->cwd), 0, sizeof(struct disk_inode), &fs_inode_table[current_running->cwd].d_inode);
	fs_inode_table[current_running->cwd].d_inode.size += sizeof(struct dirent);
	block_modify(ino2blk(current_running->cwd), 0, &fs_inode_table[current_running->cwd].d_inode, sizeof(struct disk_inode));

	/*Insert new directory (dirname)*/
	struct dirent new_entry[DIRENTS_PER_BLK];
	block_read_part(fs_inode_table[current_running->cwd].d_inode.direct[0], 0, sizeof(struct dirent) * DIRENTS_PER_BLK, &new_entry);
	insert_entry(dir_inode, new_entry, (char *)dirname);
	block_modify(fs_inode_table[current_running->cwd].d_inode.direct[0], 0, &new_entry, sizeof(struct dirent) * DIRENTS_PER_BLK);

	/*Update bitmap*/
	block_modify(S_DISK_BITMAP_POS, 0, &inode_bmap, sizeof(inode_bmap));
	block_modify(S_DISK_BITMAP_POS, BITMAP_ENTRIES, &dblk_bmap, sizeof(dblk_bmap));

	/*Return success*/
	return FSE_OK;
}

int fs_chdir(char *path)
{
	/*Check if the directory exist*/
	inode_t dir_inode = name2inode(path);
	if (dir_inode == -1)
		return FSE_NOTEXIST;

	/*Update current working directory*/
	current_running->cwd = dir_inode;

	/*Return success*/
	return FSE_OK;
}

int fs_rmdir(char *path)
{
	/*Check if the directory exist*/
	inode_t dir_inode = name2inode(path);
	if (dir_inode == -1)
		return FSE_NOTEXIST;
	if(dir_inode == current_running->cwd)
		return FSE_ERROR;

	/*Read up currect working directory and the dir inode*/
	block_read_part(ino2blk(current_running->cwd), 0, sizeof(struct disk_inode), &fs_inode_table[current_running->cwd].d_inode);
	block_read_part(ino2blk(dir_inode), 0, sizeof(struct disk_inode), &fs_inode_table[dir_inode].d_inode);

	if(fs_inode_table[dir_inode].d_inode.size > sizeof(struct dirent) * 2)
		return FSE_DNOTEMPTY;

	/*Free bitmap - inode and dblk*/
	free_bitmap_entry(dir_inode, (unsigned char *)inode_bmap);
	// free_bitmap_entry(idx2blk(dir_inode), (unsigned char *)dblk_bmap);

	int block_num = fs_inode_table[current_running->cwd].d_inode.direct[0];

	/*Remove directory entry for parent directory*/
	struct dirent parent_entries[DIRENTS_PER_BLK];
	block_read_part(block_num, 0, sizeof(struct dirent) * DIRENTS_PER_BLK, &parent_entries);
	remove_entry(dir_inode, parent_entries);

	/*Update size*/
	fs_inode_table[current_running->cwd].d_inode.size -= sizeof(struct dirent);

	/*Update bitmap*/
	block_modify(S_DISK_BITMAP_POS, 0, &inode_bmap, sizeof(inode_bmap));
	block_modify(S_DISK_BITMAP_POS, BITMAP_ENTRIES, &dblk_bmap, sizeof(dblk_bmap));

	/*Update current working*/
	block_modify(block_num, 0, &parent_entries, sizeof(struct dirent) * DIRENTS_PER_BLK);

	/*Write done current running directory and inode*/
	block_modify(ino2blk(current_running->cwd), 0, &fs_inode_table[current_running->cwd].d_inode, sizeof(struct disk_inode));
	block_modify(ino2blk(dir_inode), 0, &fs_inode_table[dir_inode].d_inode, sizeof(struct disk_inode));

	/*Return success*/
	return FSE_OK;
}

int fs_link(char *linkname, char *filename)
{
	/* Get the inode from the original filename*/
	inode_t original_inode = name2inode(filename);

	/* Check if filename doesn't exist */
	if (original_inode == -1)
		return FSE_NOTEXIST;

	/* Set the new inode equal to the original inode */
	inode_t new_inode =  original_inode; 
	if (new_inode == -1)
		return FSE_INODETABLEFULL; // -21

	/*Read inode up from disk*/
	block_read_part(ino2blk(original_inode), 0, sizeof(struct disk_inode), &fs_inode_table[original_inode].d_inode);

	/* Copy the original inode to the new inode */
	fs_inode_table[new_inode] = fs_inode_table[original_inode];

	/* Update the link count - original inode*/
	fs_inode_table[original_inode].d_inode.nlinks++;

	/* Update inode bitmap, have the same data block*/
	block_modify(S_DISK_BITMAP_POS, 0, &inode_bmap, sizeof(inode_bmap));

	/*Read up current working directory and update size and write down size*/
	block_read_part(ino2blk(current_running->cwd), 0, sizeof(struct disk_inode), &fs_inode_table[current_running->cwd].d_inode);
	fs_inode_table[current_running->cwd].d_inode.size += sizeof(struct dirent);
	block_modify(ino2blk(current_running->cwd), 0, &fs_inode_table[current_running->cwd].d_inode, sizeof(struct disk_inode));

	/*Insert new directory (dirname)*/
	struct dirent dir_entry[DIRENTS_PER_BLK];
	block_read_part(fs_inode_table[current_running->cwd].d_inode.direct[0], 0, sizeof(struct dirent) * DIRENTS_PER_BLK, &dir_entry);
	insert_entry(new_inode, dir_entry, linkname);

	block_modify(fs_inode_table[current_running->cwd].d_inode.direct[0], 0, &dir_entry, sizeof(struct dirent) * DIRENTS_PER_BLK);

	/* Update the original inode */
	block_modify(ino2blk(original_inode), 0, &fs_inode_table[original_inode].d_inode, sizeof(struct disk_inode));

	/* Write the new inode to disk */
	block_modify(ino2blk(new_inode), 0, &fs_inode_table[new_inode].d_inode, sizeof(struct disk_inode));

	return FSE_OK;
}

int fs_unlink(char *linkname)
{
	/* Get the inode of the original file */
	inode_t inode = name2inode(linkname);

	/* Check if filename doesn't exist */
	if (inode == -1)
		return FSE_NOTEXIST; // -5


	/*Read up current working directory and update size and write down size*/
	block_read_part(ino2blk(current_running->cwd), 0, sizeof(struct disk_inode), &fs_inode_table[current_running->cwd].d_inode);

	int block_num = fs_inode_table[current_running->cwd].d_inode.direct[0];

	/*Read up current working directory - to array*/
	struct dirent parent_entries[DIRENTS_PER_BLK];
	block_read_part(block_num, 0, sizeof(struct dirent) * DIRENTS_PER_BLK, &parent_entries);

	/*Remove directory entry for parent directory, and update disk*/
	remove_entry(inode, parent_entries);
	block_modify(block_num, 0, &parent_entries, sizeof(struct dirent) * DIRENTS_PER_BLK);

	/*Update current working size*/
	fs_inode_table[current_running->cwd].d_inode.size -= sizeof(struct dirent);
	block_modify(ino2blk(current_running->cwd), 0, &fs_inode_table[current_running->cwd].d_inode, sizeof(struct disk_inode));

	/* Read up inode from disk and update link count*/
	block_read_part(ino2blk(inode), 0, sizeof(struct disk_inode), &fs_inode_table[inode].d_inode);
	fs_inode_table[inode].d_inode.nlinks--;

	/* Update the inode */
	block_modify(ino2blk(inode), 0, &fs_inode_table[inode].d_inode, sizeof(struct disk_inode));


	/* Check if the link count is 0, if yes done*/
	if (fs_inode_table[inode].d_inode.nlinks >= 0)
		return FSE_OK;


	/*Free bitmap - inode and dblk*/
	free_bitmap_entry(inode, (unsigned char *)inode_bmap);
	// free_bitmap_entry(idx2blk(inode), (unsigned char *)dblk_bmap);

	/*Update bitmap*/
	block_modify(S_DISK_BITMAP_POS, 0, &inode_bmap, sizeof(inode_bmap));
	block_modify(S_DISK_BITMAP_POS, BITMAP_ENTRIES, &dblk_bmap, sizeof(dblk_bmap));

	return FSE_OK;
}

int fs_stat(int fd, char *buffer)
{
	/*Get the inode number from the file descriptor of the current running.*/
	inode_t inode = current_running->filedes[fd].idx;

	/*Read the disk_inode*/
	block_read_part(ino2blk(inode), 0, sizeof(struct disk_inode), &fs_inode_table[inode].d_inode);

	/*Copt the type field (int), the nlinks filed (short), and the size filed (int). 
	* Get the buffer offset from the: shell_sim.c, function void stat (line: 437)*/
	bcopy((const char *)&fs_inode_table[inode].d_inode.type, &buffer[0], sizeof(int));
	bcopy((const char *)&fs_inode_table[inode].d_inode.nlinks, &buffer[1], sizeof(short));
	bcopy((const char *)&fs_inode_table[inode].d_inode.size, &buffer[2], sizeof(int));
	
	/*Retrun success*/
	return FSE_OK;
}

//_______________________________________________________ Small helper function _______________________________________________________________//
/**
 * @brief Return the smallest of the two numbers
 * @param[in] a int number 1
 * @param[in] b int number 2
 * @return int, the smallest of the two numbers
 */
int min(int a, int b)
{
	return (a < b) ? a : b;
}

/**
 * @brief Find the correct data block to a directry index, using the offsett (currect position in file)
 * @param[in] offset position whitin a file
 * @return int, the block number.
 */
int dir_blk(int offset)
{
	int j = offset / BLOCK_SIZE;							 // j is the block number
	j = (j % INODE_NDIRECT + INODE_NDIRECT) % INODE_NDIRECT; // Adjust the index using modulus and restrict to a specific range
	return j;
}

/**
 * @brief Initialize the file descriptor table, by iterating over every entry in the table and set it to unused
 * @param[in] void
 * @return void
 */
void file_descripter_init(void)
{
	for (int i = 0; i < MAX_OPEN_FILES; i++)
	{
		current_running->filedes[i].mode = MODE_UNUSED;
		current_running->filedes[i].idx = -1;
	}
}

/**
 * @brief Initialize a bitmap to 0, by iterating over every entry in the bitmap and set it to 0
 * @param[in] bitmap char pointer to the bitmap (inode or dblk)
 * @return int, FSE_OK if success, FSE_BITMAP if bitmap is larger than BITMAP_ENTRIES
 */
int initialize_bitmap(char *bitmap)
{
	for (int bit = 0; bit < BITMAP_ENTRIES; bit++)
	{
		bitmap[bit] = 0;
	}

	if(bitmap > (char *)BITMAP_ENTRIES)
	    return FSE_BITMAP;
	
	return FSE_OK;
}
/**
 * @brief Insert "." and ".." into a directory
 * @param[in] curr_inode of type inode_t is the current inode.
 * @param[in] parent_inode of type inode_t is the parent inode. 
 * @param[in] array
 * @return void
 */
void insert_dir_entry(inode_t curr_inode, inode_t parent_inode, struct dirent array[DIRENTS_PER_BLK])
{
	for (unsigned int i = 0; i < DIRENTS_PER_BLK; i++)
	{
		array[i].inode = -1;
		array[i].name[0] = '\0';
	}

	/*Inser entires */
	array[0].inode = curr_inode;
	strcpy(array[0].name, ".");

	array[1].inode = parent_inode;
	strcpy(array[1].name, "..");
}
/**
 * @brief Insert a new entry into a directory using strcpy.
 * @param[in] new_index of type inode_t is the inode number of the new entry.
 * @param[in] array of type struct dirent is the array of entries in the directory.
 * @param[in] filename of type char pointer is the name of the new entry.
 * @return void
 */
void insert_entry(inode_t new_index, struct dirent array[DIRENTS_PER_BLK], const char *filename)
{
	for (unsigned int i = 2; i < DIRENTS_PER_BLK; i++)
	{
		if (array[i].inode == -1)
		{
			array[i].inode = new_index;
			strcpy(array[i].name, (char *)filename);
			break;
		}
	}
}
/**
 * @brief Remove a entry from a directory, and left shift any entry in the array to the left.
 * @param[in] inode of type inode_t is the inode number of the entry to be removed.
 * @param[in] array of type struct dirent is the array of entries in the directory.
 * @return void
 */
void remove_entry(inode_t inode, struct dirent array[DIRENTS_PER_BLK])
{
	unsigned int index = 0;
    
    for (unsigned int i = 0; i < DIRENTS_PER_BLK; i++) {
        if (array[i].inode == inode) {
            array[i].inode = -1;
        }
        else // Shift non-matching entries to the left
		{
            array[index++] = array[i];
        }
    }
    
	/*Fill the remaining elements with a default value*/
    for (; index < DIRENTS_PER_BLK; index++) {
        array[index].inode = -1;
    }
}
//_______________________________________________________ Bigger helper function  _______________________________________________________________//
/**
 * @brief Create a root directory, and init all start values to a directory. Pluss insert "." and ".." into root directory
 * @param[in] void
 * @return void
 */
void create_root_directory(void)
{
	/*Allocate new inode - Make root directroy and create a new disk indoe for the root directory*/
	inode_t root_inode_nr = get_free_entry((unsigned char *)inode_bmap);
	struct mem_inode root_mem_inode = fs_inode_table[root_inode_nr];

	/*Set values - disk_inode*/
	root_mem_inode.d_inode.type = INTYPE_DIR;
	root_mem_inode.d_inode.size = (sizeof(struct dirent) * 2); // 2 entries "." and ".." has a size of dirent struct
	root_mem_inode.d_inode.nlinks = 1;
	int index = get_free_entry((unsigned char *)dblk_bmap);
	root_mem_inode.d_inode.direct[0] = idx2blk(index); // get the block number from the index, correct is to write index

	/*Set valus - mem_inode*/
	root_mem_inode.open_count = 0;
	root_mem_inode.pos = 0;
	root_mem_inode.inode_num = root_inode_nr;
	root_mem_inode.dirty = 'F';

	/*Insert parrent ".." and current "." entry*/
	struct dirent entry_array[DIRENTS_PER_BLK];
	insert_dir_entry(root_inode_nr, root_inode_nr, entry_array);

	/*Write the two entires to disk*/
	blknum_t data_blk = get_free_entry((unsigned char *)dblk_bmap);
	root_mem_inode.d_inode.direct[0] = idx2blk(data_blk);
	block_modify(root_mem_inode.d_inode.direct[0], 0, &entry_array, (sizeof(struct dirent) * DIRENTS_PER_BLK)); // correct her is then indx2blk(index) her.

	/*Update bitmap*/
	block_modify(S_DISK_BITMAP_POS, 0, &inode_bmap, sizeof(inode_bmap));
	block_modify(S_DISK_BITMAP_POS, BITMAP_ENTRIES, &dblk_bmap, sizeof(dblk_bmap));

	/*Write to disk*/
	block_modify(ino2blk(root_inode_nr), 0, &root_mem_inode.d_inode, sizeof(struct disk_inode)); // write creatw root directory to disk
}

/**
 * @brief 1. Create a new file in the file system, by making a new entry in file descripter table, 2. allocate new inode from global inode table,
 * 3. allocate nessasery data blocks, 4. Insert entry into the directory, using inode and filename, and 4. retrun file pointer. 
 * @param[in] filename The name of the file to be created
 * @param[in] mode The mode of the file to be created
 * @return  The file descriptor of the created file
 */
int make_file(const char *filename, int mode)
{
	int file_ptr;
	for (file_ptr = 0; file_ptr < MAX_OPEN_FILES; file_ptr++)
	{
		if (current_running->filedes[file_ptr].mode == MODE_UNUSED)
		{
			/*Get index to the inode*/
			int index = get_free_entry((unsigned char *)inode_bmap); 

			/*Set correct mode and get the correct index from the indoe bitmap*/
			current_running->filedes[file_ptr].mode = mode;
			current_running->filedes[file_ptr].idx = index;

			/*Get a free data block from data block bitmap*/
			blknum_t block_number = get_free_entry((unsigned char *)dblk_bmap);

			/*Set correct value - the mem_inode*/
			fs_inode_table[index].open_count++;
			fs_inode_table[index].pos = 0;
			fs_inode_table[index].inode_num = index;
			fs_inode_table[index].dirty = 'F';

			/*Set correct value - disk_inode*/
			fs_inode_table[index].d_inode.type = INTYPE_FILE;
			fs_inode_table[index].d_inode.size = 0;
			fs_inode_table[index].d_inode.nlinks = 0; 
			fs_inode_table[index].d_inode.direct[0] = idx2blk(block_number); // block_mumber, to make it dynamic.

			/*Read up the current working directory*/
			block_read_part(ino2blk(current_running->cwd), 0, sizeof(struct disk_inode), &fs_inode_table[current_running->cwd].d_inode);

			/*Update size*/
			fs_inode_table[current_running->cwd].d_inode.size += sizeof(struct dirent);

			/*Write down size*/
			block_modify(ino2blk(current_running->cwd), 0, &fs_inode_table[current_running->cwd].d_inode, sizeof(struct disk_inode));

			/*Read into array and insert entries*/
			struct dirent dir_entry[DIRENTS_PER_BLK];
			block_read_part(fs_inode_table[current_running->cwd].d_inode.direct[0], 0, sizeof(struct dirent) * DIRENTS_PER_BLK, &dir_entry);
			insert_entry(index, dir_entry, filename);

			/*Read down entries*/
			block_modify(fs_inode_table[current_running->cwd].d_inode.direct[0], 0, &dir_entry, sizeof(struct dirent) * DIRENTS_PER_BLK);

			/*Update bitmap to disk*/
			block_modify(S_DISK_BITMAP_POS, 0, &inode_bmap, sizeof(inode_bmap));
			block_modify(S_DISK_BITMAP_POS, BITMAP_ENTRIES, &dblk_bmap, sizeof(dblk_bmap));

			/*Write new file down to disk*/
			block_modify(ino2blk(index), 0, &fs_inode_table[index].d_inode, sizeof(struct disk_inode));
		}
		break;
	}

	/*Return file pointer index*/
	return file_ptr;
}

//_____________________________________________________________ Helper function given from pre-code ___________________________________________________________//
/**
 * @brief	Search the given bitmap for the first zero bit.
 * If an entry is found it is set to one and the entry number is returned.
 * Returns -1 if all entrys in the bitmap are set.
 * @param[in] bitmap The bitmap to search
 * @return  The entry number of the first zero bit, or -1 if all bits are set
 */
static int get_free_entry(unsigned char *bitmap)
{
	int i;
	/* Seach for a free entry */
	for (i = 0; i < BITMAP_ENTRIES / 8; i++)
	{
		if (bitmap[i] == 0xff) /* All taken */
			continue;
		if ((bitmap[i] & 0x80) == 0)
		{ /* msb */
			bitmap[i] |= 0x80;
			return i * 8;
		}
		else if ((bitmap[i] & 0x40) == 0)
		{
			bitmap[i] |= 0x40;
			return i * 8 + 1;
		}
		else if ((bitmap[i] & 0x20) == 0)
		{
			bitmap[i] |= 0x20;
			return i * 8 + 2;
		}
		else if ((bitmap[i] & 0x10) == 0)
		{
			bitmap[i] |= 0x10;
			return i * 8 + 3;
		}
		else if ((bitmap[i] & 0x08) == 0)
		{
			bitmap[i] |= 0x08;
			return i * 8 + 4;
		}
		else if ((bitmap[i] & 0x04) == 0)
		{
			bitmap[i] |= 0x04;
			return i * 8 + 5;
		}
		else if ((bitmap[i] & 0x02) == 0)
		{
			bitmap[i] |= 0x02;
			return i * 8 + 6;
		}
		else if ((bitmap[i] & 0x01) == 0)
		{ /* lsb */
			bitmap[i] |= 0x01;
			return i * 8 + 7;
		}
	}
	return -1;
}

/**
 * @brief	Free the given bitmap entry. Free a bitmap entry, if the entry is not found -1 is returned, otherwise zero.
 * Note that this function does not check if the bitmap entry was used (freeing
 * an unused entry has no effect).
 * @param[in] entry The entry to free
 * @param[in] bitmap The bitmap to free the entry in
 */
static int free_bitmap_entry(int entry, unsigned char *bitmap)
{
	unsigned char *bme;

	if (entry >= BITMAP_ENTRIES)
		return -1;

	bme = &bitmap[entry / 8];

	switch (entry % 8)
	{
	case 0:
		*bme &= ~0x80;
		break;
	case 1:
		*bme &= ~0x40;
		break;
	case 2:
		*bme &= ~0x20;
		break;
	case 3:
		*bme &= ~0x10;
		break;
	case 4:
		*bme &= ~0x08;
		break;
	case 5:
		*bme &= ~0x04;
		break;
	case 6:
		*bme &= ~0x02;
		break;
	case 7:
		*bme &= ~0x01;
		break;
	}

	return 0;
}

/*
 * ino2blk:
 * Returns the filesystem block (block number relative to the super
 * block) corresponding to the inode number passed.
 */
static blknum_t ino2blk(inode_t ino)
{
	blknum_t blk = (ino + S_INODE_POS);
	return blk;
}

/*
 * idx2blk:
 * Returns the filesystem block (block number relative to the super
 * block) corresponding to the data block index passed.
 */
static blknum_t idx2blk(int index)
{
	blknum_t blk = (index + S_DATA_BLK); // +1
	return blk;
}

/**
 * @brief	Extracts every directory name from a path by replacing every '/' with '\0'. aka. Make a array of /Tmp/dir/her without the '/'
 * @param[in] path The path to parse
 * @param[out] argv The array of directory names
 * @return  The number of directory names found (argc)
 * @note	argv[0] will point to the first character of path
 * @note	IMPORTANT: This function is NOT MY, it comes from the pre-code
 * (Project 6: File System INF-2201: Operating System Fundamentals UiT The Arctic University of Norway) file shell_sim.c line 210 to 228
 */
static int parse_path(char *path, char *argv[MAX_FILENAME_LEN])
{
	char *s = path;
	int argc = 0;

	argv[argc++] = path;

	while (*s != '\0')
	{
		if (*s == '/')
		{
			*s = '\0';
			argv[argc++] = (s + 1);
		}
		s++;
	}
	return argc;
}

/**
 * @brief	Parse a file name in a file in the filesystem and returns the inode number of the file.
 * @param[in] name The name of the file to search for
 * @return  The inode number of the file, -1 if the file was not found
 */
static inode_t name2inode(char *name)
{
	/*Variables*/
	char buffer[MAX_PATH_LEN];
	struct mem_inode parent_inode;
	struct dirent parent_dirent;

	/*Make a tmp variable and copy name into it*/
	char *tmp;
	bcopy(name, buffer, MAX_PATH_LEN);
	char *argv[MAX_FILENAME_LEN];

	/*Check if it is in the current running directory - return cwd*/
	if (same_string(name, "."))
		return current_running->cwd;

	/*check if it is root - return root*/
	if (same_string(name, "/"))
		return 0;

	/*Check if it is an absolute path, if not return buffer*/
	if (name[0] == '/')
		tmp = &buffer[1];
	else
		tmp = buffer;

	/*Parse the file name into directory names*/
	int argc = parse_path(tmp, argv);

	/*Read up current working directory and parent inode - work since I know where cwd are*/
	block_read_part(fs_inode_table[current_running->cwd].d_inode.direct[0], sizeof(struct dirent), sizeof(struct dirent), &parent_dirent);
	block_read_part(ino2blk(parent_dirent.inode), 0, sizeof(struct disk_inode), &parent_inode.d_inode);

	/*Read parent into array*/
	struct dirent parent_entry[DIRENTS_PER_BLK];
	block_read_part(parent_inode.d_inode.direct[0], 0, sizeof(struct dirent) * DIRENTS_PER_BLK, &parent_entry);

	/*Iterate through all entries to sarch after a matching name - retrun if found*/
	for (unsigned int i = 0; i < DIRENTS_PER_BLK; i++)
	{
		if (same_string(parent_entry[i].name, argv[argc - 1]))
		{
			return parent_entry[i].inode;
		}
	}

	/*Check the cwd for a match*/
	struct mem_inode cwd_inode;
	block_read_part(ino2blk(current_running->cwd), 0, sizeof(struct disk_inode), &cwd_inode.d_inode);

	/*Iterates over the direcotry entries of the current working directory and check each entry name for a match*/
	struct dirent cwd_entry[DIRENTS_PER_BLK];
	block_read_part(cwd_inode.d_inode.direct[0], 0, sizeof(struct dirent) * DIRENTS_PER_BLK, &cwd_entry);
	for (unsigned int i = 0; i < DIRENTS_PER_BLK; i++)
	{
		if (same_string(cwd_entry[i].name, name))
		{
			return cwd_entry[i].inode;
		}
	}

	/*If no name is found error*/
	return FSE_ERROR;
}
