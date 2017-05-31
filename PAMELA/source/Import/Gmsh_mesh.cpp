#include "Import/Gmsh_mesh.hpp"
#include <experimental/filesystem>
#include "Utils/StringUtils.hpp"
#include "Elements/ElementFactory.hpp"
#include "Elements/Element.hpp"
#include <map>
#include "Utils/Communicator.hpp"

namespace PAMELA
{

	std::string Gmsh_mesh::m_label;
	int Gmsh_mesh::m_dimension = 0;
	int Gmsh_mesh::m_nelements = 0;
	int Gmsh_mesh::m_nnodes = 0;
	int Gmsh_mesh::m_ntriangles = 0;
	int Gmsh_mesh::m_nquadrangles = 0;
	int Gmsh_mesh::m_ntetrahedra = 0;
	int Gmsh_mesh::m_nhexahedra = 0;
	int Gmsh_mesh::m_nprisms = 0;
	int Gmsh_mesh::m_npyramids = 0;
	int Gmsh_mesh::m_nphysicalregions = 0;

	std::unordered_map<GMSH_MESH_TYPE, ELEMENTS::TYPE> Gmsh_mesh::m_TypeMap;
	std::unordered_map<int, std::string> Gmsh_mesh::m_TagNamePolygon;
	std::unordered_map<int, std::string> Gmsh_mesh::m_TagNamePolyhedron;


	void Gmsh_mesh::InitElementsMapping()
	{
		m_TypeMap[GMSH_MESH_TYPE::NODE] = ELEMENTS::TYPE::VTK_VERTEX;
		m_TypeMap[GMSH_MESH_TYPE::LINE] = ELEMENTS::TYPE::VTK_LINE;
		m_TypeMap[GMSH_MESH_TYPE::TRIANGLE] = ELEMENTS::TYPE::VTK_TRIANGLE;
		m_TypeMap[GMSH_MESH_TYPE::QUADRANGLE] = ELEMENTS::TYPE::VTK_QUAD;
		m_TypeMap[GMSH_MESH_TYPE::TETRAHEDRON] = ELEMENTS::TYPE::VTK_TETRA;
		m_TypeMap[GMSH_MESH_TYPE::HEXAHEDRON] = ELEMENTS::TYPE::VTK_HEXAHEDRON;
		m_TypeMap[GMSH_MESH_TYPE::PRISM] = ELEMENTS::TYPE::VTK_WEDGE;
		m_TypeMap[GMSH_MESH_TYPE::PYRAMID] = ELEMENTS::TYPE::VTK_PYRAMID;
		m_TypeMap[GMSH_MESH_TYPE::POINT] = ELEMENTS::TYPE::VTK_VERTEX;
	};

	Mesh* Gmsh_mesh::CreateMesh(std::string file_path)
	{

		//MPI
		auto irank = Communicator::worldRank();

		LOGINFO("*** Importing Gmsh mesh format...");

		Mesh* mesh = new UnstructuredMesh();

		std::ifstream mesh_file_;
		std::string file_contents("A");
		int file_length = 0;

		if (irank == 0)
		{
			//Open file
			mesh_file_.open(file_path);
			ASSERT(mesh_file_.is_open(), file_path + " Could not be open");

			//Transfer file content into string for easing broadcast
			file_contents = { std::istreambuf_iterator<char>(mesh_file_), std::istreambuf_iterator<char>() };
			file_length = static_cast<int>(file_contents.size());

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
		int id;
		int itype;
		int ntags;
		int itrash;
		int v0, v1, v2, v3, v4, v5, v6, v7;
		std::vector<Point*> vertexTemp3 = { nullptr,nullptr,nullptr };
		std::vector<Point*> vertexTemp4 = { nullptr,nullptr,nullptr,nullptr };
		std::vector<Point*> vertexTemp5 = { nullptr,nullptr,nullptr,nullptr,nullptr };
		std::vector<Point*> vertexTemp6 = { nullptr,nullptr,nullptr,nullptr,nullptr,nullptr };
		std::vector<Point*> vertexTemp8 = { nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr };
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

				LOGINFO("Reading nodes...");

				//data
				elementType = m_TypeMap[GMSH_MESH_TYPE::NODE];
				double x, y, z;
				std::vector<Vertex*> vertexTemp = { nullptr };
				for (int i = 0; i != m_nnodes; i++)
				{
					mesh_file >> id >> x >> y >> z;
					mesh->addPoint(elementType, i, "DEFAULT", x, y, z);
				}
				LOGINFO("Done");
			}
			else if ((line == "$PhysicalNames"))
			{
				int nb;
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
					mesh_file >> attribute; //trash
					mesh_file >> itrash;	//attribute=

					switch (itype)
					{
					case 2:	//TRIANGLE
						elementType = m_TypeMap[GMSH_MESH_TYPE::TRIANGLE];
						mesh_file >> v0 >> v1 >> v2;
						vertexTemp3[0] = vertexcollection[v0 - 1];
						vertexTemp3[1] = vertexcollection[v1 - 1];
						vertexTemp3[2] = vertexcollection[v2 - 1];

						//Find group label
						if (m_TagNamePolygon.count(attribute) == 0)
						{
							grplabel= "POLYGON_GROUP_" + std::to_string(attribute);
						}
						else
						{
							grplabel = "POLYGON_GROUP_" + m_TagNamePolygon[attribute];
						}

						mesh->addPolygon(elementType, nPolygon, grplabel, vertexTemp3);
						mesh->get_PolygonCollection()->activeGroup(grplabel);
						nPolygon++;
						m_ntriangles++;
						break;

					case 3:	//QUADRANGLE
						elementType = m_TypeMap[GMSH_MESH_TYPE::QUADRANGLE];
						mesh_file >> v0 >> v1 >> v2 >> v3;
						vertexTemp4[0] = vertexcollection[v0 - 1];
						vertexTemp4[1] = vertexcollection[v1 - 1];
						vertexTemp4[2] = vertexcollection[v2 - 1];
						vertexTemp4[3] = vertexcollection[v3 - 1];

						//Find group label
						if (m_TagNamePolygon.count(attribute) == 0)
						{
							grplabel = "POLYGON_GROUP_" + std::to_string(attribute);
						}
						else
						{
							grplabel = "POLYGON_GROUP_" + m_TagNamePolygon[attribute];
						}

						mesh->addPolygon(elementType, nPolygon, grplabel, vertexTemp4);
						mesh->get_PolygonCollection()->activeGroup(grplabel);
						nPolygon++;
						m_nquadrangles++;
						break;

					case 4:	//TETRAHEDRON
						elementType = m_TypeMap[GMSH_MESH_TYPE::TETRAHEDRON];
						mesh_file >> v0 >> v1 >> v2 >> v3;
						vertexTemp4[0] = vertexcollection[v0 - 1];
						vertexTemp4[1] = vertexcollection[v1 - 1];
						vertexTemp4[2] = vertexcollection[v2 - 1];
						vertexTemp4[3] = vertexcollection[v3 - 1];

						//Find group label
						if (m_TagNamePolyhedron.count(attribute) == 0)
						{
							grplabel = "POLYHEDRON_GROUP_" + std::to_string(attribute);
						}
						else
						{
							grplabel = "POLYHEDRON_GROUP_" + m_TagNamePolyhedron[attribute];
						}

						mesh->addPolyhedron(elementType, nPolyhedron, grplabel, vertexTemp4);
						mesh->get_PolyhedronCollection()->activeGroup(grplabel);
						nPolyhedron++;
						m_ntetrahedra++;
						break;

					case 5:	//HEXAHEDRON
						elementType = m_TypeMap[GMSH_MESH_TYPE::HEXAHEDRON];
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
							grplabel = "POLYHEDRON_GROUP_" + std::to_string(attribute);
						}
						else
						{
							grplabel = "POLYHEDRON_GROUP_" + m_TagNamePolyhedron[attribute];
						}

						mesh->addPolyhedron(elementType, nPolyhedron, grplabel, vertexTemp8);
						mesh->get_PolyhedronCollection()->activeGroup(grplabel);
						nPolyhedron++;
						m_nhexahedra++;
						break;

					case 6:	//PRISM
						elementType = m_TypeMap[GMSH_MESH_TYPE::PRISM];
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
							grplabel = "POLYHEDRON_GROUP_" + std::to_string(attribute);
						}
						else
						{
							grplabel = "POLYHEDRON_GROUP_" + m_TagNamePolyhedron[attribute];
						}

						mesh->addPolyhedron(elementType, nPolyhedron, grplabel, vertexTemp6);
						mesh->get_PolyhedronCollection()->activeGroup(grplabel);
						nPolyhedron++;
						m_nprisms++;
						break;

					case 7:	//PYRAMID
						elementType = m_TypeMap[GMSH_MESH_TYPE::PYRAMID];
						mesh_file >> v0 >> v1 >> v2 >> v3 >> v4;
						vertexTemp5[0] = vertexcollection[v0 - 1];
						vertexTemp5[1] = vertexcollection[v1 - 1];
						vertexTemp5[2] = vertexcollection[v2 - 1];
						vertexTemp5[3] = vertexcollection[v3 - 1];
						vertexTemp5[4] = vertexcollection[v4 - 1];

						//Find group label
						if (m_TagNamePolyhedron.count(attribute) == 0)
						{
							grplabel = "POLYHEDRON_GROUP_" + std::to_string(attribute);
						}
						else
						{
							grplabel = "POLYHEDRON_GROUP_" + m_TagNamePolyhedron[attribute];
						}

						mesh->addPolyhedron(elementType, nPolyhedron, grplabel, vertexTemp5);
						mesh->get_PolyhedronCollection()->activeGroup(grplabel);
						nPolyhedron++;
						m_npyramids++;
						break;

					default:

						break;
					}

				}

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