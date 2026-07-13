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

const { Router } = require('express');

const log = require("./log");
const {isArray,isObject,CleanRegEx, GenerateLimiter, SanitizeQuery, CoerceUpdateValue} = require('./utils');
const {getDb} = require('./db');

const {CheckAuth,CheckServerAuth} = require('./AuthChecker');



const router = Router();

// apply rate limiter to all requests
router.use(GenerateLimiter(global.config.RequestLimitQuery || 400, 10));


/**
 * Post: /[Collection]/[Mod]
 *
 */
router.post('/:mod', (req, res)=>{
    runQuery(req, res, req.params.mod, req.headers['auth-key'], GetCollection(req.baseUrl));
});

router.post('/Update/:mod', (req, res)=>{
    runUpdateFromQuery(req, res, req.params.mod, req.headers['auth-key'], GetCollection(req.baseUrl));
});

function GetCollection(URL){
    if (URL.includes("/Player/")){
        return "Players"
    }
    if (URL.includes("/Object/")){
        return "Objects"
    }
}

async function runQuery(req, res, mod, auth, COLL) {
    if (CheckServerAuth(auth) || ((await CheckAuth(auth)) && COLL === "Objects") ){
        var RawData = req.body;
        try{
            const db = await getDb();
            let collection = db.collection(COLL);
            // SECURITY: strip $where / JS-exec operators from the client query.
            let query = SanitizeQuery(JSON.parse(RawData.Query));
            let orderBy = SanitizeQuery(JSON.parse(RawData.OrderBy));
            let fixQuery = RawData.FixQuery || 0;
            let ReturnCol = "Data";
            if (COLL == "Players"){
                ReturnCol = mod;
            }
            if (fixQuery === 1){
                query = FixQuery(query,ReturnCol);
                orderBy = FixQuery(orderBy,ReturnCol);
            }
            if (COLL == "Players"){
                if (query && Object.keys(query).length === 0 && query.constructor === Object){
                    query[mod] = { "$exists": true };
                }
            }
            if (COLL == "Objects" && (query.Mod === undefined || query.Mod === null)){
                query.Mod = mod;
            }
            let results = collection.find(query).sort(orderBy);
            if (RawData.MaxResults >= 1){
                results.limit(RawData.MaxResults);
            }
            let theData = await results.toArray();
            let RetrunData = [];
            let count = 0;
            for (result of theData){
                for (const [key, value] of Object.entries(result)) {
                    if(key === ReturnCol){
                        if (RawData.ReturnObject != "" && RawData.ReturnObject != null){
                            count ++;
                            RetrunData.push(value[RawData.ReturnObject]);
                        } else {
                            count ++;
                            RetrunData.push(value);
                        }
                    }
                }
            }
            if (RetrunData){
                if (count == 0){
                    let simpleReturn = { Status: "NoResults", Count: 0, Results: [] }
                    log("Query:  " + JSON.stringify(query) + " against " + COLL + " for " + ReturnCol + " Got 0 Results", "info");
                    res.json(simpleReturn);
                } else {
                    let simpleReturn = {Status: "Success", Count: count, Results: RetrunData }
                    log("Query:  " + JSON.stringify(query) + " against " + COLL + " for " + ReturnCol + " Got " + count + " Results", "info");
                    res.json(simpleReturn);
                }
            }
        }catch(err){
            log("Error in Query against " + COLL + " for mod " + mod + " error: " + err, "warn");
            res.status(203);
            res.json({Status: "Error", Count: 0, Results: [] });
        }
    } else {
        res.status(401);
        res.json({Status: "Error", Error: "Invalid Auth", Count: 0, Results: [] });
    }
};

async function runUpdateFromQuery(req, res, mod, auth, COLL) {
    if ( CheckServerAuth(auth) || ((await CheckAuth(auth)) && global.config.AllowClientWrite) ){
        let RawData = req.body;
        try{
            // SECURITY: strip $where / JS-exec operators from the client query.
            let query = SanitizeQuery(JSON.parse(RawData.Query.Query));
            let orderBy = SanitizeQuery(JSON.parse(RawData.Query.OrderBy));
            let fixQuery = RawData.Query.FixQuery || 0;
            let ReturnCol = "Data";
            if (COLL == "Players"){
                ReturnCol = mod;
            }
            if (fixQuery === 1){
                query = FixQuery(query,ReturnCol);
                orderBy = FixQuery(orderBy,ReturnCol);
            }
            if (COLL == "Players"){
                if (query && Object.keys(query).length === 0 && query.constructor === Object){
                    query[mod] = { "$exists": true };
                }
            }
            if (COLL == "Objects" && (query.Mod === undefined || query.Mod === null)){
                query.Mod = mod;
            }
            let element = RawData.Element;
            let operation = RawData.Operation || "set";
            // Injection-safe value + computed field-path key (see player.js).
            let value = CoerceUpdateValue(RawData.Value);
            const db = await getDb();
            let collection = db.collection(COLL);
            const options = { upsert: false };

            let updateDocValue = {};
            updateDocValue[`Data.${element}`] = value;
            let updateDoc = { $set: updateDocValue, };

            if (operation === "pull"){
                updateDoc = { $pull: updateDocValue, };
            } else if (operation === "push"){
                updateDoc = { $push: updateDocValue, };
            } else if (operation === "unset"){
                updateDoc = { $unset: updateDocValue, };
            } else if (operation === "mul"){
                updateDoc = { $mul: updateDocValue, };
            } else if (operation === "rename"){
                updateDoc = { $rename: updateDocValue, };
            } else if (operation === "pullAll"){
                updateDoc = { $pullAll: updateDocValue, };
            }

            const result = await collection.updateOne(query, updateDoc, options);
            if (result.matchedCount >= 1 || result.upsertedCount >= 1){
                log("Updated " +  result.matchedCount + " items " + element +" for "+ mod + " Data for Query: " + JSON.stringify(query));
                res.status(200);
                res.json({ Status: "Success", Element: element, Mod: mod, Count:  result.matchedCount});
            } else {
                log("Updated " +  result.matchedCount + " items " + element +" for "+ mod + " Data for Query: " + JSON.stringify(query));
                res.status(203);
                res.json({ Status: "NoResults", Element: element, Mod: mod, Count: 0});
            }
        }catch(err){
            log(`ERROR: ${err}`, "warn");
            res.status(203);
            res.json({ Status: "Error", Element: RawData.Element, Mod: mod, Count: 0});
        }
    } else {
        res.status(401);
        // NOTE: original referenced an undefined `ObjectId` here (latent crash);
        // aligned to this route's documented response fields instead.
        res.json({ Status: "Error", Error: "Invalid Auth", Element: "", Mod: mod, Count: 0});
        log("AUTH ERROR: " + req.url, "warn");
    }
};
function FixQuery(query, prefix){
    if (isObject(query)){
        for (const [key, value] of Object.entries(query)) {
            if(!key.match(/^\$/i) && !key.match(new RegExp(`^${CleanRegEx(prefix)}\\.`, "g"))){
                query[`${prefix}.${key}`] = FixQuery(value, prefix);
                delete query[key]
            } else {
                query[key] = FixQuery(value, prefix);
            }
        }
        return query;
    } else if (isArray(query)){
        let newArr = [];
        query.forEach(e => {
            newArr.push(FixQuery(e, prefix));
        });
        return newArr;
    }
    return query;
}

module.exports = router;
