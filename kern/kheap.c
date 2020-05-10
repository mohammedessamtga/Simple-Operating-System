
#include <inc/memlayout.h>
#include <kern/kheap.h>
#include <kern/memory_manager.h>

//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)

#define INT_MAX 2147483647;
#define INT_MIN -2147483648;
int pid;
const int sz = 2000;
struct khprocess{
	uint32 Start_Virtual_Address;
	int num_of_pages;
	uint32 physical_address;
}khp[2000];

struct map
{
	uint32 ph;
	uint32 va;
}m[1024*1024];
int si;
void kheap_virtual_address_helper();
void* kmalloc(unsigned int size)
{
	uint32 v_add = KERNEL_HEAP_START;

	int counter = 0;
	bool found_1 = 0;
	bool found_2 = 0;

	uint32 best_start_address;
	uint32 first_empty_page;
	uint32 best_empty_space = INT_MAX;

	int num_of_pages =size / PAGE_SIZE;
	if(size % PAGE_SIZE > 0) num_of_pages++;

	while(v_add<KERNEL_HEAP_MAX)
	{
		uint32* ptr_page_table;
		get_page_table(ptr_page_directory,(void*)v_add,&ptr_page_table);

		uint32 entery = ptr_page_table[PTX(v_add)];

		if( (entery&PERM_PRESENT) > 0)
		{
			uint32 empty_space = counter*PAGE_SIZE;
			if(empty_space >= size&& empty_space < best_empty_space)
			{
				found_1 = 1;
				best_start_address = first_empty_page;
				best_empty_space = empty_space;
			}
			found_2 = 1;
			counter = 0;
		}
		else
		{
			if(counter == 0)
				first_empty_page=v_add;
			counter++;
		}
		v_add+=PAGE_SIZE;
	}
	uint32 empty_space = counter *PAGE_SIZE;
	if(empty_space >= size && empty_space < best_empty_space)
	{
		if(found_2 == 1)
		{
			found_1 = 1;
			best_start_address = first_empty_page;
			best_empty_space = empty_space;
		}
		else
		{
			found_1 = 1;
			best_start_address = KERNEL_HEAP_START;
		}
	}
	if(found_1)
	{
		int temp = num_of_pages;
		uint32 va = best_start_address;
		while(temp--)
		{
			struct Frame_Info* frame;
			allocate_frame(&frame);
			map_frame(ptr_page_directory,frame,(void*)va,PERM_PRESENT | PERM_WRITEABLE);
			m[si].va = va;
			uint32 * page_table;
			get_page_table(ptr_page_directory,(void*)va,&page_table);
			m[si].ph = page_table[PTX(va)]&0xfffff000;
			si++;
			va+=PAGE_SIZE;

		}
		khp[pid].Start_Virtual_Address = best_start_address;
		khp[pid].num_of_pages = num_of_pages;
		khp[pid].physical_address = kheap_physical_address(best_start_address);


		pid++;

		return (void*)best_start_address;
	}
	return NULL;
}
void kfree(void* virtual_address)
{
	bool found = 0;
	int num_of_pages = 0;
	int target;
	uint32 va =(uint32)virtual_address;
	for(int id = 0 ; id < pid ;id++)
	{
		if(khp[id].Start_Virtual_Address == va)
		{
			found = 1;
			target = id;
			num_of_pages = khp[id].num_of_pages;
			break;
		}
	}
	if(found == 1)
	{
		int j=0;
		uint32 address =va;
		while(j<num_of_pages)
		{
			int k;
			for(int i=0;i<si;++i)
			{
				if(m[i].va == address)
				{
					k=i;
					break;
				}

			}
			for(int i=k;i<si-1;++i)
			{
				m[i]=m[i+1];
			}
			si--;

			unmap_frame(ptr_page_directory,(void*)address);
			address+=PAGE_SIZE;
			j++;
		}
		for(int i= target;i<pid - 1;++i)
		{
			khp[i] = khp[i+1];
		}
		pid--;
	}
	else
	{
		return;
	}

}
unsigned int kheap_virtual_address(unsigned int physical_address)
{
	uint32 offset = physical_address&0x00000fff;
	for(int i=0;i<si;++i)
	{
		if(m[i].ph ==ROUNDDOWN(physical_address,PAGE_SIZE))
		{
			//cprintf("%x %x \n",physical_address,m[i].va);
			return m[i].va + offset;
		}
	}
	return 0;

}
void kheap_virtual_address_helper()
{
	/*for (uint32 i = KERNEL_HEAP_START ; i < KERNEL_HEAP_MAX ; i += PAGE_SIZE)
		{
			uint32 * ptr_table = NULL;
			get_page_table(ptr_page_directory,(void*)i ,&ptr_table);
			if (ptr_table != NULL)
			{
				if((ptr_table[PTX(i)] & PERM_PRESENT) > 0)
				{
					bool found = 0;
					uint32 frame_num = ptr_table[PTX(i)];
					frame_num&=0xFFFFF000;
					for(int j=0;j<si;++j)
					{
						if(frame_num == m[j].frame)
						{
							found = 1;
							break;
						}
					}
					if(!found)
					{
						m[si].frame = frame_num;
						m[si].va = i;
						si++;
					}
					if((ptr_table[PTX(i)] >> 12) == physical_address / PAGE_SIZE)
					{


						return i;
					}
				}
			}
			else
			{
				i += 1024 * 1024 * 4;
				i -= PAGE_SIZE;
			}
		}
		*/
}


unsigned int kheap_physical_address(unsigned int virtual_address)
{
	uint32 * ptr_table;
	get_page_table(ptr_page_directory, (void*)virtual_address, &ptr_table);
	if(ptr_table != NULL)
	{
		uint32 ent = ptr_table[PTX(virtual_address)];
		ent >>= 12;
		ent *= PAGE_SIZE;
		uint32 tmp = virtual_address;
		tmp &= 0x00000fff;
		ent += tmp;
		return ent;
	}
	return 0;
}
//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT 2019 - BONUS2] Kernel Heap Realloc
	// Write your code here, remove the panic and write your code

	return NULL;
	panic("krealloc() is not implemented yet...!!");

}
