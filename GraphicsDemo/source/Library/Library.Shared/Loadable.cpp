#include "pch.h"
#include "Loadable.h"

namespace Library
{
	void Loadable::ReadString(std::ifstream& file, uint32_t length, std::string& targetString)
	{
		std::vector<char> buffer;
		char bufferchar;

		for (uint32_t i = 0; i < length; ++i)
		{
			file.read(&bufferchar, sizeof(char));
			buffer.push_back(bufferchar);
		}

		targetString = std::string(buffer.begin(), buffer.end());
	}
}