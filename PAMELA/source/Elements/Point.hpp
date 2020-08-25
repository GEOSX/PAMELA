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
#include "Utils/SimpleMaths.hpp"
#include "Elements/Element.hpp"
#include "Utils/Assert.hpp"
#include "Utils/Utils.hpp"

namespace PAMELA
{

	template <typename T>
	class Collection;

	template <>
	class Element<ELEMENTS::FAMILY::POINT> : public ElementBase
	{
	public:

		Element(int index, double x, double y, double z) :ElementBase(), m_coordinates(Coordinates(x, y, z))
		{
                        utils::pamela_unused(index);
			m_family = ELEMENTS::FAMILY::POINT;
			m_vertexList = { this };
                        m_index.Init = index;
		}


		//Getters
		const Coordinates& get_coordinates() const { return m_coordinates; }
		Coordinates& get_coordinates() { return m_coordinates; }

		//Getter
		const std::vector<Element<ELEMENTS::FAMILY::POINT>*>& get_vertexList() const { return m_vertexList; }

	protected:

		Coordinates m_coordinates;

		std::vector<Element<ELEMENTS::FAMILY::POINT>*> m_vertexList;

	};
	typedef Element<ELEMENTS::FAMILY::POINT> Point;



	template <ELEMENTS::TYPE elementType>
	class ElementSpe<ELEMENTS::FAMILY::POINT, elementType> : public Element<ELEMENTS::FAMILY::POINT>
	{
	public:

		ElementSpe(int index, double x, double y, double z) :Element(index, x, y, z)
		{
			ELEMENTS::nVertex.at(static_cast<int>(elementType));
			ASSERT(ELEMENTS::TypeToFamily.at(static_cast<int>(elementType)) == m_family, "Type not compatible with family");
			m_vtkType = elementType;
		}


	private:


	};
	typedef ElementSpe<ELEMENTS::FAMILY::POINT, ELEMENTS::TYPE::VTK_VERTEX> Vertex;

}
