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
#include <fstream>
#include <string>

#include "Mesh/MeshFactory.hpp"
#include "MeshDataWriters/MeshDataWriterFactory.hpp"

namespace PAMELA {
  void save(const std::string& filename_out, Mesh * input_mesh) {

    MeshDataWriter* output_mesh = MeshDataWriterFactory::makeWriter(
        input_mesh, filename_out);

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
  }
  void load_and_save( std::ifstream& input) {
    std::string line;
    while (std::getline(input, line)) {
      const std::string filename_in = std::string(PAMELA_PATH) + "/data/" + line;
      Mesh* input_mesh = MeshFactory::makeMesh(filename_in);
      input_mesh->CreateFacesFromCells();
      input_mesh->PerformPolyhedronPartitioning(
          ELEMENTS::FAMILY::POLYGON, ELEMENTS::FAMILY::POLYGON);
      input_mesh->CreateLineGroupWithAdjacency("TopologicalC2C", input_mesh->getAdjacencySet()->get_TopologicalAdjacency(ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::FAMILY::POLYGON));
      save("vtk_mesh.vtm",input_mesh);
    }
  }
}
