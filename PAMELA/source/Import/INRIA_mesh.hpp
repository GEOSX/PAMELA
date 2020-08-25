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

  enum class INRIA_MESH_TYPE { VERTEX, TRIANGLE, EDGE, QUADRILATERAL, TETRAHEDRON, PENTRAHEDRON, HEXAHEDRON };

  class INRIA_mesh
  {

    public:
      INRIA_mesh() = default;
      Mesh* CreateMesh(const std::string file_path);

    private:
      std::string m_label {""};
      int m_dimension {0};
      int m_nvertices {0};
      int m_ntriangles {0};
      int m_nquadrilaterals {0};
      int m_ntetrahedra {0};
      int m_nhexahedra {0};

      std::unordered_map<int, ELEMENTS::TYPE> m_TypeMap {};

      void InitElementsMapping();
  };

}
