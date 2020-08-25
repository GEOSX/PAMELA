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

#pragma once
#include <vector>
#include "Mesh/UnstructuredMesh.hpp"
#include <fstream>
namespace PAMELA
{

  enum class GMSH_MESH_TYPE {NODE=0, LINE=1, TRIANGLE=2,QUADRANGLE=3,TETRAHEDRON=4,HEXAHEDRON=5, PRISM=6, PYRAMID=7,POINT=15};

  class Gmsh_mesh
  {

    public:
      Gmsh_mesh() = default;
      Mesh* CreateMesh(const std::string file_path);

    private:
      std::string m_label {""};
      int m_nphysicalregions {0};
      int m_dimension {0};
      int m_nelements {0};
      int m_nnodes {0};
      int m_ntriangles {0};
      int m_nquadrangles {0};
      int m_ntetrahedra {0};
      int m_nhexahedra {0};
      int m_nprisms {0};
      int m_npyramids {0};
      int m_npoints {0};

      std::unordered_map<int, std::string> m_TagNamePolygon {};
      std::unordered_map<int, std::string> m_TagNamePolyhedron {};

      void InitElementsMapping();


  };

}
