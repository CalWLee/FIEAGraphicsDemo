#pragma once
class ModelExporter
{
public:
	static const uint32_t ZERO;
	static void LoadAndExportModel(std::string& filename);
	static void WriteInteger(uint32_t i, std::ofstream& file);
	static void WriteFloat(float f, std::ofstream& file);
};

