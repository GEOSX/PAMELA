#include "PAMELA.hpp"
#include "Mesh/MeshFactory.hpp"
#include "Mesh/Mesh.hpp"
#include "Parallel/Communicator.hpp"
#include <thread>
#include "MeshDataWriters/MeshDataWriterFactory.hpp"

#ifdef WITH_VTK
#include<vtkSmartPointer.h>
#include<vtkMPIController.h>
#endif

int main(int argc, char **argv)
{

	using namespace PAMELA;


	Communicator::initialize();

#ifdef WITH_VTK
	vtkSmartPointer<vtkMPIController> controler = vtkMPIController::New();
	controler->Initialize(&argc, &argv, true);
	vtkMultiProcessController::SetGlobalController(controler.Get());
#endif

	//std::this_thread::sleep_for(std::chrono::seconds(10));

	//Make mesh
	Mesh* MainMesh = MeshFactory::makeMesh(6, 5, 3, 1./5, 1./3, 1./4);
	MainMesh->CreateFacesFromCells();

	//Distort Mesh
	MainMesh->Distort(0.0);
	
	//Partitioning
	MainMesh->PerformPolyhedronPartitioning(ELEMENTS::FAMILY::POLYGON, ELEMENTS::FAMILY::POLYGON);


	//Output
	//--Create 
	MeshDataWriter* OutputWriter = MeshDataWriterFactory::makeWriter(MainMesh, "Cartesian.vtm");

	//--Create variables
	OutputWriter->DeclareVariable(FAMILY::POLYHEDRON, VARIABLE_TYPE::SCALAR, VARIABLE_LOCATION::PER_CELL, "Partition");
	OutputWriter->DeclareVariable(FAMILY::POLYGON, VARIABLE_TYPE::SCALAR, VARIABLE_LOCATION::PER_CELL, "Partition");
	OutputWriter->DeclareVariable(FAMILY::POLYHEDRON, VARIABLE_TYPE::SCALAR, VARIABLE_LOCATION::PER_CELL,"Pressure");
	OutputWriter->DeclareVariable(FAMILY::POLYHEDRON, VARIABLE_TYPE::SCALAR, VARIABLE_LOCATION::PER_NODE, "Test");

	//
	OutputWriter->DeclareAdjacency("Volume to Volume", MainMesh->getMeshAdjacency()->get_Adjacency(ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::FAMILY::POLYGON));

	//--Make files
	OutputWriter->Init();

	//--Set variables
	//Set variable values
	OutputWriter->SetVariable("Pressure", 55);
	OutputWriter->SetVariable("Test", 0.1);
	OutputWriter->SetVariable("Partition", Communicator::worldRank());
	
	//Dump variables
	OutputWriter->Dump();

	Communicator::finalize();

	return 0;
}


