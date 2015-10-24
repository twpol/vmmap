//------------------------------------------------------------------------------
// http://twpol.dyndns.org/projects/vmmap
// License: New BSD License (BSD).
//------------------------------------------------------------------------------

#pragma once

class process_module
{
private:
	unsigned long long _base;
	unsigned long long _size;
	std::tstring _image_filepath;
public:
	process_module(const unsigned long long base, const unsigned long long size, const std::tstring& image_filepath);
	~process_module(void);
	const unsigned long long base(void) const { return _base; }
	const unsigned long long size(void) const { return _size; }
	const std::tstring image_filepath(void) const { return _image_filepath; }
	operator unsigned long long() const { return _base; }
};
