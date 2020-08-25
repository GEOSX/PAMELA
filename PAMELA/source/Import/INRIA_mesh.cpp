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

#include "Import/INRIA_mesh.hpp"
#include "Utils/StringUtils.hpp"
#include "Elements/ElementFactory.hpp"
#include "Elements/Element.hpp"
#include <map>
#include "Parallel/Communicator.hpp"

namespace PAMELA
{
  void INRIA_mesh::InitElementsMapping()
  {
    m_TypeMap[static_cast<int>(INRIA_MESH_TYPE::TRIANGLE)] = ELEMENTS::TYPE::VTK_TRIANGLE;
    m_TypeMap[static_cast<int>(INRIA_MESH_TYPE::EDGE)] = ELEMENTS::TYPE::VTK_LINE;
    m_TypeMap[static_cast<int>(INRIA_MESH_TYPE::HEXAHEDRON)] = ELEMENTS::TYPE::VTK_HEXAHEDRON;
    m_TypeMap[static_cast<int>(INRIA_MESH_TYPE::QUADRILATERAL)] = ELEMENTS::TYPE::VTK_QUAD;
    m_TypeMap[static_cast<int>(INRIA_MESH_TYPE::TETRAHEDRON)] = ELEMENTS::TYPE::VTK_TETRA;
    m_TypeMap[static_cast<int>(INRIA_MESH_TYPE::VERTEX)] = ELEMENTS::TYPE::VTK_VERTEX;
  }

  Mesh* INRIA_mesh::CreateMesh(std::string file_path)
  {

    //MPI
    auto irank = Communicator::worldRank();

    LOGINFO("*** Importing INRIA mesh format...");

    Mesh* mesh = new UnstructuredMesh();

    std::ifstream mesh_file_;
    std::string file_contents("A");
#ifdef WITH_MPI
    int file_length = 0;
#endif

    if (irank == 0)
    {
      //Open file
      mesh_file_.open(file_path);
      ASSERT(mesh_file_.is_open(), file_path + " Could not be open");

      //Transfer file content into string for easing broadcast
      file_contents = { std::istreambuf_iterator<char>(mesh_file_), std::istreambuf_iterator<char>() };
#ifdef WITH_MPI
      file_length = static_cast<int>(file_contents.size());
#endif

      //Close file
      mesh_file_.close();
    }

#ifdef WITH_MPI
    //Broadcast the mesh input (String)
    MPI_Bcast(&file_length, 1, MPI_INT, 0, MPI_COMM_WORLD);
    file_contents.resize(file_length);
    MPI_Bcast(&file_contents[0], file_length, MPI_CHARACTER, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
#endif

    //Create istringstream for handling content
    std::istringstream mesh_file;
    mesh_file.str(file_contents);

    //Init
    InitElementsMapping();

    //Attributes and groups
    int attribute;
    ELEMENTS::TYPE elementType;
    std::string line;
    while (StringUtils::safeGetline(mesh_file, line))
    {
      if ((line == "Dimension") || (line == " Dimension"))
      {
        //dimension
        if (!(mesh_file >> m_dimension))
        {
          LOGERROR("Error while reading" + line);
        }
      }
      else if ((line == "Vertices") || (line == " Vertices"))
      {
        //dimension
        if (!(mesh_file >> m_nvertices))
        {
          LOGERROR("Error while reading" + line);
        }

        LOGINFO("Reading vertices...");

        //data
        elementType = m_TypeMap[static_cast<int>(INRIA_MESH_TYPE::VERTEX)];
        double x, y, z;
        std::vector<Vertex*> vertexTemp = { nullptr };
        for (int i = 0; i < m_nvertices; i++)
        {
          mesh_file >> x >> y >> z >> attribute;
          mesh->addPoint(elementType, i, "POINT_GROUP_" + std::to_string(attribute), x, y, z);
        }
        LOGINFO("Done");
      }
      else if ((line == "Triangles") || (line == " Triangles"))
      {
        //dimension
        if (!(mesh_file >> m_ntriangles))
        {
          LOGERROR("Error while reading" + line);
        }

        LOGINFO("Reading triangles...");

        //data
        elementType = m_TypeMap[static_cast<int>(INRIA_MESH_TYPE::TRIANGLE)];
        int v0, v1, v2;
        ElementCollection<Point*>& vertexcollection = *(mesh->get_PointCollection());
        std::vector<Point*> vertexTemp = { nullptr,nullptr,nullptr };
        for (int i = 0; i < m_ntriangles; i++)
        {
          mesh_file >> v0 >> v1 >> v2 >> attribute;
          vertexTemp[0] = vertexcollection[v0 - 1];
          vertexTemp[1] = vertexcollection[v1 - 1];
          vertexTemp[2] = vertexcollection[v2 - 1];
          mesh->addPolygon(elementType, i, "POLYGON_GROUP_" + std::to_string(attribute), vertexTemp);
          mesh->get_PolygonCollection()->MakeActiveGroup("POLYGON_GROUP_" + std::to_string(attribute));
        }
        LOGINFO("Done");
      }
      else if ((line == "Quadrilaterals") || (line == " Quadrilaterals"))
      {
        //dimension
        if (!(mesh_file >> m_nquadrilaterals))
        {
          LOGERROR("Error while reading" + line);
        }

        LOGINFO("Reading quadrilaterals...");

        //data
        elementType = m_TypeMap[static_cast<int>(INRIA_MESH_TYPE::QUADRILATERAL)];
        int v0, v1, v2, v3;
        ElementCollection<Point*>& vertexcollection = *(mesh->get_PointCollection());
        std::vector<Point*> vertexTemp = { nullptr,nullptr,nullptr,nullptr };
        for (int i = 0; i < m_nquadrilaterals; i++)
        {
          mesh_file >> v0 >> v1 >> v2 >> v3 >> attribute;
          vertexTemp[0] = vertexcollection[v0 - 1];
          vertexTemp[1] = vertexcollection[v1 - 1];
          vertexTemp[2] = vertexcollection[v3 - 1];
          vertexTemp[3] = vertexcollection[v2 - 1];
          mesh->addPolygon(elementType, i, "POLYHEDRON_GROUP_" + std::to_string(attribute), vertexTemp);
          mesh->get_PolyhedronCollection()->MakeActiveGroup("POLYHEDRON_GROUP_" + std::to_string(attribute));
        }
        LOGINFO("Done");
      }
      else if ((line == "Tetrahedra") || (line == " Tetrahedra"))
      {
        //dimension
        if (!(mesh_file >> m_ntetrahedra))
        {
          LOGERROR("Error while reading" + line);
        }

        LOGINFO("Reading tetrahedra...");

        //data
        elementType = m_TypeMap[static_cast<int>(INRIA_MESH_TYPE::TETRAHEDRON)];
        int v0, v1, v2, v3;
        ElementCollection<Point*>& vertexcollection = *(mesh->get_PointCollection());

        std::vector<Point*> vertexTemp = { nullptr,nullptr,nullptr,nullptr };
        for (int i = 0; i < m_ntetrahedra; i++)
        {
          mesh_file >> v0 >> v1 >> v2 >> v3 >> attribute;
          vertexTemp[0] = vertexcollection[v0 - 1];
          vertexTemp[1] = vertexcollection[v1 - 1];
          vertexTemp[2] = vertexcollection[v2 - 1];
          vertexTemp[3] = vertexcollection[v3 - 1];
          mesh->addPolyhedron(elementType, i, "POLYHEDRON_GROUP_" + std::to_string(attribute), vertexTemp);
          mesh->get_PolyhedronCollection()->MakeActiveGroup("POLYHEDRON_GROUP_" + std::to_string(attribute));
        }
        LOGINFO("Done");

      }
      else if ((line == "Hexahedra") || (line == " Hexahedra"))
      {
        //dimension
        if (!(mesh_file >> m_nhexahedra))
        {
          LOGERROR("Error while reading" + line);
        }

        LOGINFO("Reading hexahedra...");

        //data
        elementType = m_TypeMap[static_cast<int>(INRIA_MESH_TYPE::HEXAHEDRON)];
        int v0, v1, v2, v3, v4, v5, v6, v7;
        ElementCollection<Point*>& vertexcollection = *(mesh->get_PointCollection());
        std::vector<Point*> vertexTemp = { nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr };
        for (int i = 0; i < m_nhexahedra; i++)
        {
          mesh_file >> v0 >> v1 >> v2 >> v3 >> v4 >> v5 >> v6 >> v7 >> attribute;
          vertexTemp[0] = vertexcollection[v0 - 1];
          vertexTemp[1] = vertexcollection[v1 - 1];
          vertexTemp[2] = vertexcollection[v3 - 1];
          vertexTemp[3] = vertexcollection[v2 - 1];
          vertexTemp[4] = vertexcollection[v4 - 1];
          vertexTemp[5] = vertexcollection[v5 - 1];
          vertexTemp[6] = vertexcollection[v7 - 1];
          vertexTemp[7] = vertexcollection[v6 - 1];
          mesh->addPolyhedron(elementType, i, "POLYHEDRON_GROUP_" + std::to_string(attribute), vertexTemp);
          mesh->get_PolyhedronCollection()->MakeActiveGroup("POLYHEDRON_GROUP_" + std::to_string(attribute));
        }
        LOGINFO("Done");
      }
      else
      {
        LOGDEBUG(line + " is ignored");
      }
    }

    //
    LOGINFO("Number of vertices = " + std::to_string(static_cast<long long>(m_nvertices)));
    LOGINFO("Number of triangles = " + std::to_string(static_cast<long long>(m_ntriangles)));
    LOGINFO("Number of quadrilaterals = " + std::to_string(static_cast<long long>(m_nquadrilaterals)));
    LOGINFO("Number of tetrahedra = " + std::to_string(static_cast<long long>(m_ntetrahedra)));
    LOGINFO("Number of hexahedra = " + std::to_string(static_cast<long long>(m_nhexahedra)));

    LOGINFO("*** Done");

    return mesh;

  }

}
