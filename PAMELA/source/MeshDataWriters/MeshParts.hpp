/*
 * ------------------------------------------------------------------------------------------------------------
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 * Copyright (c) 2018-2019 Lawrence Livermore National Security LLC
 * Copyright (c) 2018-2019 The Board of Trustees of the Leland Stanford Junior University
 * Copyright (c) 2018-2019 Total, S.A
 * Copyright (c) 2020-     GEOSX Contributors
 * All rights reserved
 *
 * See top level LICENSE, COPYRIGHT, CONTRIBUTORS, NOTICE, and ACKNOWLEDGEMENTS files for details.
 * ------------------------------------------------------------------------------------------------------------
 */

#pragma once
#include "Part.hpp"
#include "Mesh/Mesh.hpp"

namespace PAMELA {
  template <typename T>
    using PartMap = std::unordered_map<std::string, Part<T>*>;

  std::tuple< PartMap<Polyhedron*>, int> getPolyhedronPartMap(Mesh * mesh, int partIndex = 0);
  std::tuple< PartMap<Polygon*>, int> getPolygonPartMap(Mesh * mesh, int partIndex = 0);
  std::tuple< PartMap<Line*>, int> getLinePartMap(Mesh * mesh, int partIndex = 0);
  std::tuple< PartMap<Point*>, int> getPointPartMap(Mesh * mesh, int partIndex = 0);
}
