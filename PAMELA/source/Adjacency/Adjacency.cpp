#include "Adjacency/Adjacency.hpp"
#include "Utils/Logger.hpp"
#include "Elements/Polyhedron.hpp"

namespace PAMELA
{
	Adjacency::~Adjacency()
	{
		delete m_adjacencySparseMatrix;
	}

	std::pair<std::vector<int>, std::vector<int>> Adjacency::get_SingleElementAdjacency(int i) const
	{
		auto firstColumnIndex = m_adjacencySparseMatrix->columnIndex.begin() + m_adjacencySparseMatrix->rowPtr[i];
		auto lastColumnIndex = m_adjacencySparseMatrix->columnIndex.begin() + m_adjacencySparseMatrix->rowPtr[i + 1];
		auto firstValues = m_adjacencySparseMatrix->values.begin() + m_adjacencySparseMatrix->rowPtr[i];
		auto lastValues = m_adjacencySparseMatrix->values.begin() + m_adjacencySparseMatrix->rowPtr[i + 1];
		std::vector<int> columnIndexVec(firstColumnIndex, lastColumnIndex);
		std::vector<int> valuesVec(firstValues, lastValues);
		return std::make_pair(columnIndexVec, valuesVec);
	}

	Adjacency* Adjacency::transposed(Adjacency* input)
	{
		Adjacency* adj = new Adjacency(input->get_targetFamily(), input->get_sourceFamily(), input->get_baseFamily(), input->m_targetElementCollection, input->m_sourceElementCollection, input->m_baseElementCollection);
		adj->m_adjacencySparseMatrix = CSRMatrix::transpose(input->m_adjacencySparseMatrix);
		return  adj;
	}


	Adjacency* Adjacency::multiply(Adjacency* input_lhs, Adjacency* input_rhs)
	{
		Adjacency* adj = new Adjacency(input_rhs->get_targetFamily(), input_lhs->get_sourceFamily(), input_lhs->get_targetFamily(), input_rhs->m_targetElementCollection, input_lhs->m_sourceElementCollection, input_lhs->m_targetElementCollection);
		adj->m_adjacencySparseMatrix = CSRMatrix::product(input_lhs->m_adjacencySparseMatrix, input_rhs->m_adjacencySparseMatrix);
		return adj;
	}
	
}