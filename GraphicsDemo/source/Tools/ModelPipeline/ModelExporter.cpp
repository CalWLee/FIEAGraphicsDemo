#include "stdafx.h"
#include <Shlwapi.h>
#include "ModelExporter.h"

using namespace Library;
using namespace Game;

const uint32_t ModelExporter::ZERO = 0;

void ModelExporter::LoadAndExportModel(std::string& filename)
{

	// Load the model
	std::unique_ptr<Pipeline::Model> model(new Pipeline::Model(filename, true));
}

void ModelExporter::WriteInteger(uint32_t i, std::ofstream& file)
{
	char* numberToWrite = (char*)&i;
	if (i == 0)
	{
		numberToWrite = (char*)(&ZERO);
	}
	file.write(numberToWrite, sizeof(uint32_t));
}

void ModelExporter::WriteFloat(float f, std::ofstream& file)
{
	char* numberToWrite = (char*) &f;
	if (f == 0)
	{
		numberToWrite = (char*)(&ZERO);
	}
	file.write(numberToWrite, sizeof(float));
}