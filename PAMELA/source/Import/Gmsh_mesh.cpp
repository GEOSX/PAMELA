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

#include "Import/Gmsh_mesh.hpp"
#include "Utils/StringUtils.hpp"
#include "Elements/ElementFactory.hpp"
#include "Elements/Element.hpp"
#include <map>
#include "Parallel/Communicator.hpp"

namespace PAMELA
{
	Mesh* Gmsh_mesh::CreateMesh(std::string file_path)
	{

		//MPI
		auto irank = Communicator::worldRank();

		LOGINFO("*** Importing Gmsh mesh format...");

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

		//Attributes and groups
		int attribute=0;
		int id;
		int itype;
		int ntags;
		int v0, v1, v2, v3, v4, v5, v6, v7;
		std::vector<Point*> vertexTemp3 = { nullptr,nullptr,nullptr };
		std::vector<Point*> vertexTemp4 = { nullptr,nullptr,nullptr,nullptr };
		std::vector<Point*> vertexTemp5 = { nullptr,nullptr,nullptr,nullptr,nullptr };
		std::vector<Point*> vertexTemp6 = { nullptr,nullptr,nullptr,nullptr,nullptr,nullptr };
		std::vector<Point*> vertexTemp8 = { nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr };
                std::vector< int > oldToNewVertexIndex;
		ElementCollection<Point*>& vertexcollection = *(mesh->get_PointCollection());
		ELEMENTS::TYPE elementType;
		std::string line;
		while (StringUtils::safeGetline(mesh_file, line))
		{
			if ((line == "$Nodes"))
			{
				//dimension
				if (!(mesh_file >> m_nnodes))
				{
					LOGERROR("Error while reading" + line);
				}
                                oldToNewVertexIndex.resize(m_nnodes);

				LOGINFO("Reading nodes...");

				//data
				elementType =ELEMENTS::TYPE::VTK_VERTEX;
				double x, y, z;
				std::vector<Vertex*> vertexTemp = { nullptr };
                                int offset = 0;
				for (int i = 0; i != m_nnodes; i++)
				{
					mesh_file >> id >> x >> y >> z;
					auto res =  mesh->addPoint(elementType, i, "DEFAULT", x, y, z);
                                          oldToNewVertexIndex[i] = res.first->get_localIndex(); 
				}
				LOGINFO("Done" +  std::to_string(offset));
			}
			else if ((line == "$PhysicalNames"))
			{
				int dim;
				int tagid;
				std::string label;
				mesh_file >> m_nphysicalregions; //number of phhysical regions
				for (auto i = 0; i != m_nphysicalregions; i++)
				{
					mesh_file >> dim >> tagid >> label;
					StringUtils::RemoveDoubleQuotes(label);
					if (dim==2)
					{
						m_TagNamePolygon[tagid] =  label;
					}
					else if (dim==3)
					{
						m_TagNamePolyhedron[tagid] = label;
					}
				}

			}
			else if ((line == "$Elements"))
			{
				int nPolygon = 0, nPolyhedron = 0;

				//dimension
				if (!(mesh_file >> m_nelements))
				{
					LOGERROR("Error while reading" + line);
				}

				LOGINFO("Reading elements...");

				std::string grplabel;

				for (auto i = 0; i != m_nelements; i++)
				{

					//data
					mesh_file >> id;	//id
					mesh_file >> itype;	//itype
					mesh_file >> ntags;	//itype
          if( ntags < 1 )
          {
            attribute = 1;
          }
          else
          {
						mesh_file >> attribute; //trash
          }
          std::string trash;
					for (auto nt = 1; nt < ntags; nt++)
					{
            mesh_file >> trash;
					}
          if(attribute == 0) attribute = 1; // Cant have a 0 attribute

					switch (itype)
					{

					case 2:	//TRIANGLE
                                                elementType =ELEMENTS::TYPE::VTK_TRIANGLE;
						mesh_file >> v0 >> v1 >> v2;
						vertexTemp3[0] = vertexcollection[v0 - 1];
						vertexTemp3[1] = vertexcollection[v1 - 1];
						vertexTemp3[2] = vertexcollection[v2 - 1];

						//Find group label
						if (m_TagNamePolygon.count(attribute) == 0)
						{
							grplabel= "POLYGON_GROUP_DEFAULT_" + std::to_string(attribute);
						}
						else
						{
							grplabel = "POLYGON_GROUP_" + m_TagNamePolygon[attribute] + "_" + std::to_string(attribute);
						}

						mesh->addPolygon(elementType, nPolygon, grplabel, vertexTemp3);
						mesh->get_PolygonCollection()->MakeActiveGroup(grplabel);
						nPolygon++;
						m_ntriangles++;
						break;

					case 3:	//QUADRANGLE
				                elementType =ELEMENTS::TYPE::VTK_QUAD;
						mesh_file >> v0 >> v1 >> v2 >> v3;
						vertexTemp4[0] = vertexcollection[v0 - 1];
						vertexTemp4[1] = vertexcollection[v1 - 1];
						vertexTemp4[2] = vertexcollection[v2 - 1];
						vertexTemp4[3] = vertexcollection[v3 - 1];

						//Find group label
						if (m_TagNamePolygon.count(attribute) == 0)
						{
							grplabel = "POLYGON_GROUP_DEFAULT_" + std::to_string(attribute);
						}
						else
						{
							grplabel = "POLYGON_GROUP_" + m_TagNamePolygon[attribute] + "_" + std::to_string(attribute);
						}

						mesh->addPolygon(elementType, nPolygon, grplabel, vertexTemp4);
						mesh->get_PolygonCollection()->MakeActiveGroup(grplabel);
						nPolygon++;
						m_nquadrangles++;
						break;

					case 4:	//TETRAHEDRON
                                                elementType =ELEMENTS::TYPE::VTK_TETRA;
						mesh_file >> v0 >> v1 >> v2 >> v3;
						vertexTemp4[0] = vertexcollection[oldToNewVertexIndex[v0 - 1]];
						vertexTemp4[1] = vertexcollection[oldToNewVertexIndex[v1 - 1]];
						vertexTemp4[2] = vertexcollection[oldToNewVertexIndex[v2 - 1]];
						vertexTemp4[3] = vertexcollection[oldToNewVertexIndex[v3 - 1]];

						//Find group label
						if (m_TagNamePolyhedron.count(attribute) == 0)
						{
							grplabel = "POLYHEDRON_GROUP_DEFAULT_" + std::to_string(attribute);
						}
						else
						{
							grplabel = "POLYHEDRON_GROUP_" + m_TagNamePolyhedron[attribute] + "_" +  std::to_string(attribute);
						}

						mesh->addPolyhedron(elementType, nPolyhedron, grplabel, vertexTemp4);
						mesh->get_PolyhedronCollection()->MakeActiveGroup(grplabel);
						nPolyhedron++;
						m_ntetrahedra++;
						break;

					case 5:	//HEXAHEDRON
                                                elementType =ELEMENTS::TYPE::VTK_HEXAHEDRON;
						mesh_file >> v0 >> v1 >> v2 >> v3 >> v4 >> v5 >> v6 >> v7;
						vertexTemp8[0] = vertexcollection[v0 - 1];
						vertexTemp8[1] = vertexcollection[v1 - 1];
						vertexTemp8[2] = vertexcollection[v2 - 1];
						vertexTemp8[3] = vertexcollection[v3 - 1];
						vertexTemp8[4] = vertexcollection[v4 - 1];
						vertexTemp8[5] = vertexcollection[v5 - 1];
						vertexTemp8[6] = vertexcollection[v6 - 1];
						vertexTemp8[7] = vertexcollection[v7 - 1];

						//Find group label
						if (m_TagNamePolyhedron.count(attribute) == 0)
						{
							grplabel = "POLYHEDRON_GROUP_DEFAULT_" + std::to_string(attribute);
						}
						else
						{
							grplabel = "POLYHEDRON_GROUP_" + m_TagNamePolyhedron[attribute] + "_" +  std::to_string(attribute);
						}

						mesh->addPolyhedron(elementType, nPolyhedron, grplabel, vertexTemp8);
						mesh->get_PolyhedronCollection()->MakeActiveGroup(grplabel);
						nPolyhedron++;
						m_nhexahedra++;
						break;

					case 6:	//PRISM
                                                elementType =ELEMENTS::TYPE::VTK_WEDGE;
						mesh_file >> v0 >> v1 >> v2 >> v3 >> v4 >> v5;
						vertexTemp6[0] = vertexcollection[v0 - 1];
						vertexTemp6[1] = vertexcollection[v1 - 1];
						vertexTemp6[2] = vertexcollection[v2 - 1];
						vertexTemp6[3] = vertexcollection[v3 - 1];
						vertexTemp6[4] = vertexcollection[v4 - 1];
						vertexTemp6[5] = vertexcollection[v5 - 1];

						//Find group label
						if (m_TagNamePolyhedron.count(attribute) == 0)
						{
							grplabel = "POLYHEDRON_GROUP_DEFAULT_" + std::to_string(attribute);
						}
						else
						{
							grplabel = "POLYHEDRON_GROUP_" + m_TagNamePolyhedron[attribute] + "_" +  std::to_string(attribute);
						}

						mesh->addPolyhedron(elementType, nPolyhedron, grplabel, vertexTemp6);
						mesh->get_PolyhedronCollection()->MakeActiveGroup(grplabel);
						nPolyhedron++;
						m_nprisms++;
						break;

					case 7:	//PYRAMID
                                                elementType =ELEMENTS::TYPE::VTK_PYRAMID;
						mesh_file >> v0 >> v1 >> v2 >> v3 >> v4;
						vertexTemp5[0] = vertexcollection[v0 - 1];
						vertexTemp5[1] = vertexcollection[v1 - 1];
						vertexTemp5[2] = vertexcollection[v2 - 1];
						vertexTemp5[3] = vertexcollection[v3 - 1];
						vertexTemp5[4] = vertexcollection[v4 - 1];

						//Find group label
						if (m_TagNamePolyhedron.count(attribute) == 0)
						{
							grplabel = "POLYHEDRON_GROUP_DEFAULT_" + std::to_string(attribute);
						}
						else
						{
							grplabel = "POLYHEDRON_GROUP_" + m_TagNamePolyhedron[attribute] + "_" +  std::to_string(attribute);
						}

						mesh->addPolyhedron(elementType, nPolyhedron, grplabel, vertexTemp5);
						mesh->get_PolyhedronCollection()->MakeActiveGroup(grplabel);
						nPolyhedron++;
						m_npyramids++;
						break;

					default:
						std::string toto;
						getline(mesh_file, toto);  //get rest of the line
						break;
					}

				}

			}
			else if ((line == "$ElementData"))
			{
				LOGINFO("Reading element data...");
                                int nb_string_tags;
                                int nb_double_tags;
                                int nb_int_tags;

                                std::string ppt_name;
                                double time_value;
                                int time_step;
                                int nb_components;
                                int nb_values;

                                /// Read the property name
                                mesh_file >> nb_string_tags;
                                ASSERT(nb_string_tags > 0, "Property has to be named");
                                mesh_file >> ppt_name;
                                if( nb_string_tags > 1) {
                                  std::string trash;
                                  for(int i = 0 ; i < nb_string_tags -1 ; i++) {
                                    mesh_file >> trash;
                                  }
                                }

                                /// Read the time value
                                mesh_file >> nb_double_tags;
                                if( nb_double_tags !=0) {
                                  mesh_file >> time_value;
                                  if( nb_double_tags > 1) {
                                    double trash;
                                    for(int i = 0 ; i < nb_double_tags-1 ; i++) {
                                      mesh_file >> trash;
                                    }
                                  }
                                }

                                /// Read the time step and the number of elements
                                mesh_file >> nb_int_tags;
                                ASSERT(nb_int_tags > 2, "Property " + ppt_name + " has to have 3 integers tags (time step, nb of components, nb of elements");
                                mesh_file >> time_step >> nb_components >> nb_values;
                                ASSERT(nb_values == static_cast<int>(mesh->get_PolyhedronCollection()->size_all()), "");
                                if( nb_int_tags > 3) {
                                  int trash;
                                  for(int i = 0 ; i < nb_int_tags -3 ; i++) {
                                    mesh_file >> trash;
                                  }
                                }

                                /// Fill the properties
		                auto props_double = mesh->get_PolyhedronProperty_double();
                                props_double->ReferenceProperty( ppt_name, static_cast< VARIABLE_DIMENSION >(nb_components));
                                std::vector< double > ppt_vector(nb_values * nb_components);
                                ASSERT(static_cast<int>(props_double->get_Owner()->size_owned()) == nb_values, "Wrong property size");
                                int index;
                                for(int i = 0 ; i < nb_values ; i++) {
                                  mesh_file >> index;
                                  for( int j = 0; j < nb_components; j++)
                                  {
                                    mesh_file  >> ppt_vector[i*nb_components + j];   
                                  }
                                }
                                props_double->SetProperty(ppt_name,ppt_vector);
                        }

			else
			{
				LOGDEBUG(line + " is ignored");
			}
		}

		//
		LOGINFO("Number of nodes = " + std::to_string(static_cast<long long>(m_nnodes)));
		LOGINFO("Number of triangles = " + std::to_string(static_cast<long long>(m_ntriangles)));
		LOGINFO("Number of quadrilaterals = " + std::to_string(static_cast<long long>(m_nquadrangles)));
		LOGINFO("Number of tetrahedra = " + std::to_string(static_cast<long long>(m_ntetrahedra)));
		LOGINFO("Number of hexahedra = " + std::to_string(static_cast<long long>(m_nhexahedra)));
		LOGINFO("Number of pyramids = " + std::to_string(static_cast<long long>(m_npyramids)));
		LOGINFO("Number of prisms = " + std::to_string(static_cast<long long>(m_nprisms)));

		LOGINFO("*** Done");

		return mesh;

	}

}
