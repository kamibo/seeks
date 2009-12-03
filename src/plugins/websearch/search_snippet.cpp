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
 **/
 
#include "search_snippet.h"
#include "miscutil.h"
#include "encode.h"

#include <iostream>

using sp::miscutil;
using sp::encode;

namespace seeks_plugins
{
   search_snippet::search_snippet()
     :_rank(0),_seeks_ir(0.0),_seeks_rank(0),_doc_type(WEBPAGE)
       {
       }
   
   search_snippet::search_snippet(const short &rank)
     :_rank(rank),_seeks_ir(0.0),_seeks_rank(0),_doc_type(WEBPAGE)
       {
       }
      
   search_snippet::~search_snippet()
     {
     }

   void search_snippet::highlight_query(std::vector<std::string> &words,
					std::string &str)
     {
	// sort words by size.
	std::sort(words.begin(),words.end(),std::greater<std::string>());
	
	// surround every of those words appearing within the
	// argument string with <b> </b> for html
	// bold format. TODO: green ?
	for (size_t i=0;i<words.size();i++)
	  {
	     if (words.at(i).length() > 2)
	       {
		  std::string bold_str = "<b>" + words.at(i) + "</b>";
		  miscutil::ci_replace_in_string(str,words.at(i),bold_str);
	       }
	  }
     }
      
   std::ostream& search_snippet::print(std::ostream &output)
     {
	output << "-----------------------------------\n";
	output << "- seeks rank: " << _seeks_rank << std::endl;
	output << "- rank: " << _rank << std::endl;
	output << "- title: " << _title << std::endl;
	output << "- url: " << _url << std::endl;
	output << "- cite: " << _cite << std::endl;
	output << "- cached: " << _cached << std::endl;
	output << "- summary: " << _summary << std::endl;
	output << "- file format: " << _file_format << std::endl;
	output << "- date: " << _date << std::endl;
	output << "- lang: " << _lang << std::endl;
	if (_doc_type == FORUM)
	  output << "- forum thread info: " << _forum_thread_info << std::endl;
	output << "-----------------------------------\n";
	
	return output;
     }
     
   std::string search_snippet::to_html() const
     {
	std::string html_content = "<li class=g><h3 class=r><a href=\"";
	html_content += _url;
	html_content += "\" class=\"l\"><em>";
	html_content += _title;
	html_content += "</em></a></h3>";
	
	if (_summary != "")
	  {
	     html_content += "<div class=\"s\">";
	     html_content += _summary;
	  }
	if (_cite != "")
	  {
	     html_content += "<br><cite>";
	     html_content += _cite;
	     html_content += "</cite>";
	  }
	if (_cached != "")
	  {
	     html_content += "<span class=\"gl\"><a href=\"";
	     html_content += _cached;
	     html_content += "\">Cached</a></span>";
	  }
	html_content += "</div></li>\n";
	
	return html_content;
     }

   std::string search_snippet::to_html_with_highlight(std::vector<std::string> &words) const
     {
	static std::string se_icon = "<img src=\"icon.png\" alt=\"icon\" width=\"12\" height=\"12\" hspace=\"2\" vspace=\"0\" align=\"\" border=\"0\" />";
	
	std::string html_content = "<li class=\"g\"><h3 class=\"r\"><a href=\"";
	html_content += _url;
	html_content += "\" class=\"l\"><em>";
	
	std::string title = _title;
	
	html_content += title;
	html_content += "</em></a>";
	
	if (_engine.to_ulong()&SE_GOOGLE)
	  {
	     std::string ggle_se_icon = se_icon;
	     miscutil::replace_in_string(ggle_se_icon,"icon","seeks_wb_google");
	     miscutil::replace_in_string(ggle_se_icon,"hspace=\"2\"","hspace=\"5\"");
	     html_content += ggle_se_icon;
	  }
	if (_engine.to_ulong()&SE_CUIL)
	  {
	     std::string cuil_se_icon = se_icon;
	     miscutil::replace_in_string(cuil_se_icon,"icon","seeks_wb_cuil");
	     html_content += cuil_se_icon;
	  }
	if (_engine.to_ulong()&SE_BING)
	  {
	     std::string bing_se_icon = se_icon;
	     miscutil::replace_in_string(bing_se_icon,"icon","seeks_wb_bing");
	     html_content += bing_se_icon;
	  }
	
	html_content += "</h3>";
		
	if (_summary != "")
	  {
	     html_content += "<div class=\"s\">";
	     
	     std::string summary = _summary;
	     search_snippet::highlight_query(words,summary);
	     
	     html_content += summary;
	  }
	else html_content += "<div>";
	
	if (_cite != "")
	  {
	     html_content += "<br><cite>";
	     html_content += _cite;
	     html_content += "</cite>";
	  }
	
	if (_cached != "")
	  {
	     html_content += "<span class=\"gl\"><a href=\"";
	     html_content += _cached;
	     html_content += " \">Cached</a></span>";
	  }
	html_content += "</div></li>\n";
		
	/* std::cout << "html_content:\n";
	 std::cout << html_content << std::endl; */
		
	return html_content;
     }

   void search_snippet::set_url(const std::string &url)
     {
	// decode url.
	char* str = encode::url_decode(url.c_str());
	_url = std::string(str);
	free(str);
     }
   
   void search_snippet::set_url(const char *url)
     {
	// decode url.
	char *str = encode::url_decode(url);
	_url = std::string(str);
	free(str);
     }
   
   void search_snippet::set_summary(const char *summary)
     {
	// encode html so tags are not interpreted.
	char* str = encode::html_encode(summary);
	_summary = std::string(str);
	free(str);
     }
      
   void search_snippet::set_summary(const std::string &summary)
     {
	
	// encode html so tags are not interpreted.
	char* str = encode::html_encode(summary.c_str());
	_summary = std::string(str);
	free(str);
     }
		   
   // static.
   void search_snippet::delete_snippets(std::vector<search_snippet*> &snippets)
     {
	size_t snippets_size = snippets.size();
	for (size_t i=0;i<snippets_size; i++)
	  delete snippets.at(i);
	snippets.clear();
     }

   void search_snippet::merge_snippets(search_snippet *s1,
				       const search_snippet *s2)
     {
	// seeks_rank is updated after merging.
	// search engine rank.
	s1->_rank = std::min(s1->_rank,s2->_rank);
	
	// search engine.
	s1->_engine |= s2->_engine;
     
	// summary.
	if (s1->_summary.length() < s2->_summary.length())
	  s1->_summary = s2->_summary;
     
	// file format.
	if (s1->_file_format.length() < s2->_file_format.length())  // we could do better here, ok enough for now.
	  s1->_file_format = s2->_file_format;
     }
      
} /* end of namespace. */
