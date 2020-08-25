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
// Library includes
#include "MeshDataWriters/MeshDataWriter.hpp"
namespace PAMELA
{
    class Writer : public MeshDataWriter{
        public:
            Writer(Mesh * mesh, std::string name) : MeshDataWriter(mesh,name) {}
            virtual void Init() final {}
            virtual void Dump() final {}

            const PartMap<Polyhedron*> &  GetPolyhedronMap() const {
              return m_PolyhedronParts;
            }
    };
}
