#pragma once
namespace Library
{
	class Loadable
	{
	public:
		Loadable() = default;
		virtual ~Loadable() = default;

		virtual void Load(std::ifstream& file) = 0;
		void ReadString(std::ifstream& file, uint32_t length, std::string& targetString);
	};

}