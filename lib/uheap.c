#include <inc/lib.h>

#define INT_MAX 2147483647;
#define INT_MIN -2147483648;

void malloc_helper();
struct map
{
	uint32 address;
	int num_of_pages;
	bool  persent;
}mp[(USER_HEAP_MAX-USER_HEAP_START)/PAGE_SIZE];
int sz =0;
bool helper_call=0;
// malloc()
//	This function use BEST FIT strategy to allocate space in heap
//  with the given size and return void pointer to the start of the allocated space

//	To do this, we need to switch to the kernel, allocate the required space
//	in Page File then switch back to the user again.
//
//	We can use sys_allocateMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls allocateMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the allocateMem function is empty, make sure to implement it.


//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

void* malloc(uint32 size)
{
	if(helper_call == 0)
	{
		malloc_helper();
		helper_call = 1;
	}
	//TODO: [PROJECT 2019 - MS2 - [5] User Heap] malloc() [User Side]
	// Write your code here, remove the panic and write your code
	//panic("malloc() is not implemented yet...!!");

	// Steps:
	//	1) Implement BEST FIT strategy to search the heap for suitable space
	//		to the required allocation size (space should be on 4 KB BOUNDARY)
	//	2) if no suitable space found, return NULL
	//	 Else,
	//	3) Call sys_allocateMem to invoke the Kernel for allocation
	// 	4) Return pointer containing the virtual address of allocated space,
	//
	//This function should find the space of the required range
	// ******** ON 4KB BOUNDARY ******************* //

	//Use sys_isUHeapPlacementStrategyBESTFIT() to check the current strategy

	//change this "return" according to your answer

	int counter = 0;
	uint32 best_add;
	uint32 first_empty_page;
	uint32 best_space = INT_MAX;
	bool found_1 = 0;
	bool found_2 = 0;
	for(int i=0;i<sz;++i)
	{
		if(mp[i].persent != 0)
		{
			uint32 empty_space = counter * PAGE_SIZE;
			if(empty_space >= size&&best_space > empty_space)
			{
				found_1 = 1;
				best_space = empty_space;
				best_add = first_empty_page;
			}
			found_2 = 1;
			counter=0;
		}
		else
		{
			if(counter == 0) first_empty_page = mp[i].address;
			counter++;
		}
	}
	uint32 empty_space = counter *PAGE_SIZE;
	if(empty_space >= size && empty_space < best_space)
	{
		best_add=first_empty_page;
		found_1=1;
	}
	if(found_1 == 0)
	{
		return NULL;
	}
	int num_of_page=size/PAGE_SIZE;
	if (size%PAGE_SIZE!=0)
		num_of_page++;
	int tempi;
	for (int i=0;i<sz;i++)
	{
		if (mp[i].address==best_add)
		{
			tempi=i;
			break;
		}
	}
	mp[tempi].num_of_pages = num_of_page;
	while (num_of_page--)
	{
		mp[tempi].persent=1;
		tempi++;
	}
	sys_allocateMem(best_add,size);

	return (void *)best_add;

	return 0;
}
void malloc_helper()
{
	for(uint32 va = USER_HEAP_START;va<USER_HEAP_MAX;va+=PAGE_SIZE)
	{
		mp[sz].address = va;
		mp[sz++].persent = 0;
	}
}

void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	if(helper_call == 0)
	{
		malloc_helper();
		helper_call = 1;
	}
	//TODO: [PROJECT 2019 - MS2 - [6] Shared Variables: Creation] smalloc() [User Side]
	// Write your code here, remove the panic and write your code
	//panic("smalloc() is not implemented yet...!!");

	// Steps:
	//	1) Implement BEST FIT strategy to search the heap for suitable space
	//		to the required allocation size (space should be on 4 KB BOUNDARY)
	//	2) if no suitable space found, return NULL
	//	 Else,
	//	3) Call sys_createSharedObject(...) to invoke the Kernel for allocation of shared variable
	//		sys_createSharedObject(): if succeed, it returns the ID of the created variable. Else, it returns -ve
	//	4) If the Kernel successfully creates the shared variable, return its virtual address
	//	   Else, return NULL

	//This function should find the space of the required range
	// ******** ON 4KB BOUNDARY ******************* //

	//Use sys_isUHeapPlacementStrategyBESTFIT() to check the current strategy

	//change this "return" according to your answer
	if (sys_isUHeapPlacementStrategyBESTFIT()==1)
	{
		int counter = 0;
		uint32 best_add;
		uint32 first_empty_page;
		uint32 best_space = INT_MAX;
		bool found_1 = 0;
		bool found_2 = 0;
		for(int i=0;i<sz;++i)
		{
			if(mp[i].persent != 0)
			{
				uint32 empty_space = counter * PAGE_SIZE;
				if(empty_space >= size&&best_space > empty_space)
				{
					found_1 = 1;
					best_space = empty_space;
					best_add = first_empty_page;
				}
				found_2 = 1;
				counter=0;
			}
			else
			{
				if(counter == 0) first_empty_page = mp[i].address;
				counter++;
			}
		}
		uint32 empty_space = counter *PAGE_SIZE;
		if(empty_space >= size && empty_space < best_space)
		{
			best_add=first_empty_page;
			found_1=1;
		}
		if(found_1 == 0)
		{
			return NULL;
		}
		int num_of_page=size/PAGE_SIZE;
		if (size%PAGE_SIZE!=0)
			num_of_page++;
		int tempi;
		for (int i=0;i<sz;i++)
		{
			if (mp[i].address==best_add)
			{
				tempi=i;
				break;
			}
		}
		mp[tempi].num_of_pages = num_of_page;
		while (num_of_page--)
		{
			mp[tempi].persent=1;
			tempi++;
		}
		int num=sys_createSharedObject(sharedVarName,size,isWritable,(void*)best_add);

		if(num >= 0)
			return (void *)best_add;
		else
			return NULL;
	}
	return 0;
}

void* sget(int32 ownerEnvID, char *sharedVarName)
{
	//TODO: [PROJECT 2019 - MS2 - [6] Shared Variables: Get] sget() [User Side]
	// Write your code here, remove the panic and write your code
	//panic("sget() is not implemented yet...!!");

	// Steps:
	//	1) Get the size of the shared variable (use sys_getSizeOfSharedObject())
	//	2) If not exists, return NULL
	//	3) Implement BEST FIT strategy to search the heap for suitable space
	//		to share the variable (should be on 4 KB BOUNDARY)
	//	4) if no suitable space found, return NULL
	//	 Else,
	//	5) Call sys_getSharedObject(...) to invoke the Kernel for sharing this variable
	//		sys_getSharedObject(): if succeed, it returns the ID of the shared variable. Else, it returns -ve
	//	6) If the Kernel successfully share the variable, return its virtual address
	//	   Else, return NULL
	//

	//This function should find the space for sharing the variable
	// ******** ON 4KB BOUNDARY ******************* //

	//Use sys_isUHeapPlacementStrategyBESTFIT() to check the current strategy

	//change this "return" according to your answer

	if(helper_call == 0)
	{
		malloc_helper();
		helper_call = 1;
	}
	int size =sys_getSizeOfSharedObject(ownerEnvID,sharedVarName);
	if(size==E_SHARED_MEM_NOT_EXISTS)
	{
		return NULL;
	}
	int num_of_page=size/PAGE_SIZE;
	if (size%PAGE_SIZE!=0)
		num_of_page++;
	int counter = 0;
		uint32 best_add;
		uint32 first_empty_page;
		uint32 best_space = INT_MAX;
		bool found_1 = 0;
		bool found_2 = 0;
		for(int i=0;i<sz;++i)
		{
			if(mp[i].persent != 0)
			{
				uint32 empty_space = counter * PAGE_SIZE;
				if(empty_space >= size&&best_space > empty_space)
				{
					found_1 = 1;
					best_space = empty_space;
					best_add = first_empty_page;
				}
				found_2 = 1;
				counter=0;
			}
			else
			{
				if(counter == 0) first_empty_page = mp[i].address;
				counter++;
			}
		}
		uint32 empty_space = counter *PAGE_SIZE;
		if(empty_space >= size && empty_space < best_space)
		{
			best_add=first_empty_page;
			found_1=1;
		}
		if(found_1 == 0)
		{
			return NULL;
		}
		//int num_of_page=size/PAGE_SIZE;
		//if (size%PAGE_SIZE!=0)
			//num_of_page++;
		int tempi;
		for (int i=0;i<sz;i++)
		{
			if (mp[i].address==best_add)
			{
				tempi=i;
				break;
			}
		}
		mp[tempi].num_of_pages = num_of_page;
		while (num_of_page--)
		{
			mp[tempi].persent=1;
			tempi++;
		}
		int id = sys_getSharedObject(ownerEnvID,sharedVarName,(void*)best_add);
		if (id>=0)
			return(void*) best_add;

			return NULL;
}

// free():
//	This function frees the allocation of the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from page file and main memory then switch back to the user again.
//
//	We can use sys_freeMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls freeMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the freeMem function is empty, make sure to implement it.

void free(void* virtual_address)
{
	//TODO: [PROJECT 2019 - MS2 - [5] User Heap] free() [User Side]
	// Write your code here, remove the panic and write your code
	//panic("free() is not implemented yet...!!");

	//you should get the size of the given allocation using its address
	//you need to call sys_freeMem()
	//refer to the project presentation and documentation for details
	int idx;
	for(int i=0;i<sz;++i)
	{
		if((void *)mp[i].address == virtual_address)
		{
			idx = i;
			break;
		}
	}
	int num_of_pages = mp[idx].num_of_pages;
	int savepages= mp[idx].num_of_pages;
	uint32 va = mp[idx].address;
	while(num_of_pages--)
	{
		mp[idx].persent=0;
		idx++;
	}
	sys_freeMem(va, savepages);
}


//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//=============
// [1] sfree():
//=============
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
	//TODO: [PROJECT 2019 - BONUS4] Free Shared Variable [User Side]
	// Write your code here, remove the panic and write your code
	panic("sfree() is not implemented yet...!!");

	//	1) you should find the ID of the shared variable at the given address
	//	2) you need to call sys_freeSharedObject()

}


//===============
// [2] realloc():
//===============

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_moveMem(uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		which switches to the kernel mode, calls moveMem(struct Env* e, uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		in "memory_manager.c", then switch back to the user mode here
//	the moveMem function is empty, make sure to implement it.

void *realloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT 2019 - BONUS3] User Heap Realloc [User Side]
	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");

}
