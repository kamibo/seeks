/**
 * The Seeks proxy and plugin framework are part of the SEEKS project.
 * Copyright (C) 2009 Emmanuel Benazera, juban@free.fr
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#ifndef CURL_MGET_H
#define CURL_MGET_H

#include <string>
#include <vector>

namespace sp
{
   typedef struct _cbget
     {
	_cbget()
	  :_url(NULL),_output(NULL),_buffer_len(0),_buffer_pos(0)
	    {};
	
	~_cbget()
	  {};
		
	const char *_url;
	char *_output;
	
	int _buffer_len;
	int _buffer_pos;
     } cbget;
   
    void* pull_one_url(void *arg_cbget);
   
   class curl_mget
     {
      public:
	curl_mget(const int &nrequests);
	~curl_mget();
	
	// direct connection.
	char** www_mget(const std::vector<std::string> &urls, const int &nrequests);
     
      public:
	int _nrequests;
	char **_outputs;
	cbget **_cbgets;
     };
} /* end of namespace */

#endif