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
let {createHash} = require('crypto');

const {isArray, isObject, makeObjectId, isEmpty, CoerceUpdateValue} = require('./utils')
const {getDb} = require('./db');

const log = require("./log");

const queryHandler = require("./Query");
const TransactionHandler = require("./Transaction");

const router = Router();

router.use('/Query', queryHandler);
router.use('/Transaction', TransactionHandler);
router.post('/Load/:ObjectId/:mod', (req, res)=>{
    runGet(req, res, req.params.ObjectId, req.params.mod, req.headers['auth-key']);
});

router.post('/Save/:ObjectId/:mod', (req, res)=>{
    runSave(req, res, req.params.ObjectId, req.params.mod, req.headers['auth-key']);
});

router.post('/Update/:ObjectId/:mod', (req, res)=>{
    runUpdate(req, res, req.params.ObjectId, req.params.mod, req.headers['auth-key']);
});


async function runGet(req, res, ObjectId, mod, auth) {
    if (CheckServerAuth(auth) || (await CheckAuth(auth)) ){
        let RawData = req.body;
        try{
            const db = await getDb();
            let collection = db.collection("Objects");
            let query = { ObjectId: ObjectId, Mod: mod };
            let results = collection.find(query);
            if ((await collection.countDocuments(query)) == 0){
                if ((CheckServerAuth(auth) || global.config.AllowClientWrite) && !isEmpty(RawData)){
                    if (ObjectId == "NewObject"){
                        ObjectId = makeObjectId();
                        RawData.ObjectId = ObjectId;
                        console.log("Item called as NewObject for " + mod + " Generating ID " + ObjectId);
                    }
                    console.log("Can't find Object for mod " + mod + " with ID " + ObjectId + " Creating it now");
                    const doc  = {ObjectId: ObjectId, Mod: mod, Data: RawData}
                    // mongodb 4+ removed result.ops — the old `result.ops[0]` read was dead code.
                    await collection.insertOne(doc);
                }
                res.status(201);
                res.json(RawData);
            } else {
                let dataarr = await results.toArray();
                let data = dataarr[0];
                if (typeof data.Data !== 'undefined' && data.Data){
                    res.status(200);
                    res.json(data.Data);
                } else {
                    res.status(203);
                    res.json(RawData);
                }
            }
        }catch(err){
            console.log("ERROR: " + err);
            res.status(203);
            res.json(RawData);
        }
    }  else {
        res.status(401);
        res.json(req.body);
        console.log("ERROR: Bad Auth Token");
    }
};

async function runSave(req, res, ObjectId, mod, auth) {
    if (CheckServerAuth(auth) || ((await CheckAuth(auth)) && global.config.AllowClientWrite) ){
        let RawData = req.body;
        try{
            const db = await getDb();
            let collection = db.collection("Objects");
            if (ObjectId == "NewObject"){
                ObjectId = makeObjectId();
                RawData.ObjectId = ObjectId;
            }
            let query = { ObjectId: ObjectId, Mod: mod };
            const options = { upsert: true };
            const updateDocValue  =  {ObjectId: ObjectId, Mod: mod, Data: RawData};
            const updateDoc = { $set: updateDocValue, };
            const result = await collection.updateOne(query, updateDoc, options);
            if (result.matchedCount === 1 || result.upsertedCount === 1 ){
                log("Updated "+ mod + " Data for Object: " + ObjectId);
                res.status(201);
                res.json(RawData);
            } else {
                log("Error with Updating "+ mod + " Data for Object: " + ObjectId, "warn");
                res.status(203);
                res.json(RawData);
            }
        }catch(err){
            log("err " + err, "warn")
            res.json(RawData);
        }
    }  else {
        res.status(401);
        res.json(req.body);
        log("ERROR: Bad Auth Token", "warn");
    }
};

async function runUpdate(req, res, ObjectId, mod, auth) {
    if ( CheckServerAuth(auth) || ((await CheckAuth(auth)) && global.config.AllowClientWrite) ){
        let RawData = req.body;
        try{
            let element = RawData.Element;
            let operation = RawData.Operation || "set";
            // Injection-safe value + computed field-path key (see player.js).
            let value = CoerceUpdateValue(RawData.Value);
            const db = await getDb();
            let collection = db.collection("Objects");
            let query = { ObjectId: ObjectId, Mod: mod };
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
                log("Updated " + element +" for "+ mod + " Data for ObjectId: " + ObjectId);
                res.status(200);
                res.json({ Status: "Success", Element: element, Mod: mod, ID: ObjectId});
            } else {
                log("Error with Updating " + element +" for "+ mod + " Data for ObjectId: " + ObjectId, "warn");
                res.status(203);
                res.json({ Status: "NotFound", Element: element, Mod: mod, ID: ObjectId});
            }
        }catch(err){
            log(`ERROR: ${err}`, "warn");
            res.status(203);
            res.json({ Status: "Error", Element: RawData.Element, Mod: mod, ID: ObjectId});
        }
    } else {
        res.status(401);
        res.json({ Status: "Error", Error: "Invalid Auth", Element: "", Mod: mod, ID: ObjectId});
        log("AUTH ERROR: " + req.url, "warn");
    }
};

module.exports = router;
