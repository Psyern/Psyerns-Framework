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

const {CheckAuth, CheckServerAuth} = require('./AuthChecker')
const {isArray, isObject, isEmpty, CoerceUpdateValue} = require('./utils')
const {getDb} = require('./db');

const log = require("./log");


const router = Router();

router.post('/Load/:mod', (req, res)=>{
    runGet(req, res, req.params.mod, req.headers['auth-key']);
});

router.post('/Save/:mod', (req, res)=>{
    runSave(req, res, req.params.mod, req.headers['auth-key']);
});

router.post('/Transaction/:mod', (req, res)=>{
    runTransaction(req, res, req.params.mod, req.headers['auth-key']);
});

router.post('/Update/:mod', (req, res)=>{
    runUpdate(req, res, req.params.mod, req.headers['auth-key']);
});


async function runGet(req, res, mod, auth) {
    let RawData = req.body;
    if (CheckServerAuth(auth)|| (await CheckAuth(auth)) ){
        try{
            const db = await getDb();
            let collection = db.collection("Globals");
            let query = { Mod: mod };
            if ((await collection.countDocuments(query)) == 0){
                if ((CheckServerAuth(auth) || global.config.AllowClientWrite) && !isEmpty(RawData)){
                    let doc = { Mod: mod, Data: RawData };
                    let result = await collection.insertOne(doc);
                    // Behaviour preserved from 1.3.2: insertOne returns no
                    // matchedCount/upsertedCount, so this stays false (responds 200
                    // + body echo, not 201). Left as-is to keep the wire behaviour.
                    if ( result.matchedCount === 1 || result.upsertedCount === 1 ){
                        log("Created "+ mod + " Globals");
                        res.status(201);
                    }
                }
                res.json(RawData);
            } else {
                let results = collection.find(query);
                let data = await results.toArray();
                log("Retrieving "+ mod + " Globals");
                res.json(data[0].Data);
            }
        }catch(err){
            res.status(203);
            res.json(RawData);
            log("ERROR: " + err, "warn");
        }
    } else {
        res.status(401);
        res.json(RawData);
        log("AUTH ERROR: " + req.url, "warn");
    }
};
async function runSave(req, res, mod, auth) {
    let RawData = req.body;
    if (CheckServerAuth(auth) || ((await CheckAuth(auth)) && global.config.AllowClientWrite) ){
        try{
            const db = await getDb();
            let collection = db.collection("Globals");
            let query = { Mod: mod };
            const options = { upsert: true };
            const updateDoc  = {
                $set: { Mod: mod, Data: RawData }
            };
            const result = await collection.updateOne(query, updateDoc, options);
            if ( result.matchedCount === 1 || result.upsertedCount === 1 ){
                log("Updated "+ mod + " Globals");
                res.status(201);
                res.json(RawData);
            } else {
                log("Error with Updating "+ mod + "Globals", "warn");
                res.status(203);
                res.json(RawData);
            }
        }catch(err){
            res.status(203);
            res.json(RawData);
            log("ERROR: " + err, "warn");
        }
    } else {
        res.status(401);
        res.json(req.body);
        log("AUTH ERROR: " + req.url, "warn");
    }
};

async function runTransaction(req, res, mod, auth){

    if (CheckServerAuth(auth) || ((await CheckAuth(auth)) && global.config.AllowClientWrite) ){
        let RawData = req.body;
        try{
            const db = await getDb();
            let collection = db.collection("Globals");
            let  query = { Mod: mod };
            let Element =  "Data." + RawData.Element;
            // Injection-safe $inc: computed key + numeric value (was string-concatenated).
            let inc = {};
            inc[Element] = RawData.Value * 1;
            let options = { upsert: false };
            let Results = await collection.updateOne(query, { $inc: inc }, options);
            if (Results.modifiedCount >= 1 || Results.upsertedCount >= 1){
                let Value = await collection.distinct(Element, query);
                log("Transaction " + mod + " incermented " + Element + " by " + RawData.Value + " now " + Value[0], "warn");
                res.json({Status: "Success", ID: mod,  Value: Value[0], Element: RawData.Element})
            } else {
                log("Error in Transaction:  " + mod + " for Globals error: Invaild mod", "warn");
                res.json({Status: "Error", ID: mod,  Value: 0, Element: RawData.Element})
            }
        }catch(err){
            log("Error in Transaction: :  " + mod + " for Globals error: " + err, "warn");
            res.status(203);
            res.json({Status: "Error", ID: mod, Value: 0, Element: RawData.Element });
        }
    } else {
        res.status(401);
        res.json({Status: "Error", Error: "Invalid Auth", ID: mod, Value: 0, Element: RawData.Element });
    }

}
async function runUpdate(req, res, mod, auth) {
    // Contract: /Globals/Update is Server-auth ONLY (wire audit §6.3). Upstream
    // 1.3.2 tried a client branch referencing an undefined CheckPlayerAuth/GUID;
    // on Node 22 that undefined reference throws an *unhandled* rejection (this
    // async handler is not awaited) and crashes the worker — a DoS. We enforce the
    // documented server-only contract explicitly instead. Server path unchanged;
    // non-server auth gets a clean 401 (conscious change, not silent — see report).
    if ( CheckServerAuth(auth) ){
        let RawData = req.body;
        try{
            let element = RawData.Element;
            let operation = RawData.Operation || "set";
            // Injection-safe value + computed field-path key.
            let value = CoerceUpdateValue(RawData.Value);
            const db = await getDb();
            let collection = db.collection("Globals");
            let query = { Mod: mod };
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
            if ( result.matchedCount >= 1 || result.upsertedCount >= 1){
                log("Updated " + element +" for "+ mod + " Globals");
                res.status(200);
                res.json({ Status: "Success", Element: element, Mod: mod, ID: "Globals"});
            } else {
                log("Error with Updating " + element +" for "+ mod + " Globals", "warn");
                res.status(203);
                res.json({ Status: "Error", Element: element, Mod: mod, ID: "Globals"});
            }
        }catch(err){
            log(`ERROR: ${err}`, "warn");
            res.status(203);
            res.json({ Status: "Error", Element: RawData.Element, Mod: RawData.mod, ID: "Globals"});
        }
    } else {
        res.status(401);
        res.json({ Status: "Error", Error: "Invalid Auth" , Element: "", Mod: mod, ID: "Globals"});
        log("AUTH ERROR: " + req.url, "warn");
    }
};
module.exports = router;
