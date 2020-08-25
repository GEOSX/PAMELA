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

#include <iostream>

#include "command_parser.hpp"
#include "Parallel/Communicator.hpp"
#include "Mesh/MeshFactory.hpp"
#include "MeshDataWriters/MeshDataWriterFactory.hpp" 

#ifdef WITH_VTK
#include<vtkSmartPointer.h>
#include<vtkMPIController.h>
#endif

int main(int argc, char **argv) {

  using namespace PAMELA;

  Communicator::initialize();
#ifdef WITH_VTK
  vtkSmartPointer<vtkMPIController> controler = vtkMPIController::New();
  controler->Initialize(&argc, &argv, true);
  vtkMultiProcessController::SetGlobalController(controler.Get());
#endif


  args::ArgumentParser parser("PAMELA - PArallel MEsh LibrAry", "");
  args::HelpFlag help(parser, "help", "Display this help menu", { 'h', "help" });
  args::CompletionFlag completion(parser, { "complete" });
  args::ValueFlag<std::string> input(parser, "", "The input mesh", { "input" });
  args::ValueFlag<std::string> output(parser, "", "The output mesh", { "output" });
  args::ValueFlag<std::string> nx(parser, "", "Number of cells in x direction", { "nx" });
  args::ValueFlag<std::string> ny(parser, "", "Number of cells in y direction", { "ny" });
  args::ValueFlag<std::string> nz(parser, "", "Number of cells in z direction", { "nz" });
  args::ValueFlag<std::string> dx(parser, "", "Size of cells in x direction", { "dx" });
  args::ValueFlag<std::string> dy(parser, "", "Size of cells in y direction", { "dy" });
  args::ValueFlag<std::string> dz(parser, "", "Size of cells in z direction", { "dz" });
  parser.ParseCLI(argc, argv);

  if (!output) {
    std::cerr << "No output mesh defined" << std::endl;
    exit(1);
  }

  Mesh* input_mesh;
  if (!input) {
    if (!nx) {
      std::cerr << "If yo don't provide input mesh, you may want to generate "
        << "a structured grid using nx, ny, nz, dz, dy, dz" << std::endl;
    }
    input_mesh = MeshFactory::makeMesh(
        std::stoi(args::get(nx)),
        std::stoi(args::get(ny)),
        std::stoi(args::get(nz)),
        std::stod(args::get(dx)),
        std::stod(args::get(dy)),
        std::stod(args::get(dz)));
  }
  else {
    const std::string input_mesh_filename = args::get(input);
    input_mesh = MeshFactory::makeMesh(input_mesh_filename);
  }

  const std::string output_mesh_filename = args::get(output);

  input_mesh->CreateFacesFromCells();
  input_mesh->PerformPolyhedronPartitioning(
      ELEMENTS::FAMILY::POLYGON, ELEMENTS::FAMILY::POLYGON);
  input_mesh->CreateLineGroupWithAdjacency("TopologicalC2C", input_mesh->getAdjacencySet()->get_TopologicalAdjacency(ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::FAMILY::POLYGON));

  MeshDataWriter* output_mesh = MeshDataWriterFactory::makeWriter(
      input_mesh, output_mesh_filename);

  auto mesh_props = input_mesh->get_PolyhedronProperty_double()->get_PropertyMap();
  for (auto it = mesh_props.begin(); it != mesh_props.end(); ++it)
  {
    output_mesh->DeclareVariable(FAMILY::POLYHEDRON, VARIABLE_DIMENSION::SCALAR, VARIABLE_LOCATION::PER_CELL, it->first);
  }

  output_mesh->DeclareAndSetElementGlobalIndex();
  output_mesh->DeclareAndSetPartitionNumber();
  output_mesh->Init();
  for (auto& mesh_prop : mesh_props)
  {
    output_mesh->SetVariableOnPolyhedron(mesh_prop.first, mesh_prop.second);
  }


  //Dump variables
  output_mesh->Dump();
  Communicator::finalize();


}
