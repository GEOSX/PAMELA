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

#include<string>
#include "CartesianMesh.hpp"
#include "Utils/Logger.hpp"
#include <random>

namespace PAMELA
{
	CartesianMesh::CartesianMesh(const std::vector<double>& dx, const std::vector<double>& dy, const std::vector<double>& dz) :Mesh()
	{
		//Save 
		m_dx = dx;
		m_dy = dy;
		m_dz = dz;

		//
		m_Lxmin = 0;
		m_Lymin = 0;
		m_Lzmin = 0;

		LOGINFO("*** Making Cartesian Mesh...");

		LOGINFO("Initialization...");

		//Groups
		m_PolygonCollection.addAndCreateGroup("West");
		m_PolygonCollection.addAndCreateGroup("East");
		m_PolygonCollection.addAndCreateGroup("North");
		m_PolygonCollection.addAndCreateGroup("South");
		m_PolygonCollection.addAndCreateGroup("Top");
		m_PolygonCollection.addAndCreateGroup("Bottom");

		//Dim
		int dxSize = static_cast<int>(dx.size());
		int dySize = static_cast<int>(dy.size());
		int dzSize = static_cast<int>(dz.size());


		//Point vectors
		std::vector<double> xVector, yVector, zVector;
		//--x
		double val = 0;
		xVector.push_back(0);
		for (auto i = 0; i < dxSize; i++)
		{
			val = val + dx[i];
			xVector.push_back(val);
		}
		//--y
		val = 0;
		yVector.push_back(0);
		for (auto i = 0; i < dySize; i++)
		{
			val = val + dy[i];
			yVector.push_back(val);
		}

		//--z
		val = 0;
		zVector.push_back(0);
		for (auto i = 0; i < dzSize; i++)
		{
			val = val + dz[i];
			zVector.push_back(val);
		}

		//Max val
		m_Lxmax = xVector.back();
		m_Lymax = yVector.back();
		m_Lzmax = zVector.back();

		std::string CurrentPointGrp;

		LOGINFO("Create Vertices...");
		//Vertices
		int ivertex = 0;
		for (auto k = 0; k < dzSize + 1; k++)
		{
			for (auto j = 0; j < dySize + 1; j++)
			{
				for (auto i = 0; i < dxSize + 1; i++)
				{
					CurrentPointGrp = "DEFAULT";
					addPoint(ELEMENTS::TYPE::VTK_VERTEX, ivertex, CurrentPointGrp, xVector[i], yVector[j], zVector[k]);
					++ivertex;
				}
			}
		}


		LOGINFO("Create polyhedra...");

		//Polyhedron and Boundary faces
		std::vector<Point*> vertexListHexa{ nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
		std::vector<Point*> vertexListQuad{ nullptr, nullptr, nullptr, nullptr };
		int ihexa = 0;
		int iquad = 0;
		for (auto k = 0; k < dzSize; k++)
		{
			for (auto j = 0; j < dySize; j++)
			{
				for (auto i = 0; i < dxSize; i++)
				{

					//------ Hexahedra

					//0
					vertexListHexa[0] = m_PointCollection[i + j * dxSize + j + k * (dxSize + 1) * (dySize + 1)];

					//1
					vertexListHexa[1] = m_PointCollection[i + j * dxSize + j + k * (dxSize + 1) * (dySize + 1) + 1];

					//2
					vertexListHexa[2] = m_PointCollection[i + j * dxSize + j + k * (dxSize + 1) * (dySize + 1) + dxSize + 2];

					//3
					vertexListHexa[3] = m_PointCollection[i + j * dxSize + j + k * (dxSize + 1) * (dySize + 1) + dxSize + 1];

					//4
					vertexListHexa[4] = m_PointCollection[i + j * dxSize + j + (k + 1) * (dxSize + 1) * (dySize + 1)];

					//5
					vertexListHexa[5] = m_PointCollection[i + j * dxSize + j + (k + 1) * (dxSize + 1) * (dySize + 1) + 1];

					//6
					vertexListHexa[6] = m_PointCollection[i + j * dxSize + j + (k + 1) * (dxSize + 1) * (dySize + 1) + dxSize + 2];

					//7
					vertexListHexa[7] = m_PointCollection[i + j * dxSize + j + (k + 1) * (dxSize + 1) * (dySize + 1) + dxSize + 1];

					//Add polyhedron to collection
					addPolyhedron(ELEMENTS::TYPE::VTK_HEXAHEDRON, ihexa, "DEFAULT", vertexListHexa);


					//------ Quads at boundary

					if (i == 0)
					{
						//0
						vertexListQuad[0] = vertexListHexa[0];

						//1
						vertexListQuad[1] = vertexListHexa[3];

						//2
						vertexListQuad[2] = vertexListHexa[7];

						//3
						vertexListQuad[3] = vertexListHexa[4];

						//Add polyhedron to collection
						addPolygon(ELEMENTS::TYPE::VTK_QUAD, iquad, "West", vertexListQuad);
					}

					if (i == dxSize - 1)
					{
						//0
						vertexListQuad[0] = vertexListHexa[1];

						//1
						vertexListQuad[1] = vertexListHexa[2];

						//2
						vertexListQuad[2] = vertexListHexa[6];

						//3
						vertexListQuad[3] = vertexListHexa[5];

						//Add polygon to collection
						addPolygon(ELEMENTS::TYPE::VTK_QUAD, iquad, "East", vertexListQuad);
					}

					if (k == 0)
					{
						//0
						vertexListQuad[0] = vertexListHexa[0];

						//1
						vertexListQuad[1] = vertexListHexa[1];

						//2
						vertexListQuad[2] = vertexListHexa[2];

						//3
						vertexListQuad[3] = vertexListHexa[3];

						//Add polyhedron to collection
						addPolygon(ELEMENTS::TYPE::VTK_QUAD, iquad, "Bottom", vertexListQuad);
					}

					if (k == dzSize - 1)
					{
						//0
						vertexListQuad[0] = vertexListHexa[4];

						//1
						vertexListQuad[1] = vertexListHexa[5];

						//2
						vertexListQuad[2] = vertexListHexa[6];

						//3
						vertexListQuad[3] = vertexListHexa[7];

						//Add polygon to collection
						addPolygon(ELEMENTS::TYPE::VTK_QUAD, iquad, "Top", vertexListQuad);
					}

					if (j == 0)
					{
						//0
						vertexListQuad[0] = vertexListHexa[0];

						//1
						vertexListQuad[1] = vertexListHexa[1];

						//2
						vertexListQuad[2] = vertexListHexa[5];

						//3
						vertexListQuad[3] = vertexListHexa[4];

						//Add polyhedron to collection
						addPolygon(ELEMENTS::TYPE::VTK_QUAD, iquad, "South", vertexListQuad);
					}

					if (j == dySize - 1)
					{
						//0
						vertexListQuad[0] = vertexListHexa[3];

						//1
						vertexListQuad[1] = vertexListHexa[2];

						//2
						vertexListQuad[2] = vertexListHexa[6];

						//3
						vertexListQuad[3] = vertexListHexa[7];

						//Add polygon to collection
						addPolygon(ELEMENTS::TYPE::VTK_QUAD, iquad, "North", vertexListQuad);
					}

				}
			}
		}

		//By default all polyhedron groups are active, only polygon boundary groups are active
		m_PolygonCollection.MakeActiveGroup("North");
		m_PolygonCollection.MakeActiveGroup("South");
		m_PolygonCollection.MakeActiveGroup("East");
		m_PolygonCollection.MakeActiveGroup("West");
		m_PolygonCollection.MakeActiveGroup("Top");
		m_PolygonCollection.MakeActiveGroup("Bottom");

		m_PolyhedronCollection.MakeActiveGroup("DEFAULT");


	}

	void CartesianMesh::Distort(double alpha)
	{
		LOGINFO("*** Distorting Mesh...");

		//Compute minimum spacing
		double minDx = *(std::min_element(m_dx.begin(), m_dx.end()));
		double minDy = *(std::min_element(m_dy.begin(), m_dy.end()));
		double minDz = *(std::min_element(m_dz.begin(), m_dz.end()));
		double minD = std::min(minDx, minDy);
		minD = std::min(minD, minDz);

		auto& Pointcollection = m_PointCollection;

		//Randomly move points in all directions
		srand(0);
		for (auto it = Pointcollection.begin(); it != Pointcollection.end(); ++it)
		{
			auto& coord = (*it)->get_coordinates();
			coord.x = coord.x + (rand() % 10 + 1)*minDx*alpha / 10;
			coord.y = coord.y + (rand() % 10 + 1)*minDy*alpha / 10;
			coord.z = coord.z + (rand() % 10 + 1)*minDz*alpha / 10;
		}

		//Realign point on boundaries
		//--North
		auto Northgroup = m_PolygonCollection.get_Group("North");
		for (auto it = Northgroup->begin(); it != Northgroup->end(); ++it)
		{
			auto& vertexList = (*it)->get_vertexList();
			for (auto it2 = vertexList.begin(); it2 != vertexList.end(); ++it2)
			{
				(*it2)->get_coordinates().y = m_Lymax;
			}
		}
		//--South
		auto Southgroup = m_PolygonCollection.get_Group("South");
		for (auto it = Southgroup->begin(); it != Southgroup->end(); ++it)
		{
			auto& vertexList = (*it)->get_vertexList();
			for (auto it2 = vertexList.begin(); it2 != vertexList.end(); ++it2)
			{
				 (*it2)->get_coordinates().y = m_Lymin;
			}
		}
		//--East
		auto Eastgroup = m_PolygonCollection.get_Group("East");
		for (auto it = Eastgroup->begin(); it != Eastgroup->end(); ++it)
		{
			auto& vertexList = (*it)->get_vertexList();
			for (auto it2 = vertexList.begin(); it2 != vertexList.end(); ++it2)
			{
				(*it2)->get_coordinates().x = m_Lxmax;
			}
		}
		//--West
		auto Westgroup = m_PolygonCollection.get_Group("West");
		for (auto it = Westgroup->begin(); it != Westgroup->end(); ++it)
		{
			auto& vertexList = (*it)->get_vertexList();
			for (auto it2 = vertexList.begin(); it2 != vertexList.end(); ++it2)
			{
				(*it2)->get_coordinates().x = m_Lxmin;
			}
		}
		//--Top
		auto Topgroup = m_PolygonCollection.get_Group("Top");
		for (auto it = Topgroup->begin(); it != Topgroup->end(); ++it)
		{
			auto& vertexList = (*it)->get_vertexList();
			for (auto it2 = vertexList.begin(); it2 != vertexList.end(); ++it2)
			{
				(*it2)->get_coordinates().z = m_Lzmax;
			}
		}
		//--Bottom
		auto Bottomgroup = m_PolygonCollection.get_Group("Bottom");
		for (auto it = Bottomgroup->begin(); it != Bottomgroup->end(); ++it)
		{
			auto& vertexList = (*it)->get_vertexList();
			for (auto it2 = vertexList.begin(); it2 != vertexList.end(); ++it2)
			{
				(*it2)->get_coordinates().z = m_Lzmin;
			}
		}

		LOGINFO("*** Done...");
	}

}
