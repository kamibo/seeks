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

#ifndef WEBSEARCH_CONFIGURATION_H
#define WEBSEARCH_CONFIGURATION_H

#include "configuration_spec.h"
#include "se_handler.h" // NSEs.

#include <bitset>

using sp::configuration_spec;

namespace seeks_plugins
{
   
#define SE_GOOGLE   1U
#define SE_CUIL     2U
#define SE_BING     4U
   
   class websearch_configuration : public configuration_spec
     {
      public:
	websearch_configuration(const std::string &filename);
	
	~websearch_configuration();
	
	// virtual
	virtual void set_default_config();
	
	virtual void handle_config_cmd(char *cmd, const uint32_t &cmd_hash, char *arg,
				       char *buf, const unsigned long &linenum);
	
	virtual void finalize_configuration();
	
	// variables.
	std::string _lang; /**< langage of the search results. */	
	int _N; /**< max number of search results per page. */
	std::bitset<NSEs> _se_enabled; /**< enabled search engines. */
	
	// others.
	double _query_context_delay; /**< delay for query context before deletion, in seconds. */
     };
   
} /* end of namespace. */

#endif
