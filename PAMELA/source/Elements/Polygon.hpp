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
#include "Elements/Point.hpp"
#include "Utils/SimpleMaths.hpp"
#include "Line.hpp"
#include "Elements/Element.hpp"
#include "Utils/Assert.hpp"
#include "Utils/Utils.hpp"

namespace PAMELA
{

	template <typename T>
	class Collection;

	template <>
	class Element<ELEMENTS::FAMILY::POLYGON> : public ElementBase
	{
	public:

		Element(int index, const std::vector<Point*>& vertexList) :ElementBase(), m_vertexList(vertexList)
		{
                        utils::pamela_unused(index);
			m_family = ELEMENTS::FAMILY::POLYGON;
		}

		//Getters
		ELEMENTS::TYPE get_vtkType() const { return m_vtkType; }
		const std::vector<Point*>& get_vertexList() const { return m_vertexList; }
		std::vector<Point*>& get_vertexList() { return m_vertexList; }

		//Geometry
		virtual double get_SurfaceArea() = 0;
		virtual std::pair<std::vector <double>, std::vector <double>> get_NormalVectorAndCoordinates() = 0;
		virtual std::vector <double> get_centroidCoordinates() = 0;
		//virtual std::vector <double> get_centroidCoordinates() { return{ 0,0,0 }; }
	protected:

		std::vector<Point*> m_vertexList;

		//Functions
		virtual std::vector <double> get_BasisFunctions(double xi_1, double xi_2) = 0;
		virtual std::vector<std::vector<double>> get_BasisFunctionDerivatives(double xi_1, double xi_2) = 0;
		std::vector <double> get_RealSpaceCoordinatesFromReferenceSpace(double xi_1, double xi_2);
		std::pair<std::vector<std::vector<double>>, std::vector <double>> get_JacobianMatrixAndCrossProduct(double xi_1, double xi_2);
		std::pair<std::vector <double>, std::vector <double>> get_NormalVectorAndCoordinates(double xi_1, double xi_2);


	};

	typedef Element<ELEMENTS::FAMILY::POLYGON> Polygon;

	/**
	 * \brief From reference to real space
	 * \param xi_1
	 * \param xi_2
	 * \return
	 */
	inline std::vector<double> Element<ELEMENTS::FAMILY::POLYGON>::get_RealSpaceCoordinatesFromReferenceSpace(double xi_1, double xi_2)
	{
		std::vector<double> coordinate = { 0, 0, 0 };
		std::vector<double> basis_function = get_BasisFunctions(xi_1, xi_2);

		for (auto i = 0; i != static_cast<int>(m_vertexList.size()); ++i)
		{
			coordinate[0] = coordinate[0] + m_vertexList[i]->get_coordinates().x * basis_function[i];
			coordinate[1] = coordinate[1] + m_vertexList[i]->get_coordinates().y * basis_function[i];
			coordinate[2] = coordinate[2] + m_vertexList[i]->get_coordinates().z * basis_function[i];
		}

		return coordinate;

	}

	/**
	 * \brief
	 * \param xi_1
	 * \param xi_2
	 * \return
	 */
	inline std::pair<std::vector<std::vector<double>>, std::vector<double>> Element<ELEMENTS::FAMILY::POLYGON>::get_JacobianMatrixAndCrossProduct(double xi_1, double xi_2)
	{
		std::vector<std::vector<double>> matrix;
		auto basis_function_derivatives_matrix = get_BasisFunctionDerivatives(xi_1, xi_2);
		std::pair<std::vector<std::vector<double>>, std::vector<double>> result;

		matrix = { { 0, 0 },{ 0, 0 },{ 0, 0 } };


		for (auto i = 0; i != 2; ++i)
		{
			for (auto j = 0; j != static_cast<int>(m_vertexList.size()); ++j)
			{
				matrix[0][i] = matrix[0][i] + m_vertexList[j]->get_coordinates().x * basis_function_derivatives_matrix[j][i];
				matrix[1][i] = matrix[1][i] + m_vertexList[j]->get_coordinates().y * basis_function_derivatives_matrix[j][i];
				matrix[2][i] = matrix[2][i] + m_vertexList[j]->get_coordinates().z * basis_function_derivatives_matrix[j][i];
			}
		}

		std::vector<double> vector_1 = { matrix[0][0], matrix[1][0], matrix[2][0] };
		std::vector<double> vector_2 = { matrix[0][1], matrix[1][1], matrix[2][1] };

		std::vector<double> cross_product_vector = cross_product(vector_1, vector_2);

		result.first = matrix;
		result.second = cross_product_vector;

		return result;

	}


	/**
	 * \brief
	 * \param xi_1
	 * \param xi_2
	 * \return
	 */
	inline std::pair<std::vector<double>, std::vector<double>> Element<ELEMENTS::FAMILY::POLYGON>::get_NormalVectorAndCoordinates(double xi_1, double xi_2)
	{

		std::pair<std::vector<double>, std::vector<double>>  result;

		std::vector<double> cross_product_vector = get_JacobianMatrixAndCrossProduct(xi_1, xi_2).second;
		double vnorm = norm(cross_product_vector);
		std::vector<double> normal_vector(3);
		for (auto i = 0; i != 3; ++i)
		{
			normal_vector[i] = cross_product_vector[i] / vnorm;
		}

		std::vector<double> coordinates = get_RealSpaceCoordinatesFromReferenceSpace(xi_1, xi_2);

		result.first = normal_vector;
		result.second = coordinates;

		return result;
	}


	/**
	 * \brief
	 * \tparam elementType
	 */
	template <ELEMENTS::TYPE elementType>
	class ElementSpe<ELEMENTS::FAMILY::POLYGON, elementType> : public Element<ELEMENTS::FAMILY::POLYGON>
	{
	public:

		ElementSpe(int index, const std::vector<Point*>& vertexList) :Element(index, vertexList)
		{
			ELEMENTS::nVertex.at(static_cast<int>(m_vtkType));
			ASSERT(vertexList.size() == static_cast<unsigned int>(ELEMENTS::nVertex.at(static_cast<int>(elementType))), "Vertex list size is not compatible with the element type");
			ASSERT(ELEMENTS::TypeToFamily.at(static_cast<int>(elementType)) == m_family, "Type not compatible with family");
			m_vtkType = elementType;
		}

		//Geometry
		double get_SurfaceArea() override;
		std::vector <double> get_centroidCoordinates() override;
		//std::vector <double> get_centroidCoordinates();

	private:

		//Functions
		std::vector <double> get_BasisFunctions(double xi_1, double xi_2) override;
		std::vector<std::vector<double>> get_BasisFunctionDerivatives(double xi_1, double xi_2) override;
		std::pair<std::vector <double>, std::vector <double>> get_NormalVectorAndCoordinates() override;


	};




	//-------------------------------------------------------------------------------VTK_QUAD

	/**
	* \brief Provide surface area
	* \return
	*/
	template <>
	inline double ElementSpe<ELEMENTS::FAMILY::POLYGON, ELEMENTS::TYPE::VTK_QUAD>::get_SurfaceArea()
	{
		std::vector<double> Gauss_weight(4);
		Gauss_weight[0] = 1.;
		Gauss_weight[1] = 1.;
		Gauss_weight[2] = 1.;
		Gauss_weight[3] = 1.;

		double alpha = 1. / sqrt(3);
		std::vector<std::vector<double>> Gauss_point(4);
		Gauss_point[0] = { -alpha, -alpha };
		Gauss_point[1] = { alpha, -alpha };
		Gauss_point[2] = { alpha, alpha };
		Gauss_point[3] = { -alpha, alpha };

		double surface_area = 0;
		std::vector<double> cross_product_vector;
		for (auto i = 0; i != 4; ++i)
		{
			cross_product_vector = get_JacobianMatrixAndCrossProduct(Gauss_point[i][0], Gauss_point[i][1]).second;
			surface_area = surface_area + Gauss_weight[i] * norm(cross_product_vector);
		}

		return surface_area;
	}


	/**
	* \brief
	* \return
	*/
	template <>
	inline std::vector<double> ElementSpe<ELEMENTS::FAMILY::POLYGON, ELEMENTS::TYPE::VTK_QUAD>::get_centroidCoordinates()
	{
		return get_RealSpaceCoordinatesFromReferenceSpace(0, 0);
	}


	/**
	* \brief Provide normal vector and its location
	* \return
	*/
	template <>
	inline std::pair<std::vector<double>, std::vector<double>> ElementSpe<ELEMENTS::FAMILY::POLYGON, ELEMENTS::TYPE::VTK_QUAD>::get_NormalVectorAndCoordinates()
	{
		return Element<ELEMENTS::FAMILY::POLYGON>::get_NormalVectorAndCoordinates(0., 0.);
	}

	template <>
	inline std::vector<double> ElementSpe<ELEMENTS::FAMILY::POLYGON, ELEMENTS::TYPE::VTK_QUAD>::get_BasisFunctions(double xi_1, double xi_2)
	{
		std::vector<double> basis_functions;

		basis_functions.push_back(1. / 4. * (1 - xi_1) * (1 - xi_2));
		basis_functions.push_back(1. / 4. * (1 + xi_1) * (1 - xi_2));
		basis_functions.push_back(1. / 4. * (1 + xi_1) * (1 + xi_2));
		basis_functions.push_back(1. / 4. * (1 - xi_1) * (1 + xi_2));

		return basis_functions;

	}

	template <>
	inline std::vector<std::vector<double>> ElementSpe<ELEMENTS::FAMILY::POLYGON, ELEMENTS::TYPE::VTK_QUAD>::get_BasisFunctionDerivatives(double xi_1, double xi_2)
	{
		std::vector<double> row;
		std::vector<std::vector<double>> matrix;

		row = { -1. / 4. * (1 - xi_2), -1. / 4. * (1 - xi_1) };
		matrix.push_back(row);
		row = { 1. / 4. * (1 - xi_2), -1. / 4. * (1 + xi_1) };
		matrix.push_back(row);
		row = { 1. / 4. * (1 + xi_2), 1. / 4. * (1 + xi_1) };
		matrix.push_back(row);
		row = { -1. / 4. * (1 + xi_2), 1. / 4. * (1 - xi_1) };
		matrix.push_back(row);
		return matrix;

	}

	//-------------------------------------------------------------------------------VTK_TRIANGLE

	/**
	* \brief Provide surface area
	* \return
	*/
	template <>
	inline double ElementSpe<ELEMENTS::FAMILY::POLYGON, ELEMENTS::TYPE::VTK_TRIANGLE>::get_SurfaceArea()
	{
		double surface_area = 0;
		std::vector<double> cross_product_vector;
		cross_product_vector = get_JacobianMatrixAndCrossProduct(0, 0).second;
		surface_area = norm(cross_product_vector);

		return surface_area;
	}

	/**
	* \brief Provide normal vector and its location
	* \return
	*/
	template <>
	inline std::pair<std::vector<double>, std::vector<double>> ElementSpe<ELEMENTS::FAMILY::POLYGON, ELEMENTS::TYPE::VTK_TRIANGLE>::get_NormalVectorAndCoordinates()
	{
		return Element<ELEMENTS::FAMILY::POLYGON>::get_NormalVectorAndCoordinates(0., 0.);
	}

	template <>
	inline std::vector<double> ElementSpe<ELEMENTS::FAMILY::POLYGON, ELEMENTS::TYPE::VTK_TRIANGLE>::get_BasisFunctions(double xi_1, double xi_2)
	{
		std::vector<double> basis_functions;

		basis_functions.push_back(1 - xi_1 - xi_2);
		basis_functions.push_back(xi_1);
		basis_functions.push_back(xi_2);

		return basis_functions;

	}


	///**
	//* \brief
	//* \return
	//*/
	template <>
	inline std::vector<double> ElementSpe<ELEMENTS::FAMILY::POLYGON, ELEMENTS::TYPE::VTK_TRIANGLE>::get_centroidCoordinates()
	{
		return get_RealSpaceCoordinatesFromReferenceSpace(1. / 3., 1. / 3.);
	}


	/**
	* \brief
	* \param xi_1
	* \param xi_2
	* \return
	*/
	template <>
	inline std::vector<std::vector<double>> ElementSpe<ELEMENTS::FAMILY::POLYGON, ELEMENTS::TYPE::VTK_TRIANGLE>::get_BasisFunctionDerivatives(double xi_1, double xi_2)
	{
                utils::pamela_unused(xi_1);
                utils::pamela_unused(xi_2);
		std::vector<double> row;
		std::vector<std::vector<double>> matrix;

		row = { -1,-1 };
		matrix.push_back(row);
		row = { 1,0 };
		matrix.push_back(row);
		row = { 0,1 };
		matrix.push_back(row);
		return matrix;
	}


}
