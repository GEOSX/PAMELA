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

#include "Elements/Polyhedron.hpp"
#include "Elements/Element.hpp"
#include "Elements/Polygon.hpp"
#include "Elements/Line.hpp"
#include "Elements/Point.hpp"

namespace PAMELA
{
  namespace ElementFactory {
    Point* makePoint(ELEMENTS::TYPE  elementType, int index, double x, double y, double z);
    Line*  makeLine(ELEMENTS::TYPE  elementType, int index, const std::vector<Point*>& vertexList);
    Polygon*  makePolygon(ELEMENTS::TYPE  elementType, int index, const std::vector<Point*>& vertexList);
    Polyhedron* makePolyhedron(ELEMENTS::TYPE elementType, int index, const std::vector<Point*>& vertexList);
  }
}
