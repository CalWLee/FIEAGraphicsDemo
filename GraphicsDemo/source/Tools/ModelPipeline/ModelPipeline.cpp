// ModelPipeline.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ModelExporter.h"

int _tmain(int argc, CHAR* argv[])
{
	std::string input;

	std::cout << "Input name of model with extension to be converted: ";
	std::cin >> input;
	try
	{
		ModelExporter::LoadAndExportModel(input);
	}
	catch (std::exception)
	{
		std::cout << "Conversion Failed." << std::endl;
		return 1;
	}
	std::cout << "Converted Model." << std::endl;
	
	return 0;
}

