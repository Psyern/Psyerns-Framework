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
const {CheckAuth,CheckServerAuth} = require('./AuthChecker')
const {isArray,GenerateLimiter, GetClientIP} = require('./utils');
let {createHash} = require('crypto');
const {getDb} = require('./db');


const log = require("./log");


const router = Router();


router.use(GenerateLimiter(global.config.RequestLimitLogger || 500, 10));

router.post('/One/:id', (req, res)=>{
    runLoggerOne(req, res,req.params.id, req.headers['auth-key']);
});

router.post('/Many/:id', (req, res)=>{
    runLoggerMany(req, res,req.params.id, req.headers['auth-key']);
});


async function runLoggerOne(req, res, id, auth) {
    let RawData = req.body;
	let hasServerAuth = CheckServerAuth(auth);
	let hasClientAuth = await CheckAuth(auth, true);
    if ( hasClientAuth || hasServerAuth ){
        try{
            const db = await getDb();
            let collection = db.collection("Logs");
            let datetime = new Date();
            let ClientId = GetClientID(req);
            RawData.ServerId = id;
            RawData.LoggedDateTime = datetime;
            RawData.ClientId = ClientId;
			if (hasServerAuth){
				RawData.ClientType = "Server";
			} else if (hasClientAuth){
				RawData.ClientType = "Client";
            }

            const result = await collection.insertOne(RawData);
            if (result.insertedId != undefined ){
                res.json({Status: "Success", Error: ""});
                log(`New Log Registered from ${RawData.ClientType} - Device ID: ${RawData.ClientId}`);
            } else {
                log("ERROR: Database Write Error", "warn");
                res.status(500);
                res.json({Status: "Error", Error: "Database Write Error"});
            }
        }catch(err){
            log("ERROR: " + err, "warn");
            res.status(500);
            res.json({Status: "error", Error: err});
        }
    }
}

async function runLoggerMany(req, res, id, auth) {
    let RawData = req.body;
	let hasServerAuth = CheckServerAuth(auth);
	let hasClientAuth = await CheckAuth(auth, true);
    if (hasClientAuth || hasServerAuth){
        try{
            const db = await getDb();
            let collection = db.collection("Logs");
            let datetime = new Date();
            let ClientId = GetClientID(req);
            let ClientType = "Server";
            RawData.forEach(element => {
                element.ServerId = id;
                element.LoggedDateTime = datetime;
				if (hasServerAuth){
					element.ClientType = "Server";
                    ClientType = "Server";
				} else if (hasClientAuth){
					element.ClientType = "Client";
                    ClientType = "Client";
				}
                element.ClientId = ClientId;
            });
            const result = await collection.insertMany(RawData);
            if (result.insertedCount > 0 ){
                res.json({Status: "Success", Error: "" });
                log(`New Log Array Registered from ${ClientType} - Device ID: ${ClientId}`);
            } else {
                res.status(500);
                res.json({Status: "Error", Error: "Database Write Error"});
                log("ERROR: Database Write Error", "warn");
            }
        }catch(err){
            log("ERROR: " + err, "warn");
            res.status(500);
            res.json({Status: "Error", Error: err});
        }
    }
}

function GetClientID(req){
    let ip = GetClientIP(req);
    let  hash = createHash('sha256');
    let theHash = hash.update(`${ip}`).digest('base64');
    return theHash.substr(0,32); //Cutting the last few digets to save a bit of data and make sure people don't mistake it for the GUIDS
}


module.exports = router;
