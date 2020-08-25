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
#include "Line.hpp"
#include "Elements/Polygon.hpp"
#include "Utils/Assert.hpp"
#include "Elements/Element.hpp"

namespace PAMELA
{

  template <typename T>
  class Collection;

  template <>
  class Element<ELEMENTS::FAMILY::POLYHEDRON> : public ElementBase
  {

  public:

    Element(int index, const std::vector<Point*>& vertexList) :ElementBase(), m_vertexList(vertexList)
    {
                        utils::pamela_unused(index);
      m_family = ELEMENTS::FAMILY::POLYHEDRON;
    }

    virtual std::vector<Polygon*> CreateFaces() = 0;
    //Getter
    const std::vector<Point*>& get_vertexList() const { return m_vertexList; }
    std::vector<Point*>& get_vertexList() { return m_vertexList; }

    //Geometry
    virtual double get_Volume() = 0;
    virtual std::vector <double> get_centroidCoordinates() = 0;

  protected:

    std::vector<Point*> m_vertexList;

    virtual std::vector <double> get_BasisFunctions(double xi_1, double xi_2, double xi_3) = 0;
    virtual std::vector<std::vector<double>> get_BasisFunctionDerivatives(double xi_1, double xi_2, double xi_3) = 0;
    std::vector <double> get_RealSpaceCoordinatesFromReferenceSpace(double xi_1, double xi_2, double xi_3);
    std::pair<std::vector<std::vector<double>>, double> get_JacobianMatrixAndDeterminant(double xi_1, double xi_2, double xi_3);

  };


  /**
   * \brief
   * \param xi_1
   * \param xi_2
   * \param xi_3
   * \return
   */
  inline std::vector<double> Element<ELEMENTS::FAMILY::POLYHEDRON>::get_RealSpaceCoordinatesFromReferenceSpace(double xi_1, double xi_2, double xi_3)
  {

    std::vector<double> coordinate = { 0, 0, 0 };
    std::vector<double> basis_function = get_BasisFunctions(xi_1, xi_2, xi_3);

    int nbVertices = static_cast<int>(m_vertexList.size());
    for (auto i = 0; i != nbVertices; ++i)
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
   * \param xi_3
   * \return
   */
  inline std::pair<std::vector<std::vector<double>>, double> Element<ELEMENTS::FAMILY::POLYHEDRON>::get_JacobianMatrixAndDeterminant(double xi_1, double xi_2, double xi_3)
  {

    std::vector<std::vector<double>> basis_function_derivatives_matrix = get_BasisFunctionDerivatives(xi_1, xi_2, xi_3);
    std::vector<std::vector<double>>matrix = { { 0, 0, 0 },{ 0, 0, 0 },{ 0, 0, 0 } };

    for (auto i = 0; i != 3; ++i)
    {
      for (auto j = 0; j != static_cast<int>(m_vertexList.size()); ++j)
      {
        matrix[0][i] = matrix[0][i] + m_vertexList[j]->get_coordinates().x * basis_function_derivatives_matrix[j][i];
        matrix[1][i] = matrix[1][i] + m_vertexList[j]->get_coordinates().y * basis_function_derivatives_matrix[j][i];
        matrix[2][i] = matrix[2][i] + m_vertexList[j]->get_coordinates().z * basis_function_derivatives_matrix[j][i];
      }
    }
    double determinant = matrix[0][0] * (matrix[1][1] * matrix[2][2] - matrix[2][1] * matrix[1][2])
      - matrix[0][1] * (matrix[1][0] * matrix[2][2] - matrix[2][0] * matrix[1][2])
      + matrix[0][2] * (matrix[1][0] * matrix[2][1] - matrix[2][0] * matrix[1][1]);


    std::pair<std::vector<std::vector<double>>, double> result;
    result.first = matrix;
    result.second = determinant;

    return result;
  }

  typedef Element<ELEMENTS::FAMILY::POLYHEDRON> Polyhedron;


  template <ELEMENTS::TYPE elementType>
  class ElementSpe<ELEMENTS::FAMILY::POLYHEDRON, elementType> : public Element<ELEMENTS::FAMILY::POLYHEDRON>
  {
  public:

    ElementSpe(int index, const std::vector<Point*>& vertexList) :Element(index, vertexList)
    {
      ELEMENTS::nVertex.at(static_cast<int>(m_vtkType));
      ASSERT(vertexList.size() == static_cast<unsigned int>(ELEMENTS::nVertex.at(static_cast<int>(elementType))), "Vertex list size is not compatible with the element type");
      ASSERT(ELEMENTS::TypeToFamily.at(static_cast<int>(elementType)) == m_family, "Type not compatible with family");
      m_vtkType = elementType;
    }

    //Actions
    std::vector<Polygon*> CreateFaces() override;

    //Geometry
    double get_Volume() override;
    std::vector<double> get_centroidCoordinates() override;


  private:

    //Functions
    std::vector <double> get_BasisFunctions(double xi_1, double xi_2, double xi_3) override;
    std::vector<std::vector<double>> get_BasisFunctionDerivatives(double xi_1, double xi_2, double xi_3) override;

  };




  //////// VTK_TETRA


  template <>
  inline double ElementSpe<ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::TYPE::VTK_TETRA>::get_Volume()
  {
    return  std::abs(1. / 6. * get_JacobianMatrixAndDeterminant(0, 0, 0).second);
  }


  /**
  * \brief
  * \return
  */
  template <>
  inline std::vector<double> ElementSpe<ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::TYPE::VTK_TETRA>::get_centroidCoordinates()
  {
    return get_RealSpaceCoordinatesFromReferenceSpace(1. / 4., 1. / 4., 1. / 4.);
  }


  template <>
  inline std::vector<double> ElementSpe<ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::TYPE::VTK_TETRA>::get_BasisFunctions(double xi_1, double xi_2, double xi_3)
  {
    std::vector<double> basis_functions;

    basis_functions.push_back(1 - xi_1 - xi_2 - xi_3);
    basis_functions.push_back(xi_1);
    basis_functions.push_back(xi_2);
    basis_functions.push_back(xi_3);


    return basis_functions;
  }

  template <>
  inline std::vector<std::vector<double>> ElementSpe<ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::TYPE::VTK_TETRA>::get_BasisFunctionDerivatives(double xi_1, double xi_2, double xi_3)
  {
                utils::pamela_unused(xi_1);
                utils::pamela_unused(xi_2);
                utils::pamela_unused(xi_3);
    std::vector<double> row;
    std::vector<std::vector<double>> matrix;

    row = { -1, -1, -1 };
    matrix.push_back(row);
    row = { 1, 0, 0 };
    matrix.push_back(row);
    row = { 0, 1, 0 };
    matrix.push_back(row);
    row = { 0, 0, 1 };
    matrix.push_back(row);

    return matrix;

  }


  //////// VTK_HEXAHEDRON

  template <>
  inline double ElementSpe<ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::TYPE::VTK_HEXAHEDRON>::get_Volume()
  {

    std::vector<double> Gauss_weight(8);
    Gauss_weight[0] = 1.;
    Gauss_weight[1] = 1.;
    Gauss_weight[2] = 1.;
    Gauss_weight[3] = 1.;
    Gauss_weight[4] = 1.;
    Gauss_weight[5] = 1.;
    Gauss_weight[6] = 1.;
    Gauss_weight[7] = 1.;

    double alpha = 1. / sqrt(3);
    std::vector<std::vector<double>>  Gauss_point(8);
    Gauss_point[0] = { -alpha, -alpha, -alpha };
    Gauss_point[1] = { alpha, -alpha, -alpha };
    Gauss_point[2] = { alpha, alpha, -alpha };
    Gauss_point[3] = { -alpha, alpha, -alpha };
    Gauss_point[4] = { -alpha, -alpha, alpha };
    Gauss_point[5] = { alpha, -alpha, alpha };
    Gauss_point[6] = { alpha, alpha, alpha };
    Gauss_point[7] = { -alpha, alpha, alpha };

    double volume = 0;
    for (auto i = 0; i != 8; ++i)
    {
      volume = volume + Gauss_weight[i] * get_JacobianMatrixAndDeterminant(Gauss_point[i][0], Gauss_point[i][1], Gauss_point[i][2]).second;
    }

    return std::fabs(volume);
  }


  /**
  * \brief
  * \return
  */
  template <>
  inline std::vector<double> ElementSpe<ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::TYPE::VTK_HEXAHEDRON>::get_centroidCoordinates()
  {
    return get_RealSpaceCoordinatesFromReferenceSpace(0., 0., 0.);
  }


  /**
   * \brief
   * \param xi_1
   * \param xi_2
   * \param xi_3
   * \return
   */
  template <>
  inline std::vector<double> ElementSpe<ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::TYPE::VTK_HEXAHEDRON>::get_BasisFunctions(double xi_1, double xi_2, double xi_3)
  {
    std::vector<double> basis_functions;

    basis_functions.push_back(1. / 8. * (1 - xi_1) * (1 - xi_2) * (1 - xi_3));
    basis_functions.push_back(1. / 8. * (1 + xi_1) * (1 - xi_2) * (1 - xi_3));
    basis_functions.push_back(1. / 8. * (1 + xi_1) * (1 + xi_2) * (1 - xi_3));
    basis_functions.push_back(1. / 8. * (1 - xi_1) * (1 + xi_2) * (1 - xi_3));
    basis_functions.push_back(1. / 8. * (1 - xi_1) * (1 - xi_2) * (1 + xi_3));
    basis_functions.push_back(1. / 8. * (1 + xi_1) * (1 - xi_2) * (1 + xi_3));
    basis_functions.push_back(1. / 8. * (1 + xi_1) * (1 + xi_2) * (1 + xi_3));
    basis_functions.push_back(1. / 8. * (1 - xi_1) * (1 + xi_2) * (1 + xi_3));


    return basis_functions;
  }

  /**
   * \brief
   * \param xi_1
   * \param xi_2
   * \param xi_3
   * \return
   */
  template <>
  inline std::vector<std::vector<double>> ElementSpe<ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::TYPE::VTK_HEXAHEDRON>::get_BasisFunctionDerivatives(double xi_1, double xi_2, double xi_3)
  {
    std::vector<double> row;
    std::vector<std::vector<double>> matrix;

    row = { -1. / 8. * (1 - xi_2) * (1 - xi_3) , -1. / 8. * (1 - xi_1) * (1 - xi_3) , -1. / 8. * (1 - xi_1) * (1 - xi_2) };
    matrix.push_back(row);
    row = { 1. / 8. * (1 - xi_2) * (1 - xi_3) , -1. / 8. * (1 + xi_1) * (1 - xi_3) , -1. / 8. * (1 + xi_1) * (1 - xi_2) };
    matrix.push_back(row);
    row = { 1. / 8. * (1 + xi_2) * (1 - xi_3) , 1. / 8. * (1 + xi_1) * (1 - xi_3) , -1. / 8. * (1 + xi_1) * (1 + xi_2) };
    matrix.push_back(row);
    row = { -1. / 8. * (1 + xi_2) * (1 - xi_3) , 1. / 8. * (1 - xi_1) * (1 - xi_3) , -1. / 8. * (1 - xi_1) * (1 + xi_2) };
    matrix.push_back(row);
    row = { -1. / 8. * (1 - xi_2) * (1 + xi_3) , -1. / 8. * (1 - xi_1) * (1 + xi_3) , 1. / 8. * (1 - xi_1) * (1 - xi_2) };
    matrix.push_back(row);
    row = { 1. / 8. * (1 - xi_2) * (1 + xi_3) , -1. / 8. * (1 + xi_1) * (1 + xi_3) , 1. / 8. * (1 + xi_1) * (1 - xi_2) };
    matrix.push_back(row);
    row = { 1. / 8. * (1 + xi_2) * (1 + xi_3) , 1. / 8. * (1 + xi_1) * (1 + xi_3) , 1. / 8. * (1 + xi_1) * (1 + xi_2) };
    matrix.push_back(row);
    row = { -1. / 8. * (1 + xi_2) * (1 + xi_3) , 1. / 8. * (1 - xi_1) * (1 + xi_3) , 1. / 8. * (1 - xi_1) * (1 + xi_2) };
    matrix.push_back(row);

    return matrix;

  }


  //////// VTK_WEDGE


  template <>
  inline double ElementSpe<ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::TYPE::VTK_WEDGE>::get_Volume()
  {
    LOGERROR("NOT SUPPORTED YET");
    return 0;	//TODO
  }


  /**
  * \brief
  * \return
  */
  template <>
  inline std::vector<double> ElementSpe<ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::TYPE::VTK_WEDGE>::get_centroidCoordinates()
  {
    LOGERROR("NOT SUPPORTED YET");
    return{ 0,0,0,0,0,0 };  //TODO
  }


  /**
  * \brief
  * \param xi_1
  * \param xi_2
  * \param xi_3
  * \return
  */
  template <>
  inline std::vector<double> ElementSpe<ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::TYPE::VTK_WEDGE>::get_BasisFunctions(double xi_1, double xi_2, double xi_3)
  {
                utils::pamela_unused(xi_1);
                utils::pamela_unused(xi_2);
                utils::pamela_unused(xi_3);
    LOGERROR("NOT SUPPORTED YET");
    return{ 0,0,0,0,0,0 };  //TODO
  }

  /**
  * \brief
  * \param xi_1
  * \param xi_2
  * \param xi_3
  * \return
  */
  template <>
  inline std::vector<std::vector<double>> ElementSpe<ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::TYPE::VTK_WEDGE>::get_BasisFunctionDerivatives(double xi_1, double xi_2, double xi_3)
  {
                utils::pamela_unused(xi_1);
                utils::pamela_unused(xi_2);
                utils::pamela_unused(xi_3);
    std::vector<double> row;
    std::vector<std::vector<double>> matrix;
    LOGERROR("NOT SUPPORTED YET");
    return matrix;			//TODO

  }


  //////// VTK_PYRAMID

  template <>
  inline double ElementSpe<ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::TYPE::VTK_PYRAMID>::get_Volume()
  {
    LOGERROR("NOT SUPPORTED YET");
    return 0;	//TODO
  }


  /**
  * \brief
  * \return
  */
  template <>
  inline std::vector<double> ElementSpe<ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::TYPE::VTK_PYRAMID>::get_centroidCoordinates()
  {
    LOGERROR("NOT SUPPORTED YET");
    return{ 0,0,0,0,0 };  //TODO
  }


  /**
  * \brief
  * \param xi_1
  * \param xi_2
  * \param xi_3
  * \return
  */
  template <>
  inline std::vector<double> ElementSpe<ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::TYPE::VTK_PYRAMID>::get_BasisFunctions(double xi_1, double xi_2, double xi_3)
  {
                utils::pamela_unused(xi_1);
                utils::pamela_unused(xi_2);
                utils::pamela_unused(xi_3);
    LOGERROR("NOT SUPPORTED YET");
    return{ 0,0,0,0,0 };  //TODO
  }

  /**
  * \brief
  * \param xi_1
  * \param xi_2
  * \param xi_3
  * \return
  */
  template <>
  inline std::vector<std::vector<double>> ElementSpe<ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::TYPE::VTK_PYRAMID>::get_BasisFunctionDerivatives(double xi_1, double xi_2, double xi_3)
  {
                utils::pamela_unused(xi_1);
                utils::pamela_unused(xi_2);
                utils::pamela_unused(xi_3);
    std::vector<double> row;
    std::vector<std::vector<double>> matrix;
    LOGERROR("NOT SUPPORTED YET");
    return matrix;			//TODO

  }


  //////// VTK_TETRA
  template <>
  inline std::vector<Polygon*> ElementSpe<ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::TYPE::VTK_TETRA >::CreateFaces()
  {
    std::vector<Point*> vertexTemp = { nullptr,nullptr,nullptr };
    std::vector<Polygon*> faceTemp;

    //Create face 0
    vertexTemp[0] = m_vertexList[0];
    vertexTemp[1] = m_vertexList[1];
    vertexTemp[2] = m_vertexList[2];
    auto face0 = new ElementSpe<ELEMENTS::FAMILY::POLYGON, ELEMENTS::TYPE::VTK_TRIANGLE>(-1, vertexTemp);
    faceTemp.push_back(face0);

    //Create face 1
    vertexTemp[0] = m_vertexList[0];
    vertexTemp[1] = m_vertexList[1];
    vertexTemp[2] = m_vertexList[3];
    auto face1 = new ElementSpe<ELEMENTS::FAMILY::POLYGON, ELEMENTS::TYPE::VTK_TRIANGLE>(-1, vertexTemp);
    faceTemp.push_back(face1);

    //Create face 2
    vertexTemp[0] = m_vertexList[1];
    vertexTemp[1] = m_vertexList[2];
    vertexTemp[2] = m_vertexList[3];
    auto face2 = new ElementSpe<ELEMENTS::FAMILY::POLYGON, ELEMENTS::TYPE::VTK_TRIANGLE>(-1, vertexTemp);
    faceTemp.push_back(face2);

    //Create face 3
    vertexTemp[0] = m_vertexList[2];
    vertexTemp[1] = m_vertexList[0];
    vertexTemp[2] = m_vertexList[3];
    auto face3 = new ElementSpe<ELEMENTS::FAMILY::POLYGON, ELEMENTS::TYPE::VTK_TRIANGLE>(-1, vertexTemp);
    faceTemp.push_back(face3);

    return faceTemp;
  }

  //////// VTK_HEXA
  template <>
  inline std::vector<Polygon*> ElementSpe<ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::TYPE::VTK_HEXAHEDRON >::CreateFaces()
  {
    std::vector<Point*> vertexTemp = { nullptr,nullptr,nullptr,nullptr };
    std::vector<Polygon*> faceTemp;

    //Create face 0
    vertexTemp[0] = m_vertexList[0];
    vertexTemp[1] = m_vertexList[1];
    vertexTemp[2] = m_vertexList[5];
    vertexTemp[3] = m_vertexList[4];
    auto face0 = new ElementSpe<ELEMENTS::FAMILY::POLYGON, ELEMENTS::TYPE::VTK_QUAD>(-1, vertexTemp);
    faceTemp.push_back(face0);

    //Create face 1
    vertexTemp[0] = m_vertexList[1];
    vertexTemp[1] = m_vertexList[2];
    vertexTemp[2] = m_vertexList[6];
    vertexTemp[3] = m_vertexList[5];
    auto face1 = new ElementSpe<ELEMENTS::FAMILY::POLYGON, ELEMENTS::TYPE::VTK_QUAD>(-1, vertexTemp);
    faceTemp.push_back(face1);

    //Create face 2
    vertexTemp[0] = m_vertexList[2];
    vertexTemp[1] = m_vertexList[3];
    vertexTemp[2] = m_vertexList[7];
    vertexTemp[3] = m_vertexList[6];
    auto face2 = new ElementSpe<ELEMENTS::FAMILY::POLYGON, ELEMENTS::TYPE::VTK_QUAD>(-1, vertexTemp);
    faceTemp.push_back(face2);

    //Create face 3
    vertexTemp[0] = m_vertexList[3];
    vertexTemp[1] = m_vertexList[0];
    vertexTemp[2] = m_vertexList[4];
    vertexTemp[3] = m_vertexList[7];
    auto face3 = new ElementSpe<ELEMENTS::FAMILY::POLYGON, ELEMENTS::TYPE::VTK_QUAD>(-1, vertexTemp);
    faceTemp.push_back(face3);

    //Create face 4
    vertexTemp[0] = m_vertexList[4];
    vertexTemp[1] = m_vertexList[5];
    vertexTemp[2] = m_vertexList[6];
    vertexTemp[3] = m_vertexList[7];
    auto face4 = new ElementSpe<ELEMENTS::FAMILY::POLYGON, ELEMENTS::TYPE::VTK_QUAD>(-1, vertexTemp);
    faceTemp.push_back(face4);

    //Create face 5
    vertexTemp[0] = m_vertexList[0];
    vertexTemp[1] = m_vertexList[1];
    vertexTemp[2] = m_vertexList[2];
    vertexTemp[3] = m_vertexList[3];
    auto face5 = new ElementSpe<ELEMENTS::FAMILY::POLYGON, ELEMENTS::TYPE::VTK_QUAD>(-1, vertexTemp);
    faceTemp.push_back(face5);

    return faceTemp;
  }


  //VTK_WEDGE
  template <>
  inline std::vector<Polygon*> ElementSpe<ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::TYPE::VTK_WEDGE>::CreateFaces()
  {
    std::vector<Point*> vertexTemp3 = { nullptr,nullptr,nullptr };
    std::vector<Point*> vertexTemp4 = { nullptr,nullptr,nullptr,nullptr };
    std::vector<Polygon*> faceTemp;

    //Create face 0
    vertexTemp3[0] = m_vertexList[0];
    vertexTemp3[1] = m_vertexList[1];
    vertexTemp3[2] = m_vertexList[2];
    auto face0 = new ElementSpe<ELEMENTS::FAMILY::POLYGON, ELEMENTS::TYPE::VTK_TRIANGLE>(-1, vertexTemp3);
    faceTemp.push_back(face0);

    //Create face 1
    vertexTemp3[0] = m_vertexList[3];
    vertexTemp3[1] = m_vertexList[4];
    vertexTemp3[2] = m_vertexList[5];
    auto face1 = new ElementSpe<ELEMENTS::FAMILY::POLYGON, ELEMENTS::TYPE::VTK_TRIANGLE>(-1, vertexTemp3);
    faceTemp.push_back(face1);

    //Create face 2
    vertexTemp4[0] = m_vertexList[0];
    vertexTemp4[1] = m_vertexList[1];
    vertexTemp4[2] = m_vertexList[4];
    vertexTemp4[3] = m_vertexList[3];
    auto face2 = new ElementSpe<ELEMENTS::FAMILY::POLYGON, ELEMENTS::TYPE::VTK_QUAD>(-1, vertexTemp4);
    faceTemp.push_back(face2);

    //Create face 3
    vertexTemp4[0] = m_vertexList[3];
    vertexTemp4[1] = m_vertexList[0];
    vertexTemp4[2] = m_vertexList[2];
    vertexTemp4[3] = m_vertexList[5];
    auto face3 = new ElementSpe<ELEMENTS::FAMILY::POLYGON, ELEMENTS::TYPE::VTK_QUAD>(-1, vertexTemp4);
    faceTemp.push_back(face3);

    //Create face 4
    vertexTemp4[0] = m_vertexList[4];
    vertexTemp4[1] = m_vertexList[1];
    vertexTemp4[2] = m_vertexList[2];
    vertexTemp4[3] = m_vertexList[5];
    auto face4 = new ElementSpe<ELEMENTS::FAMILY::POLYGON, ELEMENTS::TYPE::VTK_QUAD>(-1, vertexTemp4);
    faceTemp.push_back(face4);

    return faceTemp;
  }

  template <>
  inline std::vector<Polygon*> ElementSpe<ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::TYPE::VTK_PYRAMID >::CreateFaces()
  {
    std::vector<Point*> vertexTemp3 = { nullptr,nullptr,nullptr };
    std::vector<Point*> vertexTemp4 = { nullptr,nullptr,nullptr,nullptr };
    std::vector<Polygon*> faceTemp;

    //Create face 0
    vertexTemp3[0] = m_vertexList[3];
    vertexTemp3[1] = m_vertexList[0];
    vertexTemp3[2] = m_vertexList[4];
    auto face0 = new ElementSpe<ELEMENTS::FAMILY::POLYGON, ELEMENTS::TYPE::VTK_TRIANGLE>(-1, vertexTemp3);
    faceTemp.push_back(face0);

    //Create face 1
    vertexTemp3[0] = m_vertexList[0];
    vertexTemp3[1] = m_vertexList[1];
    vertexTemp3[2] = m_vertexList[4];
    auto face1 = new ElementSpe<ELEMENTS::FAMILY::POLYGON, ELEMENTS::TYPE::VTK_TRIANGLE>(-1, vertexTemp3);
    faceTemp.push_back(face1);

    //Create face 2
    vertexTemp3[0] = m_vertexList[4];
    vertexTemp3[1] = m_vertexList[1];
    vertexTemp3[2] = m_vertexList[2];
    auto face2 = new ElementSpe<ELEMENTS::FAMILY::POLYGON, ELEMENTS::TYPE::VTK_TRIANGLE>(-1, vertexTemp3);
    faceTemp.push_back(face2);

    //Create face 3
    vertexTemp3[0] = m_vertexList[2];
    vertexTemp3[1] = m_vertexList[3];
    vertexTemp3[2] = m_vertexList[4];
    auto face3 = new ElementSpe<ELEMENTS::FAMILY::POLYGON, ELEMENTS::TYPE::VTK_TRIANGLE>(-1, vertexTemp3);
    faceTemp.push_back(face3);

    //Create face 4
    vertexTemp4[0] = m_vertexList[0];
    vertexTemp4[1] = m_vertexList[1];
    vertexTemp4[2] = m_vertexList[2];
    vertexTemp4[3] = m_vertexList[3];
    auto face4 = new ElementSpe<ELEMENTS::FAMILY::POLYGON, ELEMENTS::TYPE::VTK_QUAD>(-1, vertexTemp4);
    faceTemp.push_back(face4);

    return faceTemp;
  }


}
