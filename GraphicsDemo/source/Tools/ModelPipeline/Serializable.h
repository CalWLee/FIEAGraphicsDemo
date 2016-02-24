#pragma once
namespace Library
{
	class Serializable
	{
	public:
		Serializable() = default;
		virtual ~Serializable() = default;

	protected:
		virtual void Save(std::ofstream& file) = 0;
	};
}
