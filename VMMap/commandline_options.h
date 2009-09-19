#pragma once

class commandline_options
{
private:
	std::map<const std::tstring, std::list<const std::tstring> > _arguments;
	std::list<const std::tstring> _files;
public:
	static const std::tstring nopt;
	commandline_options(const std::tstring);
	~commandline_options(void) {}
	const bool has(const std::tstring);
	const std::tstring& get(const std::tstring);
	const std::list<const std::tstring>& gets(const std::tstring);
	const std::list<const std::tstring>& files(void) const { return _files; }
};
