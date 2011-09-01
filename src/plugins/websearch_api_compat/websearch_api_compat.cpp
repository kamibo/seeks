/**
 * The Seeks proxy and plugin framework are part of the SEEKS project.
 * Copyright (C) 2011 Emmanuel Benazera <ebenazer@seeks-project.info>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "websearch_api_compat.h"
#include "cgi.h"
#include "urlmatch.h"
#include "mrf.h"

using namespace sp;
using lsh::mrf;

namespace seeks_plugins
{

  websearch_api_compat::websearch_api_compat()
    : plugin()
  {
    _name = "websearch-api-compat";
    _version_major = "0";
    _version_minor = "3";

    // cgi_dispatchers.
    cgi_dispatcher *cgid_wb_search
    = new cgi_dispatcher("search", &websearch_api_compat::cgi_search_compat, NULL, TRUE);
    _cgi_dispatchers.push_back(cgid_wb_search);

    cgi_dispatcher *cgid_wb_search_cache
    = new cgi_dispatcher("search_cache", &websearch_api_compat::cgi_search_cache_compat, NULL, TRUE);
    _cgi_dispatchers.push_back(cgid_wb_search_cache);
  }

  websearch_api_compat::~websearch_api_compat()
  {
  }

  sp_err websearch_api_compat::cgi_search_compat(client_state *csp,
      http_response *rsp,
      const hash_map<const char*,const char*,hash<const char*>,eqstr> *parameters)
  {
    if (!parameters->empty())
      {
        // check for query.
        const char *query_str = miscutil::lookup(parameters,"q");
        if (!query_str || strlen(query_str) == 0)
          return SP_ERR_CGI_PARAMS;
        std::string query = query_str;

        // check for action.
        const char *action = miscutil::lookup(parameters,"action");
        if (!action || strcasecmp(action,"expand")==0
            || strcasecmp(action,"page")==0)
          {
            // route to /search/txt
            free(csp->_http._path);
            std::string path = "/search/txt/" + query;
            csp->_http._path = strdup(path.c_str());
            return websearch::cgi_websearch_search(csp,rsp,parameters);
          }
        else if (strcasecmp(action,"types")==0)
          {
            // route to /cluster/types
            free(csp->_http._path);
            std::string path = "/cluster/types/" + query;
            csp->_http._path = strdup(path.c_str());
            return websearch::cgi_websearch_clustered_types(csp,rsp,parameters);
          }
        else if (strcasecmp(action,"clusterize")==0)
          {
            // route to /cluster/auto
            free(csp->_http._path);
            std::string path = "/cluster/auto/" + query;
            csp->_http._path = strdup(path.c_str());
            return websearch::cgi_websearch_clusterize(csp,rsp,parameters);
          }
        else if (strcasecmp(action,"similarity")==0)
          {
            // route to /similar/txt
            free(csp->_http._path);
            std::string path = "/similar/txt/" + query;
            csp->_http._path = strdup(path.c_str());
            return websearch::cgi_websearch_similarity(csp,rsp,parameters);
          }
        else return SP_ERR_CGI_PARAMS;
      }
    else return SP_ERR_CGI_PARAMS;
  }

  sp_err websearch_api_compat::cgi_search_cache_compat(client_state *csp,
      http_response *rsp,
      const hash_map<const char*,const char*,hash<const char*>,eqstr> *parameters)
  {
    if (!parameters->empty())
      {
        // check for query.
        const char *query_str = miscutil::lookup(parameters,"q");
        if (!query_str || strlen(query_str) == 0)
          return SP_ERR_CGI_PARAMS;
        std::string query = query_str;

        // check for url.
        const char *url_str = miscutil::lookup(parameters,"url");
        if (!url_str)
          return SP_ERR_CGI_PARAMS;
        std::string url = url_str;
        std::transform(url.begin(),url.end(),url.begin(),tolower);
        std::string surl = urlmatch::strip_url(url);
        uint32_t id = mrf::mrf_single_feature(surl);
        std::string sid = miscutil::to_string(id);

        // route to /cache
        free(csp->_http._path);
        std::string path = "/cache/txt/" + query + "/" + sid;
        csp->_http._path = strdup(path.c_str());
        return websearch::cgi_websearch_search_cache(csp,rsp,parameters);
      }
    else return SP_ERR_CGI_PARAMS;
  }


  /* plugin registration */
  extern "C"
  {
    plugin* maker()
    {
      return new websearch_api_compat;
    }
  }

} /* end of namespace. */