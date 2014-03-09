//
//  Util.h
//  ofxLibwebsockets
//
//  Created by Brett Renfer on 4/11/12.
//  Copyright (c) 2012 Robotconscience. All rights reserved.
//

#pragma once

#include <libwebsockets.h>

#include "ofxLibwebsockets/Connection.h"
#include "ofxLibwebsockets/Reactor.h"
#include "ofxLibwebsockets/Client.h"
#include "ofxLibwebsockets/Server.h"

namespace ofxLibwebsockets {
        
    class Client;
    class Server;
        

    
    // SERVER CALLBACK
  /*
    static int lws_callback(struct libwebsocket_context* context, struct libwebsocket *ws, enum libwebsocket_callback_reasons reason, void *user, void *data, size_t len){
        const struct libwebsocket_protocols* lws_protocol = (ws == NULL ? NULL : libwebsockets_get_protocol(ws));
        int idx = lws_protocol? lws_protocol->protocol_index : 0;   
        
        // valid connection w/o a protocol
        if ( ws != NULL && lws_protocol == NULL ){
            // OK for now, returning 0 above
        }
        
        //bool bAllowAllProtocls = (ws != NULL ? lws_protocol == NULL : false);
        
        Connection* conn;    
        Connection** conn_ptr = (Connection**)user;
        Server* reactor = NULL;
        Protocol* protocol = NULL;
        
        for (int i=0; i<(int)reactors.size(); i++){
            if (reactors[i]->getContext() == context){
                reactor = (Server*) reactors[i];
                protocol = reactor->protocol( (idx > 0 ? idx : 0) );
                break;
            } else {
            }
        }
        
        ofLog( OF_LOG_VERBOSE, getServerCallbackReason(reason) );
        
        if (reason == LWS_CALLBACK_ESTABLISHED){
            if ( reactor != NULL ){
                *conn_ptr = new Connection(reactor, protocol);
            }
        } else if (reason == LWS_CALLBACK_CLOSED){
            //if (*conn_ptr != NULL)
            //delete *conn_ptr;
        }

        switch (reason)
        {
            case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
                return 0;
                
            case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:
                if (protocol != NULL ){
                    return reactor->_allow(ws, protocol, (int)(long)user)? 0 : 1;
                } else {
                    return 1;
                }
                
            case LWS_CALLBACK_HTTP:
                return reactor->_http(ws, (char*)data);
                
            case LWS_CALLBACK_ESTABLISHED:
            case LWS_CALLBACK_CLOSED:
            case LWS_CALLBACK_SERVER_WRITEABLE:
            case LWS_CALLBACK_RECEIVE:
			case LWS_CALLBACK_CLIENT_RECEIVE:
			case LWS_CALLBACK_CLIENT_RECEIVE_PONG:
            //case LWS_CALLBACK_BROADCAST:
            //In the current git version, All of the broadcast proxy stuff is removed: data must now be sent from the callback only
            // http://git.libwebsockets.org/cgi-bin/cgit/libwebsockets/commit/test-server/test-server.c?id=6f520a5195defcb7fc69c669218a8131a5f35efb
                conn = *(Connection**)user;
                
                if (conn && conn->ws != ws){
                    conn->context = context;
                    conn->ws = ws;
                    conn->setupAddress();
                }
                if (reactor){
                    return reactor->_notify(conn, reason, (char*)data, len);                
                } else {
                    return 0;
                }
                
            default:
                return 0;
        }
        
        return 1; // FAIL (e.g. unhandled case/break in switch)
    }
*/
    // not used
    /*
>>>>>>> a2daa2b812e7675b3bba2eb8152ad5e48d0ebf49
    static void dump_handshake_info(struct lws_tokens *lwst)
    {
        int n;
        static const char *token_names[WSI_TOKEN_COUNT] = {
            "GET URI",
            "Host",
            "Connection",
            "key 1",
            "key 2",
            "Protocol",
            "Upgrade",
            "Origin",
            "Draft",
            "Challenge",
            
            // new for 04 
            "Key",
            "Version",
            "Sworigin",
            
            // new for 05 
            "Extensions",
            
            // client receives these 
            "Accept",
            "Nonce",
            "Http",
            "MuxURL",
        };
        
        for (n = 0; n < WSI_TOKEN_COUNT; n++) {
            if (lwst[n].token == NULL)
                continue;
            
            fprintf(stderr, "    %s = %s\n", token_names[n], lwst[n].token);
        }
    }
    */
    
  
}
