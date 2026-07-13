/**
 * Psyerns_Framework — DME_Api
 *
 * Original work Copyright (c) daemonforge — DayZ-UniversalApi (AGPL-3.0)
 *   https://github.com/daemonforge/DayZ-UniveralApi
 * Modifications & integration Copyright (c) 2026 Psyern / Deadmans Echo
 * Modified by Deadmans Echo, 2026.   [AGPL §5(a) change notice]
 *
 * This file is part of Psyerns_Framework and is licensed under the
 * GNU Affero General Public License v3.0. See LICENSE in the repo root.
 * SPDX-License-Identifier: AGPL-3.0-only
 */

const {Router} = require('express');
// node-fetch removed — Node >=18 has a global fetch().

const log = require("./log");

const {CheckAuth,CheckServerAuth} = require('./AuthChecker')
const {isArray, GenerateLimiter, IsForwardUrlAllowed} = require('./utils');

/*
    let Request.URL
    let Request.Headers[].Key
    let Request.Headers[].Value
    let Request.Body
    let Request.Method
    let Request.ReturnValue = "" //Will look for this specific Key to return as the data
*/


const router = Router();

// apply rate limiter to all requests (express-rate-limit v7 via shared factory)
router.use(GenerateLimiter(global.config.RequestLimitForwarder || 300, 10));

router.post('', (req, res)=>{
    runFowarder(req, res, req.headers['auth-key'])
});
async function runFowarder(req, res, auth){
    let RawData = req.body;
    if ( CheckServerAuth(auth) || (await CheckAuth( auth )) ){
        // SECURITY (SSRF): only fetch URLs permitted by IsForwardUrlAllowed
        // (ForwardAllowList if set, otherwise block internal/loopback targets).
        if (!IsForwardUrlAllowed(RawData.URL)){
            log("Forward blocked (SSRF guard) for URL: " + RawData.URL, "warn");
            res.status(403);
            res.json({Status: "Error", Error: "URL not allowed"});
            return;
        }
        log("Fowarded Called URL: " + RawData.URL );
        let strHeaders = "{";
        let json;
        for (let header of RawData.Headers) {
            strHeaders = strHeaders + " \"" + header.Key + "\": \"" + header.Value + "\",";
        }
        let strHeadersLen = strHeaders.length - 1; //Remove the last extra ','
        strHeaders = strHeaders.substring(0,strHeadersLen);
        strHeaders = strHeaders + " }";
        let Headers = JSON.parse(strHeaders);
        //console.log(RawData.Body)
        try {
        json = await fetch(RawData.URL, {
            method: RawData.Method,
            body: RawData.Body,
            headers: Headers
        }).then(response => response.json());
        }catch(e) {
            console.log('Catch an error: ', e)
        }
        //console.log(json);
        let ReturnValue;
        if (RawData.ReturnValue != ""){
            try{
                ReturnValue = json[RawData.ReturnValue];
                if (RawData.ReturnValueArrayIndex >= 0){
                    ReturnValue = json[RawData.ReturnValue][RawData.ReturnValueArrayIndex];
                }
            } catch(err) {
                log("Error Trying to get Return Value " + RawData.ReturnValue + " from response " + err, "warn");
                ReturnValue = json;
            }
        } else {
            ReturnValue = json;
            try{
                if (RawData.ReturnValueArrayIndex >= 0){
                    ReturnValue = json[RawData.ReturnValueArrayIndex];
                }
            } catch(err) {
                log("Error Trying to get Return index: " + err, "warn");
                ReturnValue = json;
            }
        }
        res.json(ReturnValue);
    }else{
        res.status(401);
        res.json({});
        log("AUTH ERROR: " + req.url + " Invalid Token", "warn");
    }
}


module.exports = router;
