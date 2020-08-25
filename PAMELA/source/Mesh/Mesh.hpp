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
#include "Elements/Point.hpp"
#include "Elements/Line.hpp"
#include "Elements/Polygon.hpp"
#include "Elements/Polyhedron.hpp"
#include "Collection/Collection.hpp"
#include "Property/Property.hpp"
#include "Adjacency/AdjacencySet.hpp"
#include "Utils/Utils.hpp"
#include "MeshDataWriters/Part.hpp"

namespace PAMELA
{

  class Adjacency;

  class Mesh
  {
    public:
      virtual ~Mesh();
      Mesh();

      //Getters
      PointCollection*  get_PointCollection() { return &m_PointCollection; }
      LineCollection*  get_LineCollection() { return &m_LineCollection; }
      PolygonCollection*  get_PolygonCollection() { return &m_PolygonCollection; }
      PolyhedronCollection*  get_PolyhedronCollection() { return &m_PolyhedronCollection; }

      LineCollection*  get_ImplicitLineCollection() { return &m_ImplicitLineCollection; }

      Property<PolyhedronCollection, double>* get_PolyhedronProperty_double() const { return m_PolyhedronProperty_double; }
      Property<PolyhedronCollection, int>* get_PolyhedronProperty_int() const { return m_PolyhedronProperty_int; }

      //Adjacency
      AdjacencySet* getAdjacencySet() const
      {
        return m_AdjacencySet;
      }

      virtual void Distort(double alpha) {
        utils::pamela_unused(alpha);
      }

      ////Updaters
      void CreateFacesFromCells();

      ///Functions to add elements or group to the mesh
      //Add Element
      std::pair< Point*, bool > addPoint(ELEMENTS::TYPE elementType, int elementIndex, std::string groupLabel, double x, double y, double z);
      std::pair< Point*, bool > addPoint(std::string groupLabel, Point* point);
      std::pair< Line*, bool > addLine(ELEMENTS::TYPE elementType, int elementIndex, std::string groupLabel, const std::vector<Point*>& vertexList);
      std::pair< Polygon*, bool > addPolygon(ELEMENTS::TYPE elementType, int elementIndex, std::string groupLabel, const std::vector<Point*>& vertexList);
      std::pair< Polyhedron*, bool > addPolyhedron(ELEMENTS::TYPE elementType, int elementIndex, std::string groupLabel, const std::vector<Point*>& vertexList);

      //Add Implicit Elements
      void AddImplicitLine(ELEMENTS::TYPE elementType, std::string groupLabel, std::vector<Point*>& pointList);



      ///Partitioning
      // This is a graph-based partitioning followed by the add of ghost elements according to ghostBaseElement parameter.
      void PerformPolyhedronPartitioning(ELEMENTS::FAMILY edgeElement, ELEMENTS::FAMILY ghostBaseElement);

      void SetPartitioning( const std::string& partitioningType )
      {
        if( partitioningType != "METIS" && partitioningType != "TRIVIAL" )
        {
          LOGERROR("Unknown partioning type " + partitioningType );
        }
        m_partitioning_type = partitioningType;
      }


      //Adjacency
      void CreateLineGroupWithAdjacency(std::string Label, Adjacency* adjacency);

      std::set<int> const & getNeighborList() const { return m_neighborList; }

    protected:

      //Explicit Element Collections - First owned then ghosts
      PointCollection m_PointCollection;
      LineCollection m_LineCollection;
      PolygonCollection m_PolygonCollection;
      PolyhedronCollection m_PolyhedronCollection;

      //Implicit Element Collections
      PointCollection m_ImplicitPointCollection;
      LineCollection m_ImplicitLineCollection;

      //Property
      Property<PolyhedronCollection,double>* m_PolyhedronProperty_double;
      Property<PolyhedronCollection, int>* m_PolyhedronProperty_int;

      //Adjacency
      AdjacencySet* m_AdjacencySet;

      std::set<int> m_neighborList;

      std::vector<int> METISPartitioning(Adjacency* adjacency, unsigned int npartition);
      std::vector<int> TRIVIALPartitioning( unsigned int npartition );

    private:
      std::string m_partitioning_type { "METIS" };

  };
}
