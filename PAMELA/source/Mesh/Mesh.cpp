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

#include "Mesh/Mesh.hpp"
#include "Elements/ElementFactory.hpp"
#include "Adjacency/Adjacency.hpp"
#include "Parallel//Communicator.hpp"
#include <functional>
#ifdef WITH_MPI
#include <metis.h>
#endif
#include <algorithm>  
#include "Utils/VectorUtils.hpp"

namespace PAMELA
{
  Mesh::~Mesh()
  {
    delete m_PolyhedronProperty_double;
    delete m_PolyhedronProperty_int;
    delete m_AdjacencySet;

  }

  Mesh::Mesh() : m_PointCollection(PointCollection(ELEMENTS::FAMILY::POINT)),
  m_LineCollection(LineCollection(ELEMENTS::FAMILY::LINE)),
  m_PolygonCollection(PolygonCollection(ELEMENTS::FAMILY::POLYGON)),
  m_PolyhedronCollection(PolyhedronCollection(ELEMENTS::FAMILY::POLYHEDRON)),
  m_ImplicitPointCollection(PointCollection(ELEMENTS::FAMILY::POINT)), m_ImplicitLineCollection(LineCollection(ELEMENTS::FAMILY::LINE)),
  m_PolyhedronProperty_double(new Property<PolyhedronCollection, double>(&m_PolyhedronCollection)),
  m_PolyhedronProperty_int(new Property<PolyhedronCollection, int>(&m_PolyhedronCollection)),
  m_AdjacencySet(new AdjacencySet(this))
  {
  }

  void Mesh::CreateFacesFromCells()
  {

    LOGINFO("*** Creating Polygons from Polyhedra...");

    //Create adjacency while creating the faces
    ElementCollection<Polyhedron*>* source = &m_PolyhedronCollection;
    ElementCollection<Polygon*>* target = &m_PolygonCollection;
    ElementCollection<Polyhedron*>* base = &m_PolyhedronCollection;
    auto InitPolyhedronCollectionSize = target->size_all();
    Adjacency* adj = new Adjacency(ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::FAMILY::POLYGON, ELEMENTS::FAMILY::POLYHEDRON,source, target, base);

    int nbFace = 0;
    int FaceIndex = 0;
    int PolyhedronIndex = 0;
    int nval = 0;
    int nrow = 0;

    adj->m_adjacencySparseMatrix->rowPtr.push_back(0);
    for (auto it = source->begin(); it!= source->end();++it)
    {
      Polyhedron* polyhedron = *it;
      PolyhedronIndex = polyhedron->get_localIndex();
      auto faces = polyhedron->CreateFaces();
      nbFace = static_cast<int>(faces.size());
      for (int j = 0; j < nbFace; j++)
      {
        auto returned_polygon = target->push_back_unique(faces[j]);
        FaceIndex = returned_polygon.first->get_localIndex();
        adj->m_adjacencySparseMatrix->columnIndex.push_back(FaceIndex);
        adj->m_adjacencySparseMatrix->values.push_back(PolyhedronIndex);
        nval++;
      }
      nrow++;
      adj->m_adjacencySparseMatrix->rowPtr[nrow] = nval;
    }
    adj->m_adjacencySparseMatrix->nnz = nval;
    adj->m_adjacencySparseMatrix->dimColumn = static_cast<int>(target->size_all());
    adj->m_adjacencySparseMatrix->sortRowIndexAndMoveValues();
    adj->m_adjacencySparseMatrix->checkMatrix();

    //Add to map
    m_AdjacencySet->TopologicalAdjacencyMap[std::make_tuple(source->get_family(), target->get_family(), base->get_family())] = adj;

    //
    LOGINFO(std::to_string(target->size_all() - InitPolyhedronCollectionSize) + " polygons have been created");
    LOGINFO("*** Done");
  }


  std::pair< Point*, bool > Mesh::addPoint(ELEMENTS::TYPE elementType, int index, std::string groupLabel, double x, double y, double z)
  {
    Point* element = ElementFactory::makePoint(elementType, index, x, y, z);
    auto returnedElement = m_PointCollection.AddElement(groupLabel, element);
    if (!returnedElement.second)
    {
      //LOGWARNING("Try to add an existing element");
    }
    return returnedElement;
  }

  std::pair<Point*, bool > Mesh::addPoint(std::string groupLabel, Point* point)
  {
    auto returnedElement = m_PointCollection.AddElement(groupLabel, point);
    if (returnedElement.second)
    {
      //LOGWARNING("Try to add an existing element");
    }
    return returnedElement;
  }

  std::pair< Line*, bool> Mesh::addLine(ELEMENTS::TYPE elementType, int elementIndex, std::string groupLabel, const std::vector<Point*>& vertexList)
  {
    Line* element = ElementFactory::makeLine(elementType, elementIndex, vertexList);
    auto returnedElement = m_LineCollection.AddElement(groupLabel, element);
    if (!returnedElement.second)
    {
      //LOGWARNING("Try to add an existing element");
    }
    return returnedElement;
  }

  std::pair< Polygon*, bool > Mesh::addPolygon(ELEMENTS::TYPE elementType, int elementIndex, std::string groupLabel, const std::vector<Point*>& vertexList)
  {
    Polygon* element = ElementFactory::makePolygon(elementType, elementIndex, vertexList);
    auto returnedElement = m_PolygonCollection.AddElement(groupLabel, element);
    if ( !returnedElement.second )
    {
      //LOGWARNING("Try to add an existing element");
    }
    return returnedElement;
  }

  std::pair< Polyhedron*, bool> Mesh::addPolyhedron(ELEMENTS::TYPE elementType, int elementIndex, std::string groupLabel, const std::vector<Point*>& vertexList)
  {
    Polyhedron* element = ElementFactory::makePolyhedron(elementType, elementIndex, vertexList);
    auto returnedElement = m_PolyhedronCollection.AddElement(groupLabel, element);
    if ( !returnedElement.second)
    {
      //LOGWARNING("Try to add an existing polyhedron");
    }
    return returnedElement;


  }

  void Mesh::AddImplicitLine(ELEMENTS::TYPE elementType, std::string groupLabel, std::vector<Point*>& pointList)
  {
    utils::pamela_unused(elementType);
    m_ImplicitPointCollection.AddElement(groupLabel, pointList[0]);
    for (size_t i=1;i!= pointList.size();++i)
    {
      m_ImplicitPointCollection.AddElement(groupLabel, pointList[i]);
      Line* element = ElementFactory::makeLine(ELEMENTS::TYPE::VTK_LINE, -1, { pointList[i-1],pointList[i] });
      m_ImplicitLineCollection.AddElement(groupLabel, element);
    }
  }


  void Mesh::PerformPolyhedronPartitioning(ELEMENTS::FAMILY edgeElement, ELEMENTS::FAMILY ghostBaseElement)
  {
    //MPI data
    auto CommRankSize = Communicator::worldSize();
    bool MPIRUN = Communicator::isMPIrun();
    int ipartition = Communicator::worldRank();

    //if ((CommRankSize > 1) && (MPIRUN))	//TO be removed for debugging
    //{

    LOGINFO("*** Perform partitioning...");

    //This is a cell partitioning
    ELEMENTS::FAMILY nodeElement = ELEMENTS::FAMILY::POLYHEDRON;

    //Elements affiliation - GLOBAL
    std::vector<int> PolyhedronAffiliation(m_PolyhedronCollection.size_all());

    //PARTITION WISE
    std::set<int> PolyhedronOwned;
    std::set<int> PolygonOwned;
    std::set<int> LineOwned;
    std::set<int> PointOwned;

    std::set<int> PolyhedronGhost;
    std::set<int> PolygonGhost;
    std::set<int> LineGhost;
    std::set<int> PointGhost;
    utils::pamela_unused( edgeElement);

    //Get adjacencies
    auto adjacencyForGhosts = getAdjacencySet()->get_TopologicalAdjacency(nodeElement, nodeElement, ghostBaseElement);

    //Partitioning
    if ((CommRankSize > 1) && (MPIRUN) && m_partitioning_type == "METIS")
    {
      //Compute Partionioning vector from METIS
      LOGINFO("METIS partioning...");
      auto adjacencyForPartitioning = getAdjacencySet()->get_TopologicalAdjacency(nodeElement, nodeElement, edgeElement);
      PolyhedronAffiliation = METISPartitioning(adjacencyForPartitioning, CommRankSize);
    }
    else
    {
      //Compute TRIVIAL Partionioning for one partition
      LOGINFO("TRIVIAL partioning...");
      PolyhedronAffiliation = TRIVIALPartitioning( CommRankSize);
    }

    //PolyhedronAffiliation = METISPartitioning(adjacencyForPartitioning, 2);


    //POLYHEDRON
    //--OWNED POLYHEDRA
    int ind = 0;
    for (size_t i = 0; i != PolyhedronAffiliation.size(); ++i)
    {
      if (PolyhedronAffiliation[i] == ipartition)
      {
        PolyhedronOwned.insert(static_cast<int>(i));
        ind++;
      }
    }

    //--GHOST POLYHEDRA
    for (auto it = PolyhedronOwned.begin(); it != PolyhedronOwned.end(); ++it)
    {
      auto ele_adj = adjacencyForGhosts->get_SingleElementAdjacency(*it);
      for (auto it2 = ele_adj.first.begin(); it2 < ele_adj.first.end(); ++it2)
      {
        if (PolyhedronAffiliation[*it2] != ipartition)
        {
          PolyhedronGhost.insert(*it2);
          m_neighborList.insert(PolyhedronAffiliation[*it2]);
        }
      }
    }

    LOGINFO("Ghost elements...");

    ////OWNED AND GHOST POLYGONS   //TODO Can be much more efficient as it goes multiple times to the same point right now
    auto PolygonPolyhedronAdj = getAdjacencySet()->get_TopologicalAdjacency(ELEMENTS::FAMILY::POLYGON, ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::FAMILY::POLYHEDRON);
    auto PolyhedronPolygonAdj = getAdjacencySet()->get_TopologicalAdjacency(ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::FAMILY::POLYGON, ELEMENTS::FAMILY::POLYHEDRON);
    for (auto it = PolyhedronOwned.begin(); it != PolyhedronOwned.end(); ++it)
    {
      auto adj_Polyhedron2Polygon = PolyhedronPolygonAdj->get_SingleElementAdjacency(*it);
      auto adj_Polyhedron2PolygonSize = adj_Polyhedron2Polygon.first.size();
      for (size_t i = 0; i < adj_Polyhedron2PolygonSize; ++i)
      {
        auto adj_Polygon2Polyhedron = PolygonPolyhedronAdj->get_SingleElementAdjacency(adj_Polyhedron2Polygon.first[i]);
        auto PolyToPart = vectorUtils::Vector2VectorMapping(adj_Polygon2Polyhedron.first, PolyhedronAffiliation);
        if ((std::equal(PolyToPart.begin() + 1, PolyToPart.end(), PolyToPart.begin())) || PolyToPart.size() == 1)
        {
          //All points connect to polyhedra of the current partition
          PolygonOwned.insert(adj_Polyhedron2Polygon.first[i]);
        }
        else
        {

          int ival = CoinToss(PolyToPart[0], PolyToPart[1]);

          if (ival == ipartition)
          {
            PolygonOwned.insert(adj_Polyhedron2Polygon.first[i]);
          }
          else
          {
            PolygonGhost.insert(adj_Polyhedron2Polygon.first[i]);
          }

        }
      }
    }

    ////OWNED AND GHOST POINTS   //TODO Can be much more efficient as it goes multiple times to the same point right now
    auto PointPolyhedronAdj = getAdjacencySet()->get_TopologicalAdjacency(ELEMENTS::FAMILY::POINT, ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::FAMILY::POLYHEDRON);
    auto PolyhedronPointAdj = getAdjacencySet()->get_TopologicalAdjacency(ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::FAMILY::POINT, ELEMENTS::FAMILY::POLYHEDRON);
    for (auto it = PolyhedronOwned.begin(); it != PolyhedronOwned.end(); ++it)
    {
      auto adj_Poly2Point = PolyhedronPointAdj->get_SingleElementAdjacency(*it);
      int adj_Poly2PointSize = static_cast<int>(adj_Poly2Point.first.size());
      for (auto i = 0; i < adj_Poly2PointSize; ++i)
      {
        auto adj_Point2Poly = PointPolyhedronAdj->get_SingleElementAdjacency(adj_Poly2Point.first[i]);
        auto PolyToPart = vectorUtils::Vector2VectorMapping(adj_Point2Poly.first, PolyhedronAffiliation);
        if ((std::equal(PolyToPart.begin() + 1, PolyToPart.end(), PolyToPart.begin())) || PolyToPart.size() == 1)
        {
          //All points connect to polyhedra of the current partition
          PointOwned.insert(adj_Poly2Point.first[i]);
        }
        else
        {
          int ival = vectorUtils::MostOccuringValue(PolyToPart);


          if (ival == ipartition)
          {
            //The majority of points connect to polyhedra that belongs to the current partition
            PointOwned.insert(adj_Poly2Point.first[i]);
          }
          else
          {
            PointGhost.insert(adj_Poly2Point.first[i]);
          }

        }
      }
    }


    //ClearAfterPartitioning
    LOGINFO("Clean mesh...");
    m_PolyhedronCollection.ClearAfterPartitioning(PolyhedronOwned, PolyhedronGhost);
    m_PolygonCollection.ClearAfterPartitioning(PolygonOwned, PolygonGhost);
    m_PointCollection.ClearAfterPartitioning(PointOwned, PointGhost);
    m_PolyhedronProperty_double->ClearAfterPartitioning(PolyhedronOwned, PolyhedronGhost);
    m_PolyhedronProperty_int->ClearAfterPartitioning(PolyhedronOwned, PolyhedronGhost);
    LOGINFO("*** Done...");
    LOGINFO("Clean Adjacency...");
    m_AdjacencySet->ClearAfterPartitioning(PolyhedronOwned, PolyhedronGhost,PolygonOwned, PolygonGhost);
    LOGINFO("*** Done...");

    //}
  }


  std::vector<int> Mesh::METISPartitioning(Adjacency* adjacency, unsigned int npartition)
  {

#ifdef WITH_METIS

    ASSERT(adjacency->get_sourceElementCollection() == adjacency->get_targetElementCollection(), "Partitioning can only be done with adjacency of same elements");
    ASSERT(adjacency->get_sourceElementCollection()->size_all() > npartition, "Number of mesh elements must be greater than the number of partitions");

    auto csrMatrix = adjacency->get_adjacencySparseMatrix();


    // make sure locally we use METIS's types (which are in global namespace) and not grid::<type>
    using idx_t = ::idx_t;

    idx_t options[METIS_NOPTIONS];
    METIS_SetDefaultOptions(options);

    // Some type casts and constants
    idx_t nnodes = static_cast<idx_t>(adjacency->get_sourceElementCollection()->size_all());
    idx_t nconst = 1;
    idx_t objval = 0;
    std::vector<idx_t> partitionVector(nnodes);

    idx_t int_partition = static_cast<idx_t>(npartition);
    //TODO This is a copy to have idx_t. It can be memory consuming.
    std::vector<idx_t> rowPtrMetis(csrMatrix->rowPtr.begin(), csrMatrix->rowPtr.end());
    std::vector<idx_t> columnIndexMetis(csrMatrix->columnIndex.begin(), csrMatrix->columnIndex.end());
    METIS_PartGraphRecursive(&nnodes, &nconst, rowPtrMetis.data(), columnIndexMetis.data(),
        nullptr, nullptr, nullptr, &int_partition, nullptr, nullptr, options, &objval, partitionVector.data());

    return std::vector<int>(partitionVector.begin(), partitionVector.end());

#else
    utils::pamela_unused(adjacency);
    utils::pamela_unused(npartition);
    LOGERROR("METIS partitioner is not available");
    return {};
#endif

  }

  std::vector<int> Mesh::TRIVIALPartitioning( unsigned int npartition )
  {
    int CommRankSize = Communicator::worldSize();
    if( npartition == 1 )
    {
      std::vector<int> val(m_PolyhedronCollection.size_all(), 0);
      return val;
    }
    std::vector< double > max= {std::numeric_limits< double >::min(),
      std::numeric_limits< double >::min(),
      std::numeric_limits< double  >::min() };

    std::vector< double > min= {std::numeric_limits< double >::max(),
      std::numeric_limits< double >::max(),
      std::numeric_limits< double  >::max() };
    for( auto point : m_PointCollection )
    {
      auto coord = point->get_coordinates();
      if( coord.x > max[0] )
      {
        max[0] = coord.x;
      }
      if( coord.y > max[1] )
      {
        max[1] = coord.y;
      }
      if( coord.z > max[2] )
      {
        max[2] = coord.z;
      }
      if( coord.x < min[0] )
      {
        min[0] = coord.x;
      }
      if( coord.y < min[1] )
      {
        min[1] = coord.y;
      }
      if( coord.z < min[2] )
      {
        min[2] = coord.z;
      }
    }
    double xdiff = std::fabs( max[0] - min[0] );
    double ydiff = std::fabs( max[1] - min[1] );
    double shift = 0.;
    int dim_i = -1;
    if ( xdiff > ydiff )
    {
      LOGINFO("Will do a X division");
      shift = xdiff / npartition;
      dim_i = 0;
    }
    else
    {
      LOGINFO("Will do a Y division");
      shift = ydiff / npartition;
      dim_i = 1;
    }
    std::vector< int > partitionVector( m_PolyhedronCollection.size_all(),0 );
    int count = 0;
    for( auto cell : m_PolyhedronCollection )
    {
      auto center = cell->get_centroidCoordinates();
      double dist_to_begin = std::fabs(center[dim_i] - min[dim_i]);
      int part_nb = dist_to_begin / shift;
      partitionVector[count++] = part_nb;
      if (part_nb >= CommRankSize )
      {
        LOGERROR("Wrong partition number attribute");
      }
    }

    return partitionVector;

  }

  void Mesh::CreateLineGroupWithAdjacency(std::string Label, Adjacency* adjacency)
  {

    auto& line_collection = m_LineCollection;
    auto& point_collection = m_PointCollection;

    //CSR Matrix
    auto csr_matrix = adjacency->get_adjacencySparseMatrix();
    auto dimRow = csr_matrix->dimRow;
    auto columIndex = csr_matrix->columnIndex;
    auto rowPtr = csr_matrix->rowPtr;

    int nb_lines = 0;
    int nb_points = 0;

    if ((adjacency->get_sourceFamily() == ELEMENTS::FAMILY::POLYHEDRON) && (adjacency->get_targetFamily() == ELEMENTS::FAMILY::POLYHEDRON))
    {

      //Compute Node coordinates
      int isource = 0, itarget = 0, ipoint = static_cast<int>(m_PointCollection.size_owned()) , iline = static_cast<int>(m_LineCollection.size_owned());

      for (auto irow = 0; irow != dimRow; ++irow)
      {
        if (rowPtr[irow + 1]- rowPtr[irow]>0)
        {
          auto it = get_PolyhedronCollection()->begin_owned() + irow;
          auto xyz1 = (*it)->get_centroidCoordinates();
          auto source_point = ElementFactory::makePoint(ELEMENTS::TYPE::VTK_VERTEX, ipoint, xyz1[0], xyz1[1], xyz1[2]);
          auto source_rpoint = point_collection.AddElement(Label, source_point).first;
          ++ipoint; ++nb_points;
          itarget = isource;
          for (auto icol = rowPtr[irow]; icol != rowPtr[irow + 1]; ++icol)
          {
            if (icol != columIndex[icol])
            {
              itarget = itarget + 1;
              auto it2 = get_PolyhedronCollection()->begin_owned() + columIndex[icol];
              auto xyz2 = (*it2)->get_centroidCoordinates();
              auto target_point = ElementFactory::makePoint(ELEMENTS::TYPE::VTK_VERTEX, ipoint, xyz2[0], xyz2[1], xyz2[2]);
              auto target_rpoint = point_collection.AddElement(Label, target_point).first;
              ++ipoint;
              auto edgev = { source_rpoint , target_rpoint };
              auto edge = ElementFactory::makeLine(ELEMENTS::TYPE::VTK_LINE, iline, edgev);
              ++iline;
              line_collection.AddElement(Label, edge);
            }

          }
          isource = itarget + 1;
          nb_lines++;
        }
      }

    }
    if (nb_lines >0)
    {
      m_LineCollection.MakeActiveGroup(Label);
    }
    if (nb_points > 0)
    {
      m_PointCollection.MakeActiveGroup(Label);
    }

  }
}
