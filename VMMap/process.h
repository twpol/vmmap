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
	process(DWORD pid);
	~process(void);
	DWORD process_id(void) const { return _process_id; }
	FILETIME creation_time(void) const { return _creation_time; }
	std::tstring image_filepath(void) const { return _image_filepath; }
	std::tstring image_filename(void) const { return _image_filename; }
	std::tstring username(void) const { return _username; }
	operator DWORD(void) const { return _process_id; }
};
