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

#include "MeshDataWriters/Variable.hpp"



namespace PAMELA
{
	template <class T1,class T2>
	class Property
	{
	public:
		Property(T1* owner)
		{
			m_Owner = owner;
		}

		//Getter
		std::unordered_map<std::string, ParallelEnsemble<T2>>& get_PropertyMap() { return m_data; }
		T1* get_Owner() { return m_Owner; }

		//Property
		void ReferenceProperty(std::string label)
		{
                  ReferenceProperty( label, VARIABLE_DIMENSION::SCALAR );
		}

		void ReferenceProperty( std::string label, VARIABLE_DIMENSION dim )
		{
			m_data[label] = ParallelEnsemble<T2> ();
                        m_dimension[label] =dim;
		}

		//void CreateProperty(std::string label)
		//{
		//	ReferenceProperty(label);
		//	m_data[label] = std::vector<T2>(m_Owner->size_owned(),0);
		//}

		void SetConstantProperty(std::string label, T2 val)
		{
			ASSERT(m_data.count(label) == 1, "Property does not exist");
			this->m_data[label] = ElementEnsemble<T2>(m_Owner->size_owned(),val);
		}

		void SetProperty(std::string label, std::vector<T2> val)
		{
			ASSERT(m_data.count(label) == 1, "Property does not exist");
			//TODO:ASSERT(static_cast<int>(val.size) == m_Owner->size_owned(), "Input vector should be same size as the owner");
			m_data[label].push_back_owned(val);
		}

		void GetProperty_owned(std::string& label) { return m_data[label].data(); }

                VARIABLE_DIMENSION GetProperty_dimension(const std::string& label) { return m_dimension.at(label); }

		void ClearAfterPartitioning(std::set<int> owned, std::set<int> ghost)
		{

			for (auto it = m_data.begin(); it != m_data.end(); ++it)
			{
				it->second.Shrink(owned, ghost, static_cast<int>( m_dimension[it->first] ));
			}
		}


	protected:

		T1* m_Owner;
		//std::unordered_map<std::string, std::vector<T2>> m_data;
		std::unordered_map<std::string, ParallelEnsemble<T2>> m_data;
		std::unordered_map<std::string, VARIABLE_DIMENSION> m_dimension;
	};
	
}
