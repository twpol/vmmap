//------------------------------------------------------------------------------
// http://twpol.dyndns.org/projects/vmmap
// License: New BSD License (BSD).
//------------------------------------------------------------------------------

#pragma once

class process
{
private:
	// To uniquely identify a process over time, both its PID and start time are
	// required, since PIDs get reused. Note that this pair is NOT unique when
	// persisted.
	DWORD _process_id;
	FILETIME _creation_time;
	std::tstring _image_filepath;
	std::tstring _image_filename;
	std::tstring _username;
public:
	process(void);
	// Create a process wrapper for a given PID. The process' creation time,
	// image file path and file name and owning username are all collected.
	process(const DWORD pid);
	~process(void);
	const DWORD process_id(void) const { return _process_id; }
	const FILETIME creation_time(void) const { return _creation_time; }
	const std::tstring image_filepath(void) const { return _image_filepath; }
	const std::tstring image_filename(void) const { return _image_filename; }
	const std::tstring username(void) const { return _username; }
	operator DWORD(void) const { return _process_id; }
};
