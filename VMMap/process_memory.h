//------------------------------------------------------------------------------
// http://twpol.dyndns.org/projects/vmmap
// License: New BSD License (BSD).
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
	PMDT_LARGEST,
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
	process_memory_block(const process_memory_block_type type, const unsigned long long base, const unsigned long long size);
	~process_memory_block(void);

	const unsigned long long data(const process_memory_data_type type) const;
#define DATA_ACCESS_PMB(name, key) const unsigned long long name(void) const { return _data[PMDT_ ## key]; }
	DATA_ACCESS_PMB(base, BASE);
	DATA_ACCESS_PMB(size, SIZE);
	DATA_ACCESS_PMB(committed, COMMITTED);
	DATA_ACCESS_PMB(ws, WS_TOTAL);
	DATA_ACCESS_PMB(ws_private, WS_PRIVATE);
	DATA_ACCESS_PMB(ws_shareable, WS_SHAREABLE);
	DATA_ACCESS_PMB(ws_shared, WS_SHARED);
	DATA_ACCESS_PMB(blocks, BLOCKS);
	DATA_ACCESS_PMB(largest, LARGEST);

	const process_memory_block_type type(void) const { return _type; }
	const process_memory_protection protection(void) const { return _protection; }
	const std::tstring protection_str(void) const;
	const std::tstring details(void) const { return _details; }
	const void details(const std::tstring details) { _details = details; }

	const void add_ws_page(const PSAPI_WORKING_SET_BLOCK* ws_block, const unsigned long page_size);
	operator unsigned long long(void) const { return _data[PMDT_BASE]; }
};

enum process_memory_group_type
{
	PMGT__FIRST = 0,
	PMGT_TOTAL = PMGT__FIRST,
	PMGT_IMAGE,
	PMGT_MAPPED_FILE,
	PMGT_SHAREABLE,
	PMGT_HEAP,
	PMGT_STACK,
	PMGT_PRIVATE,
	PMGT_UNUSABLE,
	PMGT_FREE,
	PMGT__LAST
};

class process_memory_group
{
private:
	process_memory_group_type _type;
	std::tstring _details;
	std::list<process_memory_block> _blocks;
public:
	process_memory_group(void);
	process_memory_group(const process_memory& memory, const HANDLE hProcess, const PMEMORY_BASIC_INFORMATION info);
	process_memory_group(const process_memory_group_type type, const unsigned long long base, const unsigned long long size);
	~process_memory_group(void);

	const unsigned long long data(const process_memory_data_type type) const;
#define DATA_ACCESS_PMG(name, key) const unsigned long long name(void) const { return data(PMDT_ ## key); }
	DATA_ACCESS_PMG(base, BASE);
	DATA_ACCESS_PMG(size, SIZE);
	DATA_ACCESS_PMG(committed, COMMITTED);
	DATA_ACCESS_PMG(ws, WS_TOTAL);
	DATA_ACCESS_PMG(ws_private, WS_PRIVATE);
	DATA_ACCESS_PMG(ws_shareable, WS_SHAREABLE);
	DATA_ACCESS_PMG(ws_shared, WS_SHARED);
	DATA_ACCESS_PMG(blocks, BLOCKS);
	DATA_ACCESS_PMG(largest, LARGEST);

	const process_memory_group_type type(void) const { return _type; }
	const void type(const process_memory_group_type type) { _type = type; }
	const process_memory_protection protection(void) const;
	const std::tstring protection_str(void) const;
	const std::tstring details(void) const { return _details; }
	const void details(const std::tstring details) { _details = details; }

	const std::list<process_memory_block>& block_list(void) const { return _blocks; }
	const void add_block(const process_memory_block& block);
	operator unsigned long long(void) const { return data(PMDT_BASE); }
};

class process_memory
{
private:
	const process& _process;
	std::list<process_module> _modules;
	std::list<process_heap> _heaps;
	std::list<process_stack> _stacks;
	std::map<unsigned long long, process_memory_group> _groups;
	process_memory operator=(const process_memory& self);
	void enable_privilege(const std::tstring privilege_name);
	void disable_privilege(const std::tstring privilege_name);
public:
	// Create and collect memory information about a given process. All
	// collection happens immediately.
	process_memory(const process& process);
	~process_memory(void);
	// Returns the process.
	const DWORD process_id(void) const { return _process.process_id(); }
	// Returns a list of modules (mapped code) found.
	const std::list<process_module>& modules(void) const { return _modules; }
	// Returns a list of heaps (private memory) found.
	const std::list<process_heap>& heaps(void) const { return _heaps; }
	// Returns a list of stacks (per-thread private memmory) found.
	const std::list<process_stack>& stacks(void) const { return _stacks; }
	// Returns a map of memory groups (by type).
	const std::map<unsigned long long, process_memory_group>& groups(void) const { return _groups; }
	// Returns specific memory data for the whole process.
	const unsigned long long data(const process_memory_group_type group_type, const process_memory_data_type type) const;
};
