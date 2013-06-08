//
//  Server.cpp
//  ofxLibwebsockets
//
//  Created by Brett Renfer on 4/11/12.
//  Copyright (c) 2012 Robotconscience. All rights reserved.
//

#include "ofxLibwebsockets/Server.h"
#include "ofxLibwebsockets/Util.h"

#include "ofEvents.h"
#include "ofUtils.h"

namespace ofxLibwebsockets {
    // SERVER CALLBACK
 
    static string getServerCallbackReason( int reason ){
        switch (reason){
			case 0 : return "LWS_CALLBACK_ESTABLISHED";
			case 1 : return "LWS_CALLBACK_CLIENT_CONNECTION_ERROR";
			case 2 : return "LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH";
			case 3 : return "LWS_CALLBACK_CLIENT_ESTABLISHED";
			case 4 : return "LWS_CALLBACK_CLOSED";
			case 5 : return "LWS_CALLBACK_RECEIVE";
			case 6 : return "LWS_CALLBACK_CLIENT_RECEIVE";
			case 7 : return "LWS_CALLBACK_CLIENT_RECEIVE_PONG";
			case 8 : return "LWS_CALLBACK_CLIENT_WRITEABLE";
			case 9 : return "LWS_CALLBACK_SERVER_WRITEABLE";

			case 10 : return "LWS_CALLBACK_HTTP";
			case 11 : return "LWS_CALLBACK_HTTP_FILE_COMPLETION";
			case 12 : return "LWS_CALLBACK_HTTP_WRITEABLE";
			case 13 : return "LWS_CALLBACK_FILTER_NETWORK_CONNECTION";
			case 14 : return "LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION";
			case 15 : return "LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS";
			case 16 : return "LWS_CALLBACK_OPENSSL_LOAD_EXTRA_SERVER_VERIFY_CERTS";
			case 17 : return "LWS_CALLBACK_OPENSSL_PERFORM_CLIENT_CERT_VERIFICATION";
			case 18 : return "LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER";
			case 19 : return "LWS_CALLBACK_CONFIRM_EXTENSION_OKAY";
	
			case 20 : return "LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED";
			case 21 : return "LWS_CALLBACK_PROTOCOL_INIT";
			case 22 : return "LWS_CALLBACK_PROTOCOL_DESTROY";
			// external poll() management support 
			case 23 : return "LWS_CALLBACK_ADD_POLL_FD";
			case 24 : return "LWS_CALLBACK_DEL_POLL_FD";
			case 25 : return "LWS_CALLBACK_SET_MODE_POLL_FD";
			case 26 : return "LWS_CALLBACK_CLEAR_MODE_POLL_FD";

			default: 
				std::stringstream r;
				r << "Unknown callback reason id: " << reason;	
				return r.str();               
        };
    }
    int lws_callback(struct libwebsocket_context* context, struct libwebsocket *ws, enum libwebsocket_callback_reasons reason, void *user, void *data, size_t len){
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


    //--------------------------------------------------------------
    Server::Server(){
        context = NULL;
        waitMillis = 50;
        reactors.push_back(this);
        
        defaultOptions = ServerOptions::defaultServerOptions();
    }

    //--------------------------------------------------------------
    bool Server::setup( int _port, bool bUseSSL )
    {
        // setup with default protocol (http) and allow ALL other protocols
        defaultOptions.port     = _port;
        defaultOptions.bUseSSL  = bUseSSL;
        
        if ( defaultOptions.port == 80 && defaultOptions.bUseSSL == true ){
            ofLog( OF_LOG_WARNING, "SSL IS NOT USUALLY RUN OVER DEFAULT PORT (80). THIS MAY NOT WORK!");
        }
        
        return setup( defaultOptions );
    }

    //--------------------------------------------------------------
    bool Server::setup( ServerOptions options ){
		/*
			enum lws_log_levels {
			LLL_ERR = 1 << 0,
			LLL_WARN = 1 << 1,
			LLL_NOTICE = 1 << 2,
			LLL_INFO = 1 << 3,
			LLL_DEBUG = 1 << 4,
			LLL_PARSER = 1 << 5,
			LLL_HEADER = 1 << 6,
			LLL_EXT = 1 << 7,
			LLL_CLIENT = 1 << 8,
			LLL_LATENCY = 1 << 9,
			LLL_COUNT = 10 
		};
		*/
		lws_set_log_level(LLL_ERR, NULL);

        defaultOptions = options;
        
        port = defaultOptions.port = options.port;
        document_root = defaultOptions.documentRoot = options.documentRoot;
        
        // NULL protocol is required by LWS
        struct libwebsocket_protocols null_protocol = { NULL, NULL, 0 };
        
        // NULL name = any protocol
        struct libwebsocket_protocols null_name_protocol = { NULL, lws_callback, sizeof(Connection) };
        
        //setup protocols
        lws_protocols.clear();
        
        //register main protocol
        serverProtocol.binary = options.bBinaryProtocol;
        registerProtocol( options.protocol, serverProtocol );
        
        //register any added protocols
        for (int i=0; i<protocols.size(); ++i){
            struct libwebsocket_protocols lws_protocol = {
                ( protocols[i].first == "NULL" ? NULL : protocols[i].first.c_str() ),
                lws_callback,
                sizeof(Connection)
            };
            lws_protocols.push_back(lws_protocol);
        }
        lws_protocols.push_back(null_protocol);
        
        // make cert paths  null if not using ssl
        const char * sslCert = NULL;
        const char * sslKey = NULL;
        
        if ( defaultOptions.bUseSSL ){
            sslCert = defaultOptions.sslCertPath.c_str();
            sslKey = defaultOptions.sslKeyPath.c_str();
        }
        
        int opts = 0;
        struct lws_context_creation_info info;
        memset(&info, 0, sizeof info);
        info.port = port;
        info.protocols = &lws_protocols[0];
        info.extensions = libwebsocket_get_internal_extensions();
        info.ssl_cert_filepath = sslCert;
        info.ssl_private_key_filepath = sslKey;
        info.gid = -1;
        info.uid = -1;
        info.options = opts;

        context = libwebsocket_create_context(&info);

        //context = libwebsocket_create_context( port, NULL, &lws_protocols[0], libwebsocket_internal_extensions, sslCert, sslKey, /*"",*/ -1, -1, opts, NULL);
        
        if (context == NULL){
            std::cerr << "libwebsocket init failed" << std::endl;
            return false;
        } else {
            startThread(true, false); // blocking, non-verbose        
            return true;
        }
    }
    
    //--------------------------------------------------------------
	void Server::close() {
		waitForThread(true);
		libwebsocket_context_destroy(context);
	}

    //--------------------------------------------------------------
    void Server::broadcast( string message ){
        // loop through all protocols and broadcast!
        for (int i=0; i<protocols.size(); i++){
            protocols[i].second->broadcast( message );
        }
    }
    
    //--------------------------------------------------------------
    void Server::send( string message ){
        for (int i=0; i<connections.size(); i++){
            if ( connections[i] ){
                connections[i]->send( message );
            }
        }
    }
    
    //--------------------------------------------------------------
    void Server::sendBinary( unsigned char * data, int size ){
        for (int i=0; i<connections.size(); i++){
            if ( connections[i] ){
                connections[i]->sendBinary( data, size );
            }
        }
    }
    
    //--------------------------------------------------------------
    void Server::send( string message, string ip ){
        bool bFound = false;
        for (int i=0; i<connections.size(); i++){
            if ( connections[i] ){
                if ( connections[i]->getClientIP() == ip ){
                    connections[i]->send( message );
                    bFound = true;
                }
            }
        }
        if ( !bFound ) ofLog( OF_LOG_ERROR, "Connection not found!" );
    }
    
    //getters
    //--------------------------------------------------------------
    int Server::getPort(){
        return defaultOptions.port;
    }
    
    //--------------------------------------------------------------
    string Server::getProtocol(){
        return ( defaultOptions.protocol == "NULL" ? "none" : defaultOptions.protocol );
    }
    
    //--------------------------------------------------------------
    bool Server::usingSSL(){
        return defaultOptions.bUseSSL;
    }

    //--------------------------------------------------------------
    void Server::threadedFunction()
    {
        while (isThreadRunning())
        {
            for (int i=0; i<protocols.size(); ++i){
                if (protocols[i].second != NULL){
                    //lock();
                    protocols[i].second->execute();
                    //unlock();
                }
            }
            lock();
            libwebsocket_service(context, waitMillis);
            unlock();
        }
    }
}
