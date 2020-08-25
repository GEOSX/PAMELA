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
// Library includes
#ifdef WITH_MPI
#include <mpi.h>
#endif
// Std library include
#include <vector>


// Std library includes
#include <cstddef>
#include "Utils/Types.hpp"

namespace PAMELA
{

	class Communicator
	{
	public:

		using dbl_vec = std::vector<double>;
		using uint_vec = std::vector<Types::uint_t>;


		static void initialize();
		static void finalize();
		static Types::uint_t worldRank();
		static Types::uint_t worldSize();
		static bool isMPIrun();

		// Create a communicator with all procs from MPI_WORLD_COMM
		Communicator();
		// Create a communicator with specified ranks from MPI_WORLD_COMM
		explicit Communicator(const uint_vec& ranks);
		// Create a communicator with specified ranks from other communicator
		explicit Communicator(const Communicator& other, const uint_vec& ranks);
		virtual ~Communicator();
		Types::uint_t rank() const;
		Types::uint_t size() const;
		bool isRoot() const;
		// Ser neighbor topology (wraps around MPI_Dist_graph_create_adjacent)
		void setNeigbors(std::vector<Types::proc_id_t>& send_procs,
			std::vector<Types::uint_t> send_weights,
			std::vector<Types::proc_id_t>& recv_procs,
			std::vector<Types::uint_t> recv_weights);
		// Neighbor exhange (wraps around MPI_Neighbor_alltoallv)
		void neighborSync(const dbl_vec& send_buf, const uint_vec& send_counts, const uint_vec& send_displs,
			dbl_vec& recv_buf, const uint_vec& recv_counts, const uint_vec& recv_displs) const;
	private:
		void common_init();
		static void handle_error(int code);
#ifdef WITH_MPI
		MPI_Comm m_comm;
		MPI_Group m_group;
#endif
		Types::uint_t m_rank;
		Types::uint_t m_size;
		bool m_root;
		bool m_nbr_set;
	};

}
