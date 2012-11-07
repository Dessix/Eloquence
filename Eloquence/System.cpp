#include "System.hpp"
#include <string>
#include <iostream>
#include <stdio.h>
#include <crtdbg.h>
#include <memory>
#include <direct.h>

std::string SystemInterface::exec( const char* cmd )
{
	FILE* pipe = _popen(cmd, "r");
	if (!pipe) return "ERROR";
	char buffer[128];
	std::string result = "";
	while(!feof(pipe)) {
		if(fgets(buffer, 128, pipe) != NULL)
			result += buffer;
	}
	_pclose(pipe);
	return result;
}

std::string SystemInterface::cwd()
{
	const size_t chunkSize=255;
	const int maxChunks=10240; // 2550 KiBs of current path are more than enough

	char stackBuffer[chunkSize]; // Stack buffer for the "normal" case
	if(_getcwd(stackBuffer,sizeof(stackBuffer))!=NULL)
		return stackBuffer;
	if(errno!=ERANGE)
	{
		// It's not ERANGE, so we don't know how to handle it
		throw std::runtime_error("Cannot determine the current path.");
		// Of course you may choose a different error reporting method
	}
	// Ok, the stack buffer isn't long enough; fallback to heap allocation
	for(int chunks=2; chunks<maxChunks ; chunks++)
	{
		// With boost use scoped_ptr; in C++0x, use unique_ptr
		// If you want to be less C++ but more efficient you may want to use realloc
		std::auto_ptr<char> cwd(new char[chunkSize*chunks]); 
		if(_getcwd(cwd.get(),chunkSize*chunks)!=NULL)
			return cwd.get();
		if(errno!=ERANGE)
		{
			// It's not ERANGE, so we don't know how to handle it
			throw std::runtime_error("Cannot determine the current path.");
			// Of course you may choose a different error reporting method
		}   
	}
	throw std::runtime_error("Cannot determine the current path; the path is apparently unreasonably long");
}

bool SystemInterface::setcwd(std::string path)
{
	return _chdir(path.c_str()) == 0;
}
