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
#include <unordered_map>
#include <typeindex>
#include "Collection/Indexing.hpp"
#include "Utils/Assert.hpp"
#include "Utils/Utils.hpp"
#include <set>

namespace PAMELA
{

	///VTK ELEMENTS
	namespace ELEMENTS
	{

		//Family
		enum class FAMILY { POLYHEDRON = 3, POLYGON = 2, LINE = 1, POINT = 0, UNKNOWN = -1 };

		static const std::unordered_map<int, int> dimension =
		{
			{ static_cast<int>(FAMILY::POLYHEDRON) ,3 },
			{ static_cast<int>(FAMILY::POLYGON) ,2 },
			{ static_cast<int>(FAMILY::LINE) ,1 },
			{ static_cast<int>(FAMILY::POINT) ,0 },
			{ static_cast<int>(FAMILY::UNKNOWN) ,-1 },
		};

		//LABEL
		const std::unordered_map<int, std::string> label =
		{
			{ static_cast<int>(FAMILY::POLYHEDRON) ,"POLYHEDRON" },
			{ static_cast<int>(FAMILY::POLYGON) ,"POLYGON" },
			{ static_cast<int>(FAMILY::LINE) ,"LINE" },
			{ static_cast<int>(FAMILY::POINT) ,"POINT" },
			{ static_cast<int>(FAMILY::UNKNOWN) ,"UNKNOWN" },
		};


		//TYPE
		enum class TYPE { UNKNOWN = -1, VTK_VERTEX = 1, VTK_LINE = 3, VTK_TRIANGLE = 5, VTK_QUAD = 9, VTK_TETRA = 10, VTK_HEXAHEDRON = 12, VTK_WEDGE = 13, VTK_PYRAMID = 14 };

		const std::unordered_map<int, int> nVertex =
		{
			{ static_cast<int>(TYPE::VTK_VERTEX) ,1 },
			{ static_cast<int>(TYPE::VTK_LINE) ,1 },
			{ static_cast<int>(TYPE::VTK_TRIANGLE) ,3 },
			{ static_cast<int>(TYPE::VTK_QUAD) ,4 },
			{ static_cast<int>(TYPE::VTK_TETRA) ,4 },
			{ static_cast<int>(TYPE::VTK_HEXAHEDRON) ,8 },
			{ static_cast<int>(TYPE::VTK_WEDGE) ,6 },
			{ static_cast<int>(TYPE::VTK_PYRAMID) ,5 },
			{ static_cast<int>(TYPE::UNKNOWN),-1 },
		};

		const std::unordered_map<int, int> nFace =
		{
			{ static_cast<int>(TYPE::VTK_TETRA) ,4 },
			{ static_cast<int>(TYPE::VTK_HEXAHEDRON) ,8 },
			{ static_cast<int>(TYPE::VTK_WEDGE) ,5 },
			{ static_cast<int>(TYPE::VTK_PYRAMID) ,5 },
			{ static_cast<int>(TYPE::UNKNOWN) ,-1 },
		};


		//MAPPING
		const std::unordered_map<int, FAMILY> TypeToFamily =
		{
			{ static_cast<int>(TYPE::VTK_VERTEX) ,FAMILY::POINT },
			{ static_cast<int>(TYPE::VTK_LINE) ,FAMILY::LINE },
			{ static_cast<int>(TYPE::VTK_TRIANGLE) ,FAMILY::POLYGON },
			{ static_cast<int>(TYPE::VTK_QUAD) ,FAMILY::POLYGON },
			{ static_cast<int>(TYPE::VTK_TETRA) ,FAMILY::POLYHEDRON },
			{ static_cast<int>(TYPE::VTK_HEXAHEDRON) ,FAMILY::POLYHEDRON },
			{ static_cast<int>(TYPE::VTK_WEDGE) ,FAMILY::POLYHEDRON },
			{ static_cast<int>(TYPE::VTK_PYRAMID) ,FAMILY::POLYHEDRON }

		};

    struct EnumClassHash
    {
      template <typename T>
      std::size_t operator()(T t) const
      {
        return static_cast<int>(t);
      }
    };
	}

	/**
	 * \brief Element base class
	 */
	class ElementBase
	{
	public:
		ElementBase() : m_vtkType(ELEMENTS::TYPE::UNKNOWN), m_family(ELEMENTS::FAMILY::UNKNOWN), m_index(-1),
		                m_partitionOwner(0), m_IsGhost(false)
		{
		}

		//ElementBase(ELEMENTS::FAMILY family,ELEMENTS::TYPE elementType) : m_vtkType(elementType), m_family(family), m_index(-1) {};
		//ElementBase(ELEMENTS::FAMILY family, ELEMENTS::TYPE elementType, int index) : m_vtkType(elementType), m_family(family), m_index(index), m_partitionOwner(0) {};
		//ElementBase(ELEMENTS::FAMILY family, ELEMENTS::TYPE elementType, int index, int partition_owner) : m_vtkType(elementType), m_family(family), m_index(index), m_partitionOwner(partition_owner) {};

		//Getters
		int get_localIndex() const { return m_index.Local; }
		int get_globalIndex() const { return m_index.Global; }
		int get_initIndex() const { return m_index.Init; }
		int get_dimension() { return ELEMENTS::dimension.at(static_cast<int>(m_family)); }
		ELEMENTS::FAMILY get_family() { return m_family; }
		ELEMENTS::TYPE get_vtkType() { return m_vtkType; }

		//Setters
		void set_index(int i) { set_localIndex(i); }
		void set_localIndex(int i) { m_index.Local = i; }
		void set_globalIndex(int i) { m_index.Global = i; }
		void set_initIndex(int i) { m_index.Init = i; }
		void set_partionOwner(int i) { m_partitionOwner = i; }
		void set_IsGhost() { m_IsGhost = true; }

	protected:

		virtual ~ElementBase() = default ;
		ELEMENTS::TYPE m_vtkType;
		ELEMENTS::FAMILY m_family;

		//Index
		IndexData m_index;

		//
		int m_partitionOwner;
		bool m_IsGhost;

	};

	template <ELEMENTS::FAMILY family>
	class Element;

	template <ELEMENTS::FAMILY family, ELEMENTS::TYPE elementType>
	class ElementSpe;

}
