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

// Header includes
#include <Parallel/Communicator.hpp>
// Project includes
#include <Utils/Assert.hpp>
#include <Utils/Utils.hpp>

namespace PAMELA
{

	// ---------- Static methods -----------
	void Communicator::initialize()
	{
#ifdef WITH_MPI
		MPI_Init(nullptr, nullptr);
#endif
	}
	void Communicator::finalize()
	{
#ifdef WITH_MPI
		MPI_Finalize();
#endif
	}
	Types::uint_t Communicator::worldRank()
	{
#ifdef WITH_MPI
		int rank, err;
		err = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
		handle_error(err);
		return static_cast<Types::uint_t>(rank);
#else
		return 0;
#endif
	}
	Types::uint_t Communicator::worldSize()
	{
#ifdef WITH_MPI
		int size, err;
		err = MPI_Comm_size(MPI_COMM_WORLD, &size);
		handle_error(err);
		return static_cast<Types::uint_t>(size);
#else
		return 1;
#endif
	}

	bool Communicator::isMPIrun()
	{
#ifdef WITH_MPI
		return true;
#else
		return false;
#endif
	}

	// ---------- Constructors -------------
	Communicator::Communicator()
		: m_nbr_set(false)
	{
#ifdef WITH_MPI
		int err;
		err = MPI_Comm_dup(MPI_COMM_WORLD, &m_comm);
		handle_error(err);
		err = MPI_Comm_group(m_comm, &m_group);
		handle_error(err);
#endif
		common_init();
	}
	Communicator::Communicator(const uint_vec& ranks)
		: m_nbr_set(false)
	{
		Types::uint_t n = static_cast<Types::uint_t>(ranks.size());
		ASSERT(n > 0, "Empty communicator requested");
#ifdef WITH_MPI
		int err;
		std::vector<int> iranks(ranks.begin(), ranks.end());
		MPI_Comm world_comm = MPI_COMM_WORLD;
		MPI_Group world_grp;
		err = MPI_Comm_group(world_comm, &world_grp);
		handle_error(err);
		err = MPI_Group_incl(world_grp, n, iranks.data(), &m_group);
		handle_error(err);
		err = MPI_Comm_create(MPI_COMM_WORLD, m_group, &m_comm);
		handle_error(err);
#endif
		common_init();
	}
	Communicator::Communicator(const Communicator& other, const uint_vec& ranks)
		: m_nbr_set(false)
	{
		Types::uint_t n = static_cast<Types::uint_t>(ranks.size());
		ASSERT(n > 0, "Empty communicator requested");
		ASSERT(n <= static_cast<Types::uint_t>(other.m_size), "Communicator with too many procs requested");
#ifdef WITH_MPI
		int err;
		std::vector<int> iranks(ranks.begin(), ranks.end());
		err = MPI_Group_incl(other.m_group, n, iranks.data(), &m_group);
		handle_error(err);
		err = MPI_Comm_create(MPI_COMM_WORLD, m_group, &m_comm);
		handle_error(err);
#endif
		common_init();
	}
	Communicator::~Communicator()
	{
#ifdef WITH_MPI
		int err;
		err = MPI_Comm_free(&m_comm);
		handle_error(err);
		err = MPI_Group_free(&m_group);
		handle_error(err);
#endif
	}
	Types::uint_t Communicator::rank() const
	{
		return m_rank;
	}
	Types::uint_t Communicator::size() const
	{
		return m_size;
	}
	bool Communicator::isRoot() const
	{
		return m_root;
	}
	void Communicator::setNeigbors(std::vector<Types::proc_id_t>& send_procs, std::vector<Types::uint_t> send_weights,
		std::vector<Types::proc_id_t>& recv_procs, std::vector<Types::uint_t> recv_weights)
	{
#ifdef WITH_MPI
		ASSERT(send_procs.size() == send_weights.size(), "Vector size mismatch in setNeighbors()");
		ASSERT(recv_procs.size() == recv_weights.size(), "Vector size mismatch in setNeighbors()");
		// Casting unsigned to signed ints used in MPI calls (ugh...)
		// Reverse terminology: MPI sources are processes we recieve from (recv_procs),
		// MPI desitnations are processes we send to (send_procs); same for weights.
		std::vector<int> src(recv_procs.begin(), recv_procs.end());
		std::vector<int> dest(send_procs.begin(), send_procs.end());
		std::vector<int> src_weights(recv_weights.begin(), recv_weights.end());
		std::vector<int> dest_weights(send_weights.begin(), send_weights.end());
		const int nsrc = static_cast<int>(src.size());
		const int ndest = static_cast<int>(dest.size());
		MPI_Comm old_comm = m_comm;
		int err = MPI_Dist_graph_create_adjacent(m_comm,
			nsrc, src.data(), src_weights.data(),
			ndest, dest.data(), dest_weights.data(),
			MPI_INFO_NULL, 0, &m_comm);
		handle_error(err);
		err = MPI_Comm_free(&old_comm);
		handle_error(err);

#else

		(void)send_procs;
		(void)send_weights;
		(void)recv_procs;
		(void)recv_weights;

#endif
		m_nbr_set = true;
	}
	void Communicator::neighborSync(const dbl_vec& send_buf, const uint_vec& send_counts, const uint_vec& send_displs,
		dbl_vec& recv_buf, const uint_vec& recv_counts, const uint_vec& recv_displs) const
	{
                utils::pamela_unused(send_buf);
                utils::pamela_unused(recv_buf);
		if (!m_nbr_set)
			throw std::string("Neigbors not set on communicator");
#ifdef WITH_MPI
		if (m_size == 1)
			return;
		// If MPI-3 is available, use neighbor exchange functions directly
		// Otherwise mimic same behavior using all-to-all exchange 
		// by inserting a bunch of zero counts for non-neighbor procs
#if MPI_VERSION >= 3
	// Casting unsigned to signed ints used in MPI calls (ugh...)
		std::vector<int> send_cnt(send_counts.begin(), send_counts.end());
		std::vector<int> send_disp(send_displs.begin(), send_displs.end());
		std::vector<int> recv_cnt(recv_counts.begin(), recv_counts.end());
		std::vector<int> recv_disp(recv_displs.begin(), recv_displs.end());
                //TODO this code seems weird : dataAtTime is not a member function
                //of std::vector...
		//int err = MPI_Neighbor_alltoallv(send_buf.data(), send_cnt.dataAtTime(), send_disp.dataAtTime(), MPI_DOUBLE,
			//recv_buf.data(), recv_cnt.dataAtTime(), recv_disp.dataAtTime(), MPI_DOUBLE, m_comm);
		//handle_error(err);
#else
	// Extract neighbor topology information
	// Reverse terminology: MPI sources are processes we recieve from (recv_procs),
	// MPI destinations are processes we send to (send_procs); same for weights.
		std::vector<int> send(m_size, -1), recv(m_size, -1), send_w(m_size, 0), recv_w(m_size, 0);
		int err = MPI_Dist_graph_neighbors(m_comm,
			m_size, recv.data(), recv_w.data(),
			m_size, send.data(), send_w.data());
		handle_error(err);
		std::vector<int> send_cnt, send_disp;
		for (int iproc = 0, inbr = 0; iproc < static_cast<int>(m_size); ++iproc)
		{
			if (send[inbr] == iproc)
			{
				send_cnt.push_back(send_counts[inbr]);
				send_disp.push_back(send_displs[inbr]);
				++inbr;
			}
			else
			{
				send_cnt.push_back(0);
				send_disp.push_back(0);
			}
		}
		std::vector<int> recv_cnt, recv_disp;
		for (int iproc = 0, inbr = 0; iproc < static_cast<int>(m_size); ++iproc)
		{
			if (recv[inbr] == iproc)
			{
				recv_cnt.push_back(recv_counts[inbr]);
				recv_disp.push_back(recv_displs[inbr]);
				++inbr;
			}
			else
			{
				recv_cnt.push_back(0);
				recv_disp.push_back(0);
			}
		}
		err = MPI_Alltoallv(send_buf.data(), send_cnt.data(), send_disp.data(), MPI_DOUBLE,
			recv_buf.data(), recv_cnt.data(), recv_disp.data(), MPI_DOUBLE, m_comm);
		handle_error(err);
#endif // MPI_VERSION >= 3

#else

		(void)send_buf;
		(void)send_counts;
		(void)send_displs;
		(void)recv_buf;
		(void)recv_counts;
		(void)recv_displs;
#endif // WITH_MPI
	}
	void Communicator::common_init()
	{
#ifdef WITH_MPI
		int size, rank, err;
		err = MPI_Comm_size(m_comm, &size);
		handle_error(err);
		err = MPI_Comm_rank(m_comm, &rank);
		handle_error(err);
		m_size = static_cast<Types::uint_t>(size);
		m_rank = static_cast<Types::uint_t>(rank);
#else
		m_size = 1;
		m_rank = 0;
#endif
		m_root = (m_rank == 0);
	}
	void Communicator::handle_error(int code)
	{
#ifdef WITH_MPI
		if (code == MPI_SUCCESS)
			return;
		char err_str[MPI_MAX_ERROR_STRING];
		int len;
		MPI_Error_string(code, err_str, &len);
		// TODO: proper handling
		throw std::string(err_str);
#else
		(void)code;
#endif
	}

}
