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
#include "Elements/Point.hpp"
#include "Utils/SimpleMaths.hpp"
#include "Elements/Element.hpp"
#include "Utils/Assert.hpp"
#include "Utils/Utils.hpp"

namespace PAMELA
{

	template <typename T>
	class Collection;

	template<>
	class Element<ELEMENTS::FAMILY::LINE> : public ElementBase
	{
	public:

		Element(int index, const std::vector<Point*>& vertexList) :ElementBase(), m_vertexList(vertexList)
		{
                        utils::pamela_unused(index);
			m_family = ELEMENTS::FAMILY::LINE;
		}


		//Getter
		const std::vector<Point*>& get_vertexList() const { return m_vertexList; }

	protected:

		std::vector<Point*> m_vertexList;

		//Functions
		virtual double get_length() = 0;
		virtual Coordinates get_centroidLocation() = 0;

	};

	typedef Element<ELEMENTS::FAMILY::LINE> Line;

	template <ELEMENTS::TYPE elementType>
	class ElementSpe<ELEMENTS::FAMILY::LINE, elementType> : public Element<ELEMENTS::FAMILY::LINE>
	{
	public:
		ElementSpe(int index, const std::vector<Point*>& vertexList) :Element(index, vertexList)
		{
			ELEMENTS::nVertex.at(static_cast<int>(elementType));
			ASSERT(ELEMENTS::TypeToFamily.at(static_cast<int>(elementType)) == m_family, "Type not compatible with family");
			m_vtkType = elementType;
		}

	private:

		//Functions
		double get_length() { return 0; }
		Coordinates get_centroidLocation() { return Coordinates(); }

	};

}
