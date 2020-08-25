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


#ifdef WITH_VTK
#include "MeshDataWriters/VTKWriter.hpp" 
#include "Mesh/Mesh.hpp"
#include "MeshDataWriters/Variable.hpp"
#include "Parallel/Communicator.hpp"
#include "Utils/Logger.hpp"
#include "Utils/Utils.hpp"

#include <vtkMPIController.h>
#include <vtkXMLMultiBlockDataWriter.h>
//#include <vtkBlockIdScalars.h>
#include <vtkUnstructuredGrid.h>
#include <vtkCellArray.h>
//#include <vtkXMLUnstructuredGridWriter.h>
//#include <vtkXMLMultiBlockDataWriter.h>
//#include <vtkInformation.h>
#include <vtkCellData.h>
//#include <vtkAOSDataArrayTemplate.h>
#include <vtkDoubleArray.h>
#include <vtkInformation.h>
#include <vtkCompositeDataSet.h>

#include <iomanip>
namespace PAMELA
{
    /// -------------- PUBLIC METHODS
    void VTKWriter::Dump() {
        DeclareAllVariables();
        MakeChildFiles();
        MakeParentFile();
    }

    void VTKWriter::Init() {
    }
    
    /// -------------- PRIVATE METHODS
    
    void VTKWriter::DeclareAllVariables() 
	{
        
    }

    template<typename T>
    void VTKWriter::MakeChildFile(const PartMap<T>* parts, const std::string& prefix) {
      utils::pamela_unused(prefix);

        for (auto it = parts->begin();it != parts->end();++it) 
		{
			m_block_->SetNumberOfBlocks(m_block_->GetNumberOfBlocks()+1);
            vtkSmartPointer<vtkUnstructuredGrid> ug = vtkUnstructuredGrid::New();
            vtkSmartPointer<vtkPoints> vertices = vtkPoints::New();
            auto partptr = it->second;
            for (auto it2 = partptr->Points.begin(); it2 != partptr->Points.end(); ++it2) {
                double point[3] = 
                {(*it2)->get_coordinates().x, (*it2)->get_coordinates().y,
                    (*it2)->get_coordinates().z};
                vertices->InsertNextPoint(point);
            }
            ug->SetPoints(vertices);
            for (auto it2 = partptr->SubParts.begin();it2 != partptr->SubParts.end(); ++it2)
			{
                if (it2->second->SubCollection.size_owned() > 0) 
				{
                    auto subpart = it2->second;
                    auto elementType = subpart->ElementType;
                    auto vtkTypeLabel = ElementToLabel.at(elementType);
                    std::vector<int> cell_types;
                    vtkSmartPointer<vtkCellArray> vtkcells= vtkCellArray::New();
                    for (auto it3 = subpart->SubCollection.begin_owned(); it3 != subpart->SubCollection.end_owned(); ++it3)
                    {
                        cell_types.emplace_back(vtkTypeLabel);
                        auto VertexList = (*it3)->get_vertexList();
                        auto nVertex = VertexList.size();
	                    auto*corners = new vtkIdType[nVertex];
                        for (size_t j = 0; j != nVertex; ++j)
                        {
                            corners[j] = partptr->GlobalToLocalPointMapping.at(VertexList[j]->get_globalIndex());
                        }
                        vtkcells->InsertNextCell(nVertex,corners);
                    }
                    ug->SetCells(cell_types.data(),vtkcells);
                }
            }
			m_block_->SetBlock(m_block_->GetNumberOfBlocks() - 1, ug);
			std::stringstream  name;
			name << it->second->Label;
			m_block_->GetMetaData(m_block_->GetNumberOfBlocks() - 1)->Set(vtkCompositeDataSet::NAME(), name.str().c_str());

			vtkSmartPointer< vtkCellData > cell_data = ug->GetCellData();
			for (auto it2 = partptr->PerElementVariable.begin(); it2 != partptr->PerElementVariable.end(); ++it2)
			{
				auto variableptr = (*it2);

				for (auto it3 = partptr->SubParts.begin(); it3 != partptr->SubParts.end(); ++it3)
				{
					
					if (it3->second->SubCollection.size_owned() > 0)
					{
						auto subpart = it3->second;
						auto dataarray = vtkDoubleArray::New();
						dataarray->SetName(variableptr->Label.c_str());
						std::vector<double> data_vector;
						data_vector.reserve(it3->second->SubCollection.size_owned());

						for (auto it4 = subpart->SubCollection.begin_owned(); it4 != subpart->SubCollection.end_owned(); ++it4)
						{
							auto collectionIndex = it4 - subpart->SubCollection.begin_owned();
							auto variableIndex = subpart->IndexMapping[collectionIndex];
							auto variableData = variableptr->get_data(variableIndex);
							for (auto it5 = variableData.begin(); it5 != variableData.end(); ++it5)
							{
							dataarray->InsertNextTuple1(*it5);
							//data_vector.push_back(*it5);
							}
						}
						cell_data->AddArray(dataarray);

					}
				}

				vtkSmartPointer<vtkDataArray> data_array;
				if (variableptr->Dimension == VARIABLE_DIMENSION::SCALAR) {
				}
				else if (variableptr->Dimension == VARIABLE_DIMENSION::VECTOR) {
				}
				else if (variableptr->Dimension == VARIABLE_DIMENSION::TENSOR_SYMM) {
				}
				else {
					continue;
				}
			}
		}


    }
    void VTKWriter::MakeChildFiles() {
        //-- Line
        MakeChildFile(&m_LineParts, "edge_");

        //-- Polygon
        MakeChildFile(&m_PolygonParts, "polygons_");

        //-- Polyhedron
        MakeChildFile(&m_PolyhedronParts, "polyhedron_");
    }

	void VTKWriter::MakeParentFile()
	{
		vtkSmartPointer<vtkMultiBlockDataSet> multi_block = vtkMultiBlockDataSet::New();
		multi_block->SetNumberOfBlocks(Communicator::worldSize());
		auto controler = vtkMPIController::GetGlobalController();

		if (Communicator::worldRank() != 0)
		{
			//Send blocks to master
			controler->Send(m_block_, 0, Communicator::worldRank());
		}
		else
		{
			for (Types::uint_t proc = 1; proc < Communicator::worldSize(); proc++)
			{
				//Receive blocks from slaves
				vtkSmartPointer< vtkMultiBlockDataSet> block = vtkMultiBlockDataSet::New();
				controler->Receive(block, proc, proc);
				std::stringstream  name;
				name << "RANK_" << proc;
				multi_block->GetMetaData(static_cast<unsigned>(proc))->Set(vtkCompositeDataSet::NAME(), name.str().c_str());
				multi_block->SetBlock(proc, block);
			}
		}

		if (Communicator::worldRank() == 0)		//Master
		{
			//Create own block
			std::stringstream  name;
			name << "RANK_" << 0;
			multi_block->GetMetaData(static_cast<unsigned>(0))->Set(vtkCompositeDataSet::NAME(), name.str().c_str());
			multi_block->SetBlock(0, m_block_);

			//Create master file
			std::string filename = m_name + ".vtm";
			vtkSmartPointer< vtkXMLMultiBlockDataWriter> write =
				vtkXMLMultiBlockDataWriter::New();
			write->SetInputData(multi_block);
			write->SetFileName(filename.c_str());
			write->Write();
		}
	}

}
#endif
