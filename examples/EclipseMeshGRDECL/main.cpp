/*
 * ------------------------------------------------------------------------------------------------------------
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 * Copyright (c) 2018-2020 Lawrence Livermore National Security LLC
 * Copyright (c) 2018-2020 The Board of Trustees of the Leland Stanford Junior University
 * Copyright (c) 2018-2020 Total, S.A
 * Copyright (c) 2020-     GEOSX Contributors
 * All rights reserved
 *
 * See top level LICENSE, COPYRIGHT, CONTRIBUTORS, NOTICE, and ACKNOWLEDGEMENTS files for details.
 * ------------------------------------------------------------------------------------------------------------
 */

#include "PAMELA.hpp"
#include "Mesh/MeshFactory.hpp"
#include "Mesh/Mesh.hpp"
#include "Adjacency/Adjacency.hpp"
#include "Parallel/Communicator.hpp"
#include <thread>
#include "MeshDataWriters/MeshDataWriterFactory.hpp"
#ifdef WITH_VTK
#include <vtkMultiProcessController.h>
#include <vtkMPIController.h>
#endif

int main(int argc, char **argv) {

	using namespace  PAMELA;
        utils::pamela_unused(argc);
        utils::pamela_unused(argv);


	Communicator::initialize();
#ifdef WITH_VTK
	vtkSmartPointer<vtkMPIController> controler = vtkMPIController::New();
	controler->Initialize(&argc, &argv, true);
	vtkMultiProcessController::SetGlobalController(controler.Get());
#endif

	//std::this_thread::sleep_for(std::chrono::seconds(10));
	//Mesh* MainMesh = MeshFactory::makeMesh(PAMELA_PATH"/data/eclipse/Johansen/FULLFIELD_IMAXJMAX.GRDECL");
	Mesh* MainMesh = MeshFactory::makeMesh(PAMELA_PATH"/data/DataSets/Norne/Eclipse/IRAP_1005.GRDECL");
	//Mesh* MainMesh = MeshFactory::makeMesh(PAMELA_PATH"/data/eclipse/Dalia/grid.GRDECL");

	MainMesh->CreateFacesFromCells();
	MainMesh->PerformPolyhedronPartitioning(ELEMENTS::FAMILY::POLYGON, ELEMENTS::FAMILY::POLYGON);
	MainMesh->CreateLineGroupWithAdjacency("TopologicalC2C", MainMesh->getAdjacencySet()->get_TopologicalAdjacency(ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::FAMILY::POLYGON));

	////-------------------------Output
#ifdef WITH_VTK
	MeshDataWriter* OutputWriter = MeshDataWriterFactory::makeWriter(MainMesh, "EclipseGrid.vtm");
#else
	MeshDataWriter* OutputWriter = MeshDataWriterFactory::makeWriter(MainMesh, "EclipseGrid.case");
#endif


	//Variable declarations
	
	OutputWriter->DeclareVariable(FAMILY::POLYHEDRON, VARIABLE_DIMENSION::SCALAR, VARIABLE_LOCATION::PER_CELL, "Partition");
	OutputWriter->DeclareVariable(FAMILY::POLYGON, VARIABLE_DIMENSION::SCALAR, VARIABLE_LOCATION::PER_CELL, "Partition");

	auto mesh_props = MainMesh->get_PolyhedronProperty_double()->get_PropertyMap();
	for (auto it = mesh_props.begin(); it != mesh_props.end(); ++it)
	{
		OutputWriter->DeclareVariable(FAMILY::POLYHEDRON, VARIABLE_DIMENSION::SCALAR, VARIABLE_LOCATION::PER_CELL, it->first);
	}

	//
	OutputWriter->DeclareAndSetElementGlobalIndex();
	OutputWriter->DeclareAndSetPartitionNumber();

	//Set
	for (auto& mesh_prop : mesh_props)
	{
		OutputWriter->SetVariableOnPolyhedron(mesh_prop.first, mesh_prop.second);
	}

	//Init
	OutputWriter->Init();

	//Dump
	OutputWriter->Dump();

	Communicator::finalize();

	return 0;
}
