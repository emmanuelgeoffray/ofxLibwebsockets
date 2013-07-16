//
//  Client.cpp
//  ofxLibwebsockets
//
//  Created by Brett Renfer on 4/11/12.
//  Copyright (c) 2012 Robotconscience. All rights reserved.
//

#include "ofxLibwebsockets/Client.h"
#include "ofxLibwebsockets/Util.h"

namespace ofxLibwebsockets {
// CLIENT CALLBACK
    static string getClientCallbackReason( int reason ){
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
        }
    }
 
    extern int lws_client_callback(struct libwebsocket_context* context, struct libwebsocket *ws, enum libwebsocket_callback_reasons reason, void *user, void *data, size_t len){
        const struct libwebsocket_protocols* lws_protocol = (ws == NULL ? NULL : libwebsockets_get_protocol(ws));
        int idx = lws_protocol? lws_protocol->protocol_index : 0;          
        
        Connection* conn;
        
        Reactor* reactor = NULL;
        Protocol* protocol;
            
        for (int i=0; i<(int)reactors.size(); i++){
            if (reactors[i]->getContext() == context){
                reactor =  reactors[i];
                protocol = reactor->protocol(idx);
                conn = ((Client*) reactor)->getConnection();
                break;
            } else {
            }
        }
      
//      ofxLogVerbose() << getClientCallbackReason(reason) << endl;
        ofLog( OF_LOG_VERBOSE, getClientCallbackReason(reason) );
        if (reason == LWS_CALLBACK_CLIENT_ESTABLISHED ){
        } else if (reason == LWS_CALLBACK_CLOSED){
        }
        
        switch (reason)
        {            
            case LWS_CALLBACK_CONFIRM_EXTENSION_OKAY:
                return 0;
                
            case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:
                if (protocol != NULL){
                    return reactor->_allow(ws, protocol, (int)(long)user)? 0 : 1;
                } else {
                    return 0;
                }
            case LWS_CALLBACK_HTTP:
                if ( reactor != NULL){
                    return reactor->_http(ws, (char*)data);
                } else {
                    return 0;
                }
                
            case LWS_CALLBACK_CLIENT_WRITEABLE:
            case LWS_CALLBACK_CLIENT_ESTABLISHED:
            case LWS_CALLBACK_CLOSED:
            case LWS_CALLBACK_CLIENT_RECEIVE:
            case LWS_CALLBACK_CLIENT_RECEIVE_PONG:
                if ( reactor != NULL ){
                    //conn = *(Connection**)user;
                    if (conn && conn->ws != ws) conn->ws = ws;
                    return reactor->_notify(conn, reason, (char*)data, len);
                } else {
                    return 0;
                }
            default:
                return 0;
        }
        
        return 1; // FAIL (e.g. unhandled case/break in switch)
    }

    Client::Client(){
      time_t rawtime;
      struct tm * timeinfo;
      char buffer [40];
      time ( &rawtime );
      timeinfo = localtime ( &rawtime );
      strftime (buffer,40,"%Y-%m-%d",timeinfo);
      string logPath=buffer;
      ofxLogSetLogLevel(LOG_VERBOSE);
      if (!ofDirectory("logs").exists()){
        ofDirectory("logs").create(true);
      }
      ofxLogSetLogToFile(true, ofToDataPath("logs/"+logPath+".log"));
      ofxLogSetLogLineNumber(true);
      ofxLogSetLogCaller(true);
      ofxLogSetLogOptions(LOG_USE_TIME | LOG_USE_CALL | LOG_USE_TYPE | LOG_USE_PADD | LOG_USE_FILE);
        context = NULL;
        connection = NULL;
        waitMillis = 50;
        //count_pollfds = 0;
        reactors.push_back(this);
        
        defaultOptions = ClientOptions::defaultClientOptions();
        
        ofAddListener( clientProtocol.oncloseEvent, this, &Client::onClose);
    }

    //--------------------------------------------------------------
    bool Client::connect ( string _address, bool bUseSSL ){
        defaultOptions.host     = _address;
        defaultOptions.bUseSSL  = bUseSSL;
        defaultOptions.port     = (bUseSSL ? 443 : 80);
        
        return connect( defaultOptions );
    }

    //--------------------------------------------------------------
    bool Client::connect ( string _address, int _port, bool bUseSSL ){
        defaultOptions.host     = _address;
        defaultOptions.port     = _port;
        defaultOptions.bUseSSL  = bUseSSL;
        
        return connect( defaultOptions );
    }

    //--------------------------------------------------------------
    bool Client::connect ( ClientOptions options ){
      ofxLogVerbose() << "connect: "+options.host+":"+ofToString(options.port)+options.channel+":"+ofToString(options.bUseSSL) << endl;
//        ofLog( OF_LOG_VERBOSE, "connect: "+options.host+":"+ofToString(options.port)+options.channel+":"+ofToString(options.bUseSSL) );
        address = options.host;
        port    = options.port;  
        channel = options.channel;
        
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

        // set up default protocols
        struct libwebsocket_protocols null_protocol = { NULL, NULL, 0 };
        
        // setup the default protocol (the one that works when you do addListener())
        registerProtocol( options.protocol, clientProtocol );  
        
        lws_protocols.clear();
        for (int i=0; i<protocols.size(); ++i)
        {
            struct libwebsocket_protocols lws_protocol = {
                ( protocols[i].first == "NULL" ? NULL : protocols[i].first.c_str() ),
                lws_client_callback,
                sizeof(Connection)
            };
            lws_protocols.push_back(lws_protocol);
        }
        lws_protocols.push_back(null_protocol);

        struct lws_context_creation_info info;
        memset(&info, 0, sizeof info);
        info.port = CONTEXT_PORT_NO_LISTEN;
        info.protocols = &lws_protocols[0];
        info.extensions = libwebsocket_get_internal_extensions();
        info.gid = -1;
        info.uid = -1;

        context = libwebsocket_create_context(&info);

        
        //context = libwebsocket_create_context(CONTEXT_PORT_NO_LISTEN, NULL,
        //                                      &lws_protocols[0], libwebsocket_internal_extensions,
        //                                      NULL, NULL, /*NULL,*/ -1, -1, 0, NULL);
        if (context == NULL){
          ofxLogError() << "libwebsocket init failed" << endl;
//            std::cerr << "libwebsocket init failed" << std::endl;
            return false;
        } else {
          ofxLogVerbose() << "libwebsocket init success" << endl;
//            std::cerr << "libwebsocket init success" << std::endl;  
          
            string host = options.host +":"+ ofToString( options.port );
            
            // register with or without a protocol
            if ( options.protocol == "NULL"){
                lwsconnection = libwebsocket_client_connect( context, 
                                                            options.host.c_str(), options.port, (options.bUseSSL ? 2 : 0 ), 
                                                            options.channel.c_str(), host.c_str(), host.c_str(), NULL, options.version);
            } else {
                lwsconnection = libwebsocket_client_connect( context, 
                                                            options.host.c_str(), options.port, (options.bUseSSL ? 2 : 0 ), 
                                                            options.channel.c_str(), host.c_str(), host.c_str(), options.protocol.c_str(), options.version);
            }
                        
            if ( lwsconnection == NULL ){
                std::cerr << "client connection failed" << std::endl;
                return false;
            } else {
                
                connection = new Connection( (Reactor*) &context, &clientProtocol );
                connection->ws = lwsconnection;
                                                
                std::cerr << "client connection success" << std::endl;
                startThread(true, false); // blocking, non-verbose   
                return true;
            }
        }
    }

    //--------------------------------------------------------------
    void Client::close(){
        if (isThreadRunning()){
            waitForThread(true);
        } else {
			return;
		}
        if ( context != NULL ){
            //libwebsocket_close_and_free_session( context, lwsconnection, LWS_CLOSE_STATUS_NORMAL);
            closeAndFree = true;
            libwebsocket_context_destroy( context );
            context = NULL;        
            lwsconnection = NULL;
        }
		if ( connection != NULL){
                //delete connection;
			connection = NULL;                
		}
    }


    //--------------------------------------------------------------
    void Client::onClose( Event& args ){
		// on windows an exit of the server let's the client crash
		// the event is called from the processing of the thread
		// thus all we can do wait for the thread to stop itself
		// by detecting that lwsconnection is NULL
		if ( context != NULL ){
			closeAndFree = true;
			lwsconnection = NULL;
		}     
    }

    //--------------------------------------------------------------
    void Client::send( string message ){
        if ( connection != NULL){
            connection->send( message );
        }
    }

    //--------------------------------------------------------------
    void Client::threadedFunction(){
        while ( isThreadRunning() ){
            for (int i=0; i<protocols.size(); ++i){
                if (protocols[i].second != NULL){
                    //lock();
                    protocols[i].second->execute();
                    //unlock();
                }
            }
            if (context != NULL && lwsconnection != NULL){
                //libwebsocket_callback_on_writable(context, lwsconnection);
                int n = libwebsocket_service(context, waitMillis);
            } else {
				stopThread();
				if ( context != NULL ){
					closeAndFree = true;
					libwebsocket_context_destroy( context );
					context = NULL;        
					lwsconnection = NULL;
				}
				if (connection != NULL){
					connection = NULL;                
				}
            }
        }
    }

}
