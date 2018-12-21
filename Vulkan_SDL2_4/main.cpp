#include"Demo.h"

int main()
{
	Demo app;
	try
	{
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

//#include"Model.h"
//#include<iostream>
//int main()
//{
//	ModelImporter importer;
//	importer.initialize();
//	Model* model = importer.loadModel("Model/test.fbx");
//	std::cout << model->getMeshCount() << std::endl;
//	for (int i = 0; i < model->getMeshCount(); ++i)
//	{
//		std::cout << "Triangle : " << model->getBuffer()[i].getTriangleCount() << std::endl;
//		std::cout << "Vertices : " << model->getBuffer()[i].getVertexCount() << std::endl;
//	}
//	delete model;
//	return 0;
//}