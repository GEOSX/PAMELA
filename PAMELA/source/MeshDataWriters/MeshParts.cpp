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

#include "MeshParts.hpp"
#include "Parallel/Communicator.hpp"

namespace PAMELA {
  std::string PartitionNumberForExtension()
  {
    std::string Ext = std::to_string(Communicator::worldRank() + 1);
    int nExt = static_cast<int>(Ext.size());
    for (auto i = 0; i < (5 - nExt); ++i)
    {
      Ext.insert(0, "0");
    }

    return  Ext;
  }

  template<typename T>
    void FillParts(std::string prefixLabel, PartMap<T>* parts)
    {
      //--Iterate over parts
      for (auto it = parts->begin(); it != parts->end(); ++it)	//Loop over group and act on active groups
      {
        auto partptr = it->second;

        //Update Label
        partptr->Label = prefixLabel + "_" + partptr->Label;

        //----Count number of elements per part
        for (auto it2 = partptr->Collection->begin_owned(); it2 != partptr->Collection->end_owned(); ++it2)
        {
          auto vtkType = (*it2)->get_vtkType();
          partptr->numberOfElementsPerSubPart[static_cast<int>(vtkType)] = partptr->numberOfElementsPerSubPart.at(static_cast<int>(vtkType)) + 1;
        }
        //----Create as many subparts as there are different elements
        for (auto it2 = partptr->numberOfElementsPerSubPart.begin(); it2 != partptr->numberOfElementsPerSubPart.end(); ++it2)
        {
          partptr->SubParts[it2->first] = new SubPart<T>(it2->second, static_cast<ELEMENTS::TYPE>(it2->first));
        }
        //----Fill subparts with elements
        for (auto it2 = partptr->Collection->begin_owned(); it2 != partptr->Collection->end_owned(); ++it2)
        {
          auto vtkType = (*it2)->get_vtkType();
          partptr->SubParts[static_cast<int>(vtkType)]->SubCollection.push_back_owned_unique(*it2);
        }

        //----Mapping from local to global
        int id = 0;
        for (auto it2 = partptr->Collection->begin_owned(); it2 != partptr->Collection->end_owned(); ++it2)
        {
          auto vtkType = (*it2)->get_vtkType();
          auto subpart = partptr->SubParts[static_cast<int>(vtkType)];
          //Map local indexes
          subpart->IndexMapping.push_back(id);
          id++;

          //std::set<Point*> PointsSet;

          //Insert vertices
          auto vertexlist = (*it2)->get_vertexList();
          int vertexsize = static_cast<int>(vertexlist.size());
          for (auto i = 0; i != vertexsize; ++i)
          {
            partptr->Points.push_back(vertexlist[i]);
          }

        }

        std::sort(partptr->Points.begin(), partptr->Points.end());
        partptr->Points.erase(std::unique(partptr->Points.begin(), partptr->Points.end()), partptr->Points.end());


        //----Map Point Coordinates
        int i = 0;
        for (auto it2 = partptr->Points.begin(); it2 != partptr->Points.end(); ++it2)
        {
          partptr->GlobalToLocalPointMapping[(*it2)->get_globalIndex()] = i;
          i++;
        }

      }

    }

  std::tuple< PartMap<Polyhedron*>, int > getPolyhedronPartMap(Mesh * mesh, int partIndex) {
    PartMap<Polyhedron*> partMap;
    auto polyhedronCollection = mesh->get_PolyhedronCollection();
    auto ActiveGroupMapPolyhedron = polyhedronCollection->get_ActiveGroupsMap();

    //--Add active parts
    for (auto it = ActiveGroupMapPolyhedron.begin(); it != ActiveGroupMapPolyhedron.end(); ++it)	//Loop over group and act on active groups
    {
      if (it->second)
      {
        std::string grplabel = it->first;
        std::string grplabelCopy = grplabel;
        std::replace(grplabelCopy.begin(), grplabelCopy.end(), '_', ' ');

        std::vector<std::string> grplabelArray;
        std::stringstream ss(grplabelCopy);
        std::string temp;
        while (ss >> temp)
           grplabelArray.push_back( temp );
        int grpIndex = std::stoi(grplabelArray[grplabelArray.size() -1] ) -1;
        auto groupEnsemble = polyhedronCollection->get_Group(grplabel);
        partMap[grplabel] = new Part<Polyhedron*>(grplabel, grpIndex, grpIndex, groupEnsemble);
        partIndex++;
      }
    }
    FillParts("PART" + PartitionNumberForExtension() + "_" + "POLYHEDRON", &partMap);

    auto mesh_props = mesh->get_PolyhedronProperty_double()->get_PropertyMap();
    for( auto& part : partMap )
    {
      auto curPart = part.second;
      for (auto& mesh_prop : mesh_props)
      {
        auto values = mesh_prop.second;
        auto dim = mesh->get_PolyhedronProperty_double()->GetProperty_dimension(mesh_prop.first);
        int dimInt = static_cast< int >( dim );
        auto var = curPart->AddVariable( dim, VARIABLE_LOCATION::PER_CELL,mesh_prop.first);
        auto nbElements = curPart->Collection->size_owned();
        std::vector< double > values_in_part(nbElements * static_cast<int>(dim));
        for(auto cellBlockItr =curPart->SubParts.begin();
            cellBlockItr != curPart->SubParts.end();
            cellBlockItr++)
        {
          auto cellBlockPtr = cellBlockItr->second;
          for(auto cellItr = cellBlockPtr->SubCollection.begin_owned();
              cellItr != cellBlockPtr->SubCollection.end_owned();
              cellItr++)
          {
            auto cellPtr = *(cellItr);
            auto localIndex2 = cellPtr->get_localIndex();
            auto globalIndex = cellPtr->get_globalIndex();
            for(int i = 0; i < dimInt; i++)
            {
              values_in_part[localIndex2*dimInt+i] = values[globalIndex*dimInt+i];
            }
          }
        }
        var->set_data(values_in_part.begin(), values_in_part.end());
      }
    }
    return std::make_pair(partMap, partIndex);
  }

  std::tuple< PartMap<Polygon*>, int> getPolygonPartMap(Mesh * mesh, int partIndex) {
    PartMap<Polygon*> partMap;
    auto polygonCollection =mesh->get_PolygonCollection();
    auto ActiveGroupMapPolygon = polygonCollection->get_ActiveGroupsMap();

    //--Add active parts
    int localIndex = 0;
    for (auto it = ActiveGroupMapPolygon.begin(); it != ActiveGroupMapPolygon.end(); ++it)	//Loop over group and act on active groups
    {
      if (it->second)
      {
        std::string grplabel = it->first;
        auto groupEnsemble = polygonCollection->get_Group(grplabel);
        partMap[grplabel] = new Part<Polygon*>(grplabel, partIndex, localIndex++, groupEnsemble);
        partIndex++;
      }
    }
    FillParts("PART" + PartitionNumberForExtension() + "_" + "POLYGON", &partMap);
    return std::make_pair(partMap, partIndex);
  }

  std::tuple< PartMap<Line*>, int> getLinePartMap(Mesh * mesh, int partIndex) {
    PartMap<Line*> partMap;
    auto lineCollection = mesh->get_LineCollection();
    auto ActiveGroupMapLine = lineCollection->get_ActiveGroupsMap();

    //--Add active parts
    int localIndex = 0;
    for (auto it = ActiveGroupMapLine.begin(); it != ActiveGroupMapLine.end(); ++it)	//Loop over group and act on active groups
    {
      if (it->second)
      {
        std::string grplabel = it->first;
        auto groupEnsemble = lineCollection->get_Group(grplabel);
        partMap[grplabel] = new Part<Line*>(grplabel, partIndex, localIndex++, groupEnsemble);
        partIndex++;
      }
    }
    FillParts("PART" + PartitionNumberForExtension() + "_" + "LINE", &partMap);

    auto ImplicitLineCollection = mesh->get_ImplicitLineCollection();
    auto ActiveGroupMapImplicitLine = ImplicitLineCollection->get_ActiveGroupsMap();

    //--Add active parts
    localIndex = 0;
    for (auto it = ActiveGroupMapImplicitLine.begin(); it != ActiveGroupMapImplicitLine.end(); ++it)	//Loop over group and act on active groups
    {
      if (it->second)
      {
        std::string grplabel = it->first;
        auto groupEnsemble = ImplicitLineCollection->get_Group(grplabel);
        partMap[grplabel] = new Part<Line*>(grplabel, partIndex, localIndex++, groupEnsemble);
        partIndex++;
      }
    }
    FillParts("PART" + PartitionNumberForExtension() + "_" + "LINE", &partMap);
    return std::make_pair(partMap, partIndex);
  }

  std::tuple< PartMap<Point*>, int> getPointPartMap(Mesh * mesh, int partIndex) {
    PartMap<Point*> partMap;
    auto pointCollection = mesh->get_PointCollection();
    auto ActiveGroupMapPoint = pointCollection->get_ActiveGroupsMap();

    //--Add active parts
    int localIndex = 0;
    for (auto it = ActiveGroupMapPoint.begin(); it != ActiveGroupMapPoint.end(); ++it)	//Loop over group and act on active groups
    {
      if (it->second)
      {
        std::string grplabel = it->first;
        auto groupEnsemble = pointCollection->get_Group(grplabel);
        partMap[grplabel] = new Part<Point*>(grplabel, partIndex, localIndex++, groupEnsemble);
        partIndex++;
      }
    }
    FillParts("PART" + PartitionNumberForExtension() + "_" + "POINT", &partMap);
    return std::make_pair(partMap, partIndex);
  }
}
