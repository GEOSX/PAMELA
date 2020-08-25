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
#include <unordered_map>
#include "Elements/Element.hpp"
#include "Parallel/ParallelEnsemble.hpp"

namespace PAMELA
{

	//Default Equal
	template <class T>
	struct DefaultEqual
	{
		bool operator()(const T lhs, const T rhs)  const
		{

			return (lhs == rhs);

		}
	};


	template <class T, class HashStruct = std::hash<T>, class EqualStruct = DefaultEqual<T>>
	class ElementEnsemble : public ParallelEnsemble<T>
	{

	public:

		//Defaut constructor for empty ensemble
		ElementEnsemble() :ParallelEnsemble< T >() {}


		//Push back T
		void push_back_owned(T data)  override
		{
                    this->m_data.insert(this->end_owned(), data);
                    this->Increment_owned();
		}

		void push_back_ghost(T data)  override
		{
			this->m_data.insert(this->end_ghost(), data);
			this->Increment_ghost();
		}

		void push_back_owned(std::vector<T> data)  override
		{
			this->m_data.insert(this->end_owned(), data.begin(),data.end());
			this->Increment_owned(data.size());
		}

		void push_back_ghost(std::vector<T> data)  override
		{
			this->m_data.insert(this->end_ghost(), data.begin(), data.end());
			this->Increment_ghost(data.size());
		}


		//Push_back unique
                std::pair< T, bool > push_back_owned_unique(T data)
		{
			auto index = static_cast<int>(this->end_owned() - this->begin_owned());
			auto insertion = m_pointerToLocalIndex.insert(std::make_pair(data, index));
			if (insertion.second) //the element is new and the map has been updated
			{
				this->m_data.insert(this->end_owned(), data);
				(*data).set_localIndex(index);
				this->Increment_owned();
				return std::make_pair( data, true );
			}
			return std::make_pair(insertion.first->first, false);
		}

		std::pair< T, bool > push_back_ghost_unique(T data)
		{
			T returned_element = NULL;
			int index = static_cast<int>(this->end_ghost() - this->begin_ghost());
			auto insertion = m_pointerToLocalIndex.insert(std::make_pair(data, index));
			returned_element = insertion.first->first;
			if (insertion.second) //the element is new and the map has been updated
			{
				this->m_data.insert(this->end_ghost(), data);
				(*data).set_localIndex(index);
				this->Increment_ghost();
				//m_elementToLocalIndexMap.insert(std::make_pair(data, index));
				return std::make_pair( data, true );
			}
			return std::make_pair(insertion.first->first, false);
		}



		std::pair< T, bool > push_back_unique(T data)   //To be use before partitioning
		{
			int index = static_cast<int>(this->end() - this->begin());
			auto insertion = m_pointerToLocalIndex.insert(std::make_pair(data, index));
			if (insertion.second) //the element is new and the map has been updated
			{
				this->m_data.push_back(data);
				(*data).set_localIndex(index);
				(*data).set_globalIndex(index);
				this->Increment_all();
				return std::make_pair( data, true );
			}
                        //insertion.first->first->set_localIndex(index);
                        //insertion.first->first->set_globalIndex(index);
			return std::make_pair(insertion.first->first, false);
		}


		//Getter
		std::unordered_map<int, int> & get_GlobalToLocalIndex() { return m_GlobalToLocalIndex; }


		//Make Empty
		void MakeEmpty() override
		{
			this->m_sizeAll = 0;
			this->m_sizeOwned = 0;
			this->m_sizeGhost = 0;
			this->m_data.clear();
			m_pointerToLocalIndex.clear();
		}

		//Shrink
		void Shrink(std::set<int> owned, std::set<int> ghost, int dimension = 1) override
		{
			//Copy ghost elements amd owned elements in temporary vector
			std::vector<T> ghost_vec_temp; ghost_vec_temp.reserve(ghost.size() * dimension);
			std::vector<T> owned_vec_temp; ghost_vec_temp.reserve(owned.size() * dimension);
			for (auto it = this->m_data.begin(); it != this->m_data.end(); ++it)
			{
				if (ghost.count((*it)->get_globalIndex()) == 1)
				{
					(*it)->set_IsGhost();
					ghost_vec_temp.push_back(*it);
				}
				else if (owned.count((*it)->get_globalIndex()) == 1)
				{
					owned_vec_temp.push_back(*it);
				}
				else
				{
					//delete *it;  //TODO, need to delete dynamics objects but delete multiple time now. Smart pointer?
					//*it = nullptr;
				}
			}

			//Rebuilt data
			this->m_data.clear();
			this->m_data.reserve(ghost_vec_temp.size() + owned_vec_temp.size());
			this->m_data.insert(this->m_data.end(), owned_vec_temp.begin(), owned_vec_temp.end());
			this->m_data.insert(this->m_data.end(), ghost_vec_temp.begin(), ghost_vec_temp.end());
			this->resize_owned(static_cast<int>(owned_vec_temp.size()));
			this->resize_ghost(static_cast<int>(ghost_vec_temp.size()));

			//Update Numbering and map
			int i = 0;
			m_pointerToLocalIndex.clear();
			m_GlobalToLocalIndex.clear();
			for (auto it = this->m_data.begin(); it != this->m_data.end(); ++it)
			{
				(*it)->set_localIndex(i);
				m_pointerToLocalIndex.insert(std::make_pair((*it), static_cast<int>(it - this->m_data.begin())));
				m_GlobalToLocalIndex.insert(std::make_pair((*it)->get_globalIndex(), static_cast<int>(it - this->m_data.begin())));
				i++;
			}

			//Test for emptyness
			if (this->m_data.size() == 0) MakeEmpty();

		}

	protected:

		//Pointer to Index
		std::unordered_map<T, int, HashStruct, EqualStruct> m_pointerToLocalIndex;

		std::unordered_map<int, int> m_GlobalToLocalIndex;


	};

}
