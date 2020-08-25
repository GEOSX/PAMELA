/*
 * ------------------------------------------------------------------------------------------------------------
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 * Copyright (c) 2018-2019 Lawrence Livermore National Security LLC
 * Copyright (c) 2018-2019 The Board of Trustees of the Leland Stanford Junior University
 * Copyright (c) 2018-2019 Total, S.A
 * Copyright (c) 2019-     GEOSX Contributors
 * All rights reserved
 *
 * See top level LICENSE, COPYRIGHT, CONTRIBUTORS, NOTICE, and ACKNOWLEDGEMENTS files for details.
 * ------------------------------------------------------------------------------------------------------------
 */

// Header include
#include "StringUtils.hpp"

// Std library includes
#include <algorithm>


namespace PAMELA

    {
    void StringUtils::EclipseDataBufferToVector(std::string& input_buffer, std::vector<double>& v)
    {
        std::string star("*");
        int position_of_star(0);
        int n_repetitions(0);
        double repeated_value(0.);

        // split the input_buffer in chunks of information (spaces are used to split)
        std::istringstream split_buffer(input_buffer);
        std::string chunk;

        // Traverse through all chunks
        do
        {
            // Read a chunk from the buffer
            split_buffer >> chunk;
            if (chunk.size()>0)
            {
                // does this word contain a star "*"
                position_of_star = int(chunk.find(star));
                if(position_of_star>0)
                {
                    std::string number_times_the_value_is_repeated_as_string( chunk.substr(0, position_of_star));
                    std::string value_as_string( chunk.substr(position_of_star+1, chunk.size()));
                    n_repetitions = std::stoi(number_times_the_value_is_repeated_as_string);
                    repeated_value = std::stod(value_as_string);
                    std::fill_n(back_inserter(v), n_repetitions, repeated_value);  // append n_repetitions of the repeated_value as double
                }
                else
                {
                    // The value is only present once (no repetition)
                    repeated_value=std::stod(chunk);  // conversion to double
                    v.push_back(repeated_value);  // append the repeated_value once
                }
            }
            // While there is more to read
        } while (split_buffer);
        
    }
    
    
    void StringUtils::EclipseDataBufferToVector(std::string& input_buffer, std::vector<int>& v)
    {
        std::string star("*");
        int position_of_star(0);
        int n_repetitions(0);
        int repeated_value(0);

        // split the input_buffer in chunks of information (spaces are used to split)
        std::istringstream split_buffer(input_buffer);

        // Traverse through all chunks
        do
        {
            // Read a word
            std::string word;
            split_buffer >> word;
            if (word.size()>0)
            {
                // does this word contain a "*"
                position_of_star = int(word.find(star));
                if(position_of_star>0){
                    std::string multiplier_as_string( word.substr(0, position_of_star));
                    std::string value_as_string( word.substr(position_of_star+1, word.size()));
                    n_repetitions = std::stoi(multiplier_as_string);
                    repeated_value = std::stoi(value_as_string);
                    std::fill_n(back_inserter(v), n_repetitions, repeated_value);  // append n_repetitions of the repeated_value as int
                }
                else{
                    // The value is only present once (no repetition)
                    repeated_value=std::stoi(word);  // conversion to integer
                    v.push_back(repeated_value);  // append the repeated_value once
                }
            }
            // While there is more to read
        } while (split_buffer);
    }


	std::string StringUtils::FileToString(const std::string filepath)
	{
		std::ifstream mesh_file;
		std::string file_contents("A");

		//Open file
		mesh_file.open(filepath);
		ASSERT(mesh_file.is_open(), filepath + " Could not be open");

		//Transfer file content into string for easing broadcast
		std::stringstream buffer;
		buffer << mesh_file.rdbuf();
		//file_contents = { std::istreambuf_iterator<char>(mesh_file), std::istreambuf_iterator<char>() };

		file_contents = buffer.str();

		//Close file
		mesh_file.close();

		return file_contents;

	}


	std::string StringUtils::BinaryFileToString(const std::string filepath)
	{
		std::ifstream mesh_file;
		std::string file_contents("A");

		//Open file
		mesh_file.open(filepath, std::ios::binary);
		ASSERT(mesh_file.is_open(), filepath + " Could not be open");

		//Transfer file content into string for easing broadcast
		std::stringstream buffer;
		buffer << mesh_file.rdbuf();
		//file_contents = { std::istreambuf_iterator<char>(mesh_file), std::istreambuf_iterator<char>() };

		file_contents = buffer.str();

		//Close file
		mesh_file.close();

		return file_contents;

	}


	void StringUtils::Trim(std::string& str)
	{
		if (str.size() > 0)
		{
			int first = int(str.find_first_not_of(' '));
			int last = int(str.find_last_not_of(' '));
			if (first < 0)
			{
				str.resize(0);
			}
			else
			{
				str = str.substr(first, (last - first + 1));
			}

		}
	}


	bool StringUtils::RemoveStringAndFollowingContentFromLine(std::string ToBeRemoved, std::string& line)
	{
		// does the line have a "ToBeRemoved" character(s)
		std::size_t pos = line.find(ToBeRemoved);
        std::size_t initial_line_length = line.size();
		bool res = false;
		if (pos != std::string::npos)
		{
            res = true;
          
            std::size_t end_line_position = line.find('\n');
            if(end_line_position != std::string::npos)
            {
                // remove the character and everything afterwards
                line = line.substr(0, pos)+line.substr(end_line_position+1,initial_line_length);
            }
            else
            {
                line = line.substr(0, pos);

            }
		}
		return res;
	}

	std::string StringUtils::RemoveString(std::string ToBeRemoved, std::string data)
	{
		std::size_t pos = data.find(ToBeRemoved);

		if (pos != std::string::npos)
		{
			std::string line = data.erase(pos, ToBeRemoved.size());
			return line;
		}
		else
		{
			return data;
		}

	}


	void StringUtils::RemoveTab(std::vector<std::string>& v)
	{

		for (unsigned int i = 0; i < v.size(); i++)
		{
			std::replace(v[i].begin(), v[i].end(), '\t', ' ');
		}


	}

	void StringUtils::RemoveEndOfLine(std::vector<std::string>& v)
	{

		for (unsigned int i = 0; i < v.size(); i++)
		{
			std::replace(v[i].begin(), v[i].end(), '\r', ' ');
			std::replace(v[i].begin(), v[i].end(), '\n', ' ');
		}

	}

	void StringUtils::RemoveTab(std::string& v)
	{

		std::replace(v.begin(), v.end(), '\t', ' ');

	}

	void StringUtils::RemoveEndOfLine(std::string& v)
	{

		std::replace(v.begin(), v.end(), '\r', ' ');
		std::replace(v.begin(), v.end(), '\n', ' ');

	}

	bool StringUtils::ExtendDefault(int n, std::string& v)
	{

		std::stringstream iss(v);
		std::string sub;
		int cpt = 0;
		while (std::getline(iss, sub, ' '))  //Count number of elements in string
		{
			cpt++;
		}

		v = iss.str();

		for (int i = cpt; i < n; i++)
		{
			v = v + " DEFAULT";
		}

		RemoveExtraSpaces(v);

		return cpt <= n;


	}


	void StringUtils::RemoveExtraSpaces(std::string& v)
	{

		v.erase(std::unique(v.begin(), v.end(),
			[](char a, char b) { return a == ' ' && b == ' '; }), v.end());

	}

	void StringUtils::RemoveDoubleQuotes(std::string& v)
	{
		v.erase(remove(v.begin(), v.end(), '\"'), v.end());

	}

	void StringUtils::ExpandStarExpression(std::string& v)
	{

		int pos;
		int ntime;
		std::string ntimes;
		std::string val;
		int val_size;
		std::string star("*");
		std::string subset;
		std::string expanded;
		std::pair<int, int> bounds;


			do
			{
				//Find *, extract subset
				pos = int(v.find(star));
				if (pos != -1)
				{
					bounds = boundsSubstr(v, pos);
					subset = v.substr(bounds.first, bounds.second - bounds.first + 1);

					//Expand * expression
					ntimes = v.substr(bounds.first, pos - bounds.first);
					ntime = std::stoi(ntimes);

					if (pos == bounds.second) //Default value case
					{
						val = "DEFAULT"; // Replace 1* by DEFAULT
					}
					else
					{
						val = v.substr(pos + 1, bounds.second - pos);
					}

					//Erase * expression
					v.erase(v.begin() + bounds.first, v.begin() + bounds.second + 1);
					val_size = int(val.size());

					//Create expanded expression
					expanded.reserve(val_size*ntime);
					for (int j = 0; j < ntime; j++)
					{
						expanded.insert((val_size + 1)*j, val + " ");
					}
					expanded.erase(expanded.end() - 1, expanded.end());

					//Insert expanded expression
					v.insert(bounds.first, expanded);
					expanded.resize(0);

				}
			} while (pos != -1);

	}

	std::pair<int, int> StringUtils::boundsSubstr(std::string str, int pos)
	{
		int lower;
		int upper;

		//Upper Bound
		int i = pos;
		do
		{
			i++;
		} while ((str[i] != ' ') && (i < static_cast<int>(str.size()) - 1));

		upper = i - 1;

		if (i == static_cast<int>(str.size()) - 1) upper++;



		//Lower Bound
		int j = pos;
		do
		{
			j--;
		} while ((str[j] != ' ') && (j > 0));

		lower = j + 1;

		if (j == 0) lower--;

		return std::make_pair(lower, upper);


	}


	std::string StringUtils::GetNthWord(int n, std::string data)
	{

		std::size_t found = data.find(" ");

		if ((found == std::string::npos) && (n > 1))
			return "";

		int i = n - 1;
		while (i > 0) //Remove n-1 first words
		{
			auto pos = data.find_first_of(' ');
			if (pos == std::string::npos) data = "";
			data = data.substr(pos + 1);
			i--;
		}

		return data.substr(0, data.find(' '));

	}


	std::istream& StringUtils::safeGetline(std::istream& is, std::string& t)
	{
		t.clear();

		// The characters in the stream are read one-by-one using a std::streambuf.
		// That is faster than reading them one-by-one using the std::istream.
		// Code that uses streambuf this way must be guarded by a sentry object.
		// The sentry object performs various tasks,
		// such as thread synchronization and updating the stream state.

		std::istream::sentry se(is, true);
		std::streambuf* sb = is.rdbuf();

		for (;;)
		{
			int c = sb->sbumpc();
			switch (c)
			{
			case '\n':
				return is;
			case '\r':
				if (sb->sgetc() == '\n')
					sb->sbumpc();
				return is;
			case EOF:
				// Also handle the case when the last line has no line ending
				if (t.empty())
					is.setstate(std::ios::eofbit);
				return is;
			default:
				t += static_cast<char>(c);
			}
		}
	}

}
