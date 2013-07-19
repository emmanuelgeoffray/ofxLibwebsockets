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
        

    
    // not used
    /*
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
