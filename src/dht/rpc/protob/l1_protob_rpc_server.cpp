/**
 * This is the p2p messaging component of the Seeks project,
 * a collaborative websearch overlay network.
 *
 * Copyright (C) 2010  Emmanuel Benazera, juban@free.fr
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
  
#include "l1_protob_rpc_server.h"
#include "l1_protob_wrapper.h"
#include "DHTNode.h"
#include "errlog.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

using sp::errlog;

namespace dht
{
   l1_protob_rpc_server::l1_protob_rpc_server(const std::string &hostname, const short &port,
					      DHTNode *pnode)
     :l1_rpc_server(hostname,port,pnode)
       {
       }
   
   l1_protob_rpc_server::~l1_protob_rpc_server()
     {
     }
   
   dht_err l1_protob_rpc_server::serve_response(const std::string &msg,
						const std::string &addr,
						std::string &resp_msg)
     {
	//debug
	//std::cerr << "[Debug]: executing serve_response\n";
	//debug
	
	// deserialize query message.
	l1::l1_query l1q;
	try 
	  {
	     l1_protob_wrapper::deserialize(msg,&l1q);
	  }
	catch (l1_fail_deserialize_exception ex) 
	  {
	     errlog::log_error(LOG_LEVEL_DHT, "l1_protob_rpc_server::serve_response exception %s", ex.what().c_str());
	     return DHT_ERR_MSG;
	  }
			
	// read query.
	uint32_t layer_id;
	uint32_t fct_id;
	DHTKey recipient_key, sender_key, node_key;
	NetAddress recipient_na, sender_na;
	l1_protob_wrapper::read_l1_query(&l1q,layer_id,fct_id,recipient_key,
					 recipient_na,sender_key,sender_na,node_key);
	// check on the layer id.
	if (layer_id != 1)
	  {
	     int status = DHT_ERR_OK;
	     lx_server_response(fct_id,recipient_key,recipient_na,sender_key,sender_na,node_key,status,resp_msg);
	     return DHT_ERR_OK;
	  }
	
	// check on sender address, if specified, that sender is not lying.
	if (sender_na.empty())
	  sender_na = NetAddress(addr,0);
	else
	  {
	     struct addrinfo hints, *result;
	     memset((char *)&hints, 0, sizeof(hints));
	     hints.ai_family = AF_UNSPEC;
	     hints.ai_socktype = SOCK_DGRAM;
	     hints.ai_flags = AI_ADDRCONFIG; /* avoid service look-up */
	     int ret = getaddrinfo(sender_na.getNetAddress().c_str(), NULL, &hints, &result);
	     if (ret != 0)
	       {
		  errlog::log_error(LOG_LEVEL_ERROR,
				    "Can not resolve %s: %s", sender_na.getNetAddress().c_str(), 
				    gai_strerror(ret));
		  sender_na = NetAddress();
	       }
	     else
	       {
		  bool lying = true;
		  struct addrinfo *rp;
		  for (rp = result; rp != NULL; rp = rp->ai_next)
		    {
		       struct sockaddr_in *ai_addr = (struct sockaddr_in*)rp->ai_addr;
		       uint32_t v4_addr = ai_addr->sin_addr.s_addr;
		       std::string addr_from = NetAddress::unserialize_ip(v4_addr);
		       
		       if (addr_from == addr)
			 {
			    lying = false;
			    break;
			 }
		    }
		  if (lying)
		    {
		       /**
			* Seems like the sender is lying on its address.
			* We still serve it, but we do not use the potentially
			* fake/lying IP.
			*/
		       sender_na = NetAddress();
		    }
	       }
	  }
		
	// decides which response to give.
	int status = DHT_ERR_OK;
	dht_err err = execute_callback(fct_id,recipient_key,recipient_na,
				       sender_key,sender_na,node_key,status,resp_msg);
	
	//debug
	//std::cerr << "[Debug]: rsp_msg: " << resp_msg << std::endl;
	//debug
	
	return err;
     }
   
   void l1_protob_rpc_server::lx_server_response(const uint32_t &fct_id,
						 const DHTKey &recipient_key,
						 const NetAddress &recipient_na,
						 const DHTKey &sender_key,
						 const NetAddress &sender_na,
						 const DHTKey &node_key,
						 int &status,
						 std::string &resp_msg)
     {
	// XXX: we could communicate something nice back the sender,
	// but we won't.
	throw rpc_server_wrong_layer_exception();
     }
      
   dht_err l1_protob_rpc_server::execute_callback(const uint32_t &fct_id,
						  const DHTKey &recipient_key,
						  const NetAddress &recipient_na,
						  const DHTKey &sender_key,
						  const NetAddress &sender_na,
						  const DHTKey &node_key,
						  int& status,
						  std::string &resp_msg)
     {
	//debug
	std::cerr << "[Debug]:execute_callback: ";
	//debug
	
	l1::l1_response *l1r = NULL;
	
	if (fct_id == hash_get_successor)
	  {
	     //debug
	     std::cerr << "get_successor\n";
	     //debug
	     
	     DHTKey dkres;
	     NetAddress dkres_na;
	     RPC_getSuccessor_cb(recipient_key,recipient_na,
				 dkres,dkres_na,status);
	     
	     // create a response.
	     if (status == DHT_ERR_OK)
	       l1r = l1_protob_wrapper::create_l1_response(status,dkres,dkres_na);
	     else l1r = l1_protob_wrapper::create_l1_response(status);
	  }
	else if (fct_id == hash_get_predecessor)
	  {
	     //debug
	     std::cerr << "get_predecessor\n";
	     //debug
	     
	     DHTKey dkres;
	     NetAddress dkres_na;
	     RPC_getPredecessor_cb(recipient_key,recipient_na,
				   dkres,dkres_na,status);
	     
	     // create a response.
	     if (status == DHT_ERR_OK)
	       l1r = l1_protob_wrapper::create_l1_response(status,dkres,dkres_na);
	     else l1r = l1_protob_wrapper::create_l1_response(status);
	  }
	else if (fct_id == hash_notify)
	  {
	     //debug
	     std::cerr << "notify\n";
	     //debug
	     	     
	     RPC_notify_cb(recipient_key,recipient_na,
			   sender_key,sender_na,
			   status);
	     
	     // create a response.
	     l1r = l1_protob_wrapper::create_l1_response(status);
	  }
	else if (fct_id == hash_get_succlist)
	  {
	     //debug
	     std::cerr << "get_succlist\n";
	     //debug
	     
	     std::list<DHTKey> dkres_list;
	     std::list<NetAddress> na_list;
	     RPC_getSuccList_cb(recipient_key,recipient_na,
				dkres_list,na_list,status);
	     
	     // create a response.
	     if (status == DHT_ERR_OK)
	       l1r = l1_protob_wrapper::create_l1_response(status,dkres_list,na_list);
	     else l1r = l1_protob_wrapper::create_l1_response(status);
	  }
	else if (fct_id == hash_find_closest_predecessor)
	  {
	     //debug
	     std::cerr << "find_closest_predecessor\n";
	     //debug
	     
	     //debug
	     //TODO: catch this error.
	     assert(node_key.count()>0);
	     //debug
	     
	     DHTKey dkres, dkres_succ;
	     NetAddress dkres_na, dkres_succ_na;
	     RPC_findClosestPredecessor_cb(recipient_key,recipient_na,
					   node_key,
					   dkres,dkres_na,
					   dkres_succ,dkres_succ_na,
					   status);
	     
	     // create a response.
	     if (status == DHT_ERR_OK)
	       {
		  if (dkres_succ.count() > 0)
		    l1r = l1_protob_wrapper::create_l1_response(status,dkres,dkres_na,
								dkres_succ,dkres_succ_na);
		  else l1r = l1_protob_wrapper::create_l1_response(status,dkres,dkres_na);
	       }
	     else l1r = l1_protob_wrapper::create_l1_response(status);
	  }
	else if (fct_id == hash_join_get_succ)
	  {
	     //debug
	     std::cerr << "join_get_succ\n";
	     //debug
	     	     
	     DHTKey dkres;
	     NetAddress dkres_na;
	     RPC_joinGetSucc_cb(recipient_key,recipient_na,
				sender_key,
				dkres,dkres_na,status);
	     
	     // create a response.
	     if (status == DHT_ERR_OK)
	       l1r = l1_protob_wrapper::create_l1_response(status,dkres,dkres_na);
	     else l1r = l1_protob_wrapper::create_l1_response(status);
	  }
	else if (fct_id == hash_ping)
	  {
	     //debug
	     std::cerr << "ping\n";
	     //debug
	     
	     RPC_ping_cb(recipient_key,recipient_na,
			 status);
	     
	     // create a reponse.
	     l1r = l1_protob_wrapper::create_l1_response(status);
	  }
	else 
	  {
	     // TODO: unknown cb.
	     
	     //debug
	     std::cerr << "[Debug]:unknown callback.\n";
	     //debug
	     
	     errlog::log_error(LOG_LEVEL_DHT, "Couldn't find callback with id %u", fct_id);
	     return DHT_ERR_CALLBACK;
	  }
	
	// serialize the response.
	l1_protob_wrapper::serialize_to_string(l1r,resp_msg);
	delete l1r;
	
	//debug
	/* l1::l1_response l1rt;
	l1_protob_wrapper::deserialize(resp_msg,&l1rt);
	std::cerr << "layer_id resp deser: " << l1rt.head().layer_id() << std::endl; */
	//debug
	
	return DHT_ERR_OK;	
     }

   /*- l1 interface. -*/
   dht_err l1_protob_rpc_server::RPC_getSuccessor_cb(const DHTKey& recipientKey,
						     const NetAddress &recipient,
						     DHTKey& dkres, NetAddress& na,
						     int& status)
     {
	return _pnode->getSuccessor_cb(recipientKey,dkres,na,status);
     }
      
   dht_err l1_protob_rpc_server::RPC_getPredecessor_cb(const DHTKey& recipientKey,
						       const NetAddress &recipient,
						       DHTKey& dkres, NetAddress& na,
						       int& status)
     {
	return _pnode->getPredecessor_cb(recipientKey,dkres,na,status); 
     }
      
   dht_err l1_protob_rpc_server::RPC_notify_cb(const DHTKey& recipientKey,
					       const NetAddress &recipient,
					       const DHTKey& senderKey,
					       const NetAddress& senderAddress,
					       int& status)
     {
	if (senderAddress.empty() || senderAddress.getPort()==0)
	  return DHT_ERR_ADDRESS_MISMATCH;
	else return _pnode->notify_cb(recipientKey,senderKey,senderAddress,status);
     }

   dht_err l1_protob_rpc_server::RPC_getSuccList_cb(const DHTKey& recipientKey,
						    const NetAddress &recipient,
						    std::list<DHTKey> &dkres_list,
						    std::list<NetAddress> &na_list,
						    int& status)
     {
	return _pnode->getSuccList_cb(recipientKey,dkres_list,na_list,status);
     }
   
   dht_err l1_protob_rpc_server::RPC_findClosestPredecessor_cb(const DHTKey& recipientKey,
							       const NetAddress &recipient,
							       const DHTKey& nodeKey,
							       DHTKey& dkres, NetAddress& na,
							       DHTKey& dkres_succ, NetAddress &dkres_succ_na,
							       int& status)
     {
	return _pnode->findClosestPredecessor_cb(recipientKey,nodeKey,dkres,na,
						 dkres_succ,dkres_succ_na,status);	
     }

   dht_err l1_protob_rpc_server::RPC_joinGetSucc_cb(const DHTKey& recipientKey,
						    const NetAddress &recipient,
						    const DHTKey& senderKey,
						    DHTKey& dkres, NetAddress& na,
						    int& status)
     {
	return _pnode->joinGetSucc_cb(recipientKey,senderKey,dkres,na,status);
     }
      
   dht_err l1_protob_rpc_server::RPC_ping_cb(const DHTKey& recipientKey,
					     const NetAddress &recipient,
					     int& status)
     {
	return _pnode->ping_cb(recipientKey,status);
     }
      
} /* end of namespace. */
