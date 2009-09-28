//------------------------------------------------------------------------------
// http://twpol.dyndns.org/projects/vmmap
// License: Microsoft Public License (Ms-PL).
//------------------------------------------------------------------------------

#pragma once
#include "process.h"
#include "process_module.h"
#include "process_stack.h"
#include "process_heap.h"

typedef unsigned int process_memory_protection;
#define PMP_NONE          0x0000
#define PMP_READ          0x0001
#define PMP_WRITE         0x0002
#define PMP_COPY          0x0004
#define PMP_EXECUTE       0x0008
#define PMP_ACCESS_MASK   0x000F
#define PMP_GUARD         0x0100
#define PMP_NO_CACHE      0x0200
#define PMP_WRITE_COMBINE 0x0400
#define PMP_FLAG_MASK     0x0700

enum process_memory_data_type
{
	PMDT__FIRST = 0,
	PMDT_BASE = PMDT__FIRST,
	PMDT_SIZE,
	PMDT_COMMITTED,
	PMDT_WS_TOTAL,
	PMDT_WS_PRIVATE,
	PMDT_WS_SHAREABLE,
	PMDT_WS_SHARED,
	PMDT_BLOCKS,
	PMDT__LAST
};

enum process_memory_block_type
{
	PMBT__FIRST = 0,
	PMBT_FREE = PMBT__FIRST,
	PMBT_RESERVED,
	PMBT_COMMITTED,
	PMBT__LAST
};

class process_memory;

class process_memory_group;

class process_memory_block
{
private:
	unsigned long long _data[/* process_memory_data_type */PMDT__LAST];
	process_memory_block_type _type;
	process_memory_protection _protection;
	std::tstring _details;
public:
	process_memory_block(const process_memory& memory, const HANDLE hProcess, process_memory_group& group, const PMEMORY_BASIC_INFORMATION info);
	~process_memory_block(void);

	unsigned long long data(process_memory_data_type type) const;
#define DATA_ACCESS_PMB(name, key) unsigned long long name(void) const { return _data[PMDT_ ## key]; }
	DATA_ACCESS_PMB(base, BASE);
	DATA_ACCESS_PMB(size, SIZE);
	DATA_ACCESS_PMB(committed, COMMITTED);
	DATA_ACCESS_PMB(ws, WS_TOTAL);
	DATA_ACCESS_PMB(ws_private, WS_PRIVATE);
	DATA_ACCESS_PMB(ws_shareable, WS_SHAREABLE);
	DATA_ACCESS_PMB(ws_shared, WS_SHARED);
	DATA_ACCESS_PMB(blocks, BLOCKS);

	process_memory_block_type type(void) const { return _type; }
	process_memory_protection protection(void) const { return _protection; }
	std::tstring protection_str(void) const;
	const std::tstring details(void) const { return _details; }
	void details(const std::tstring details) { _details = details; }

	void add_ws_page(PSAPI_WORKING_SET_BLOCK* ws_block, unsigned long page_size);
	operator unsigned long long(void) const { return _data[PMDT_BASE]; }
};

enum process_memory_group_type
{
	PMGT__FIRST = 0,
	PMGT_TOTAL = PMGT__FIRST,
	PMGT_IMAGE,
	PMGT_PRIVATE,
	PMGT_SHAREABLE,
	PMGT_MAPPED_FILE,
	PMGT_HEAP,
	PMGT_STACK,
	PMGT_FREE,
	PMGT__LAST
};

class process_memory_group
{
private:
	process_memory_group_type _type;
	std::tstring _details;
	std::list<const process_memory_block> _blocks;
public:
	process_memory_group(void);
	process_memory_group(const process_memory& memory, const HANDLE hProcess, const PMEMORY_BASIC_INFORMATION info);
	~process_memory_group(void);

	unsigned long long data(process_memory_data_type type) const;
#define DATA_ACCESS_PMG(name, key) unsigned long long name(void) const { return data(PMDT_ ## key); }
	DATA_ACCESS_PMG(base, BASE);
	DATA_ACCESS_PMG(size, SIZE);
	DATA_ACCESS_PMG(committed, COMMITTED);
	DATA_ACCESS_PMG(ws, WS_TOTAL);
	DATA_ACCESS_PMG(ws_private, WS_PRIVATE);
	DATA_ACCESS_PMG(ws_shareable, WS_SHAREABLE);
	DATA_ACCESS_PMG(ws_shared, WS_SHARED);
	DATA_ACCESS_PMG(blocks, BLOCKS);

	process_memory_group_type type(void) const { return _type; }
	void type(process_memory_group_type type) { _type = type; }
	process_memory_protection protection(void) const;
	std::tstring protection_str(void) const;
	const std::tstring details(void) const { return _details; }
	void details(const std::tstring details) { _details = details; }

	const std::list<const process_memory_block>& block_list(void) const { return _blocks; }
	void add_block(const process_memory_block& block);
	operator unsigned long long(void) const { return data(PMDT_BASE); }
};

class process_memory
{
private:
	const process& _process;
	std::list<const process_module> _modules;
	std::list<const process_heap> _heaps;
	std::list<const process_stack> _stacks;
	std::map<unsigned long long, process_memory_group> _groups;
	process_memory operator=(const process_memory& self);
public:
	process_memory(const process& process);
	~process_memory(void);
	const std::list<const process_module>& modules(void) const { return _modules; }
	const std::list<const process_heap>& heaps(void) const { return _heaps; }
	const std::list<const process_stack>& stacks(void) const { return _stacks; }
	const std::map<unsigned long long, process_memory_group>& groups(void) const { return _groups; }

	unsigned long long data(process_memory_group_type group_type, process_memory_data_type type) const;
};

//struct PEB 
//{ 
//	BOOLEAN InheritedAddressSpace; // 00 
//	BOOLEAN ReadImageFileExecOptions; // 01 
//	BOOLEAN BeingDebugged; // 02 
//	BOOLEAN SpareBool; // 03 
//	HANDLE Mutant; // 04 
//	HMODULE ImageBaseAddress; // 08 
//	PVOID LdrData; // 0c 
//	PVOID ProcessParameters; // 10 
//	PVOID SubSystemData; // 14 
//	HANDLE ProcessHeap; // 18 
//	PVOID FastPebLock; // 1c 
//	PVOID /*PPEBLOCKROUTI*/ FastPebLockRoutine; // 20 
//	PVOID /*PPEBLOCKROUTI*/ FastPebUnlockRoutine; // 24 
//	ULONG EnvironmentUpdateCount; // 28 
//	PVOID KernelCallbackTable; // 2c 
//	PVOID EventLogSection; // 30 
//	PVOID EventLog; // 34 
//	PVOID /*PPEB_FREE_BLO*/ FreeList; // 38 
//	ULONG TlsExpansionCounter; // 3c 
//	PVOID TlsBitmap; // 40 
//	ULONG TlsBitmapBits[2]; // 44 
//	PVOID ReadOnlySharedMemoryBase; // 4c 
//	PVOID ReadOnlySharedMemoryHeap; // 50 
//	PVOID *ReadOnlyStaticServerData; // 54 
//	PVOID AnsiCodePageData; // 58 
//	PVOID OemCodePageData; // 5c 
//	PVOID UnicodeCaseTableData; // 60 
//	ULONG NumberOfProcessors; // 64 
//	ULONG NtGlobalFlag; // 68 
//	BYTE Spare2[4]; // 6c 
//	LARGE_INTEGER CriticalSectionTimeout; // 70 
//	ULONG HeapSegmentReserve; // 78 
//	ULONG HeapSegmentCommit; // 7c 
//	ULONG HeapDeCommitTotalFreeTh; // 80 
//	ULONG HeapDeCommitFreeBlockTh; // 84 
//	ULONG NumberOfHeaps; // 88 
//	ULONG MaximumNumberOfHeaps; // 8c 
//	PVOID *ProcessHeaps; // 90 
//	PVOID GdiSharedHandleTable; // 94 
//	PVOID ProcessStarterHelper; // 98 
//	PVOID GdiDCAttributeList; // 9c 
//	PVOID LoaderLock; // a0 
//	ULONG OSMajorVersion; // a4 
//	ULONG OSMinorVersion; // a8 
//	ULONG OSBuildNumber; // ac 
//	ULONG OSPlatformId; // b0 
//	ULONG ImageSubSystem; // b4 
//	ULONG ImageSubSystemMajorVersion; // b8 
//	ULONG ImageSubSystemMinorVersion; // bc 
//	ULONG ImageProcessAffinityMask; // c0 
//	ULONG GdiHandleBuffer[34]; // c4 
//	ULONG PostProcessInitRoutine; // 14c 
//	PVOID TlsExpansionBitmap; // 150 
//	ULONG TlsExpansionBitmapBits[32]; // 154 
//	ULONG SessionId; // 1d4 
//};
