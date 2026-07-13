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
const {isArray, isObject, isEmpty, NormalizeToGUID, CoerceUpdateValue} = require('./utils')
const log = require("./log");
const {getDb} = require('./db');

const {CheckAuth, CheckPlayerAuth,CheckServerAuth} = require('./AuthChecker')


const queryHandler = require("./Query");
const TransactionHandler = require("./Transaction");


const router = Router();
router.use('/Query', queryHandler);
router.use('/Transaction', TransactionHandler);
router.post('/Load/:GUID/:mod', (req, res)=>{
    let GUID = NormalizeToGUID(req.params.GUID);
    runGet(req, res, GUID, req.params.mod, req.headers['auth-key']);
});
router.post('/Save/:GUID/:mod', (req, res)=>{
    let GUID = NormalizeToGUID(req.params.GUID);
    runSave(req, res, GUID, req.params.mod, req.headers['auth-key']);
});

router.post('/Update/:GUID/:mod', (req, res)=>{
    let GUID = NormalizeToGUID(req.params.GUID);
    runUpdate(req, res, GUID, req.params.mod, req.headers['auth-key']);
});

router.post('/PublicLoad/:GUID/:mod', (req, res)=>{
    let GUID = NormalizeToGUID(req.params.GUID);
    runGetPublic(req, res, GUID, req.params.mod, req.headers['auth-key']);
});

router.post('/PublicSave/:GUID/:mod', (req, res)=>{
    let GUID = NormalizeToGUID(req.params.GUID);
    runSavePublic(req, res, GUID, req.params.mod, req.headers['auth-key']);
});


async function runGet(req, res, GUID, mod, auth) {
    if (  CheckServerAuth(auth) || (await CheckPlayerAuth(GUID, auth))){
        try{
            const db = await getDb();
            let collection = db.collection("Players");
            let query = { GUID: GUID };
            let results = collection.find(query);
            let RawData = req.body;

            if ((await collection.countDocuments(query)) == 0){
                if ((CheckServerAuth(auth) || global.config.AllowClientWrite) && !isEmpty(RawData)){
                    log("Can't find Player with ID " + GUID + "Creating it now");
                    // Build the document programmatically instead of concatenating
                    // the mod name / body into a JSON string (injection-safe).
                    const doc = { GUID: GUID };
                    doc[mod] = RawData;
                    await collection.insertOne(doc);
                } else {
                    log("Can't find Player with ID " + GUID, "warn");
                }
                res.status(201);
                res.json(RawData);
            } else {
                let dataarr = await results.toArray();
                let data = dataarr[0];
                let sent = false;
                for (const [key, value] of Object.entries(data)) {
                    if(key === mod){
                        sent = true;
                        res.json(value);
                        log("Retrieving "+ mod + " Data for GUID: " + GUID);
                    }
                }
                if (sent != true){
                    if ((CheckServerAuth(auth) || global.config.AllowClientWrite) && !isEmpty(RawData)){
                        const updateDocValue = {};
                        updateDocValue[mod] = RawData;
                        const updateDoc = { $set: updateDocValue, };
                        const options = { upsert: false };
                        await collection.updateOne(query, updateDoc, options);
                        log("Can't find "+ mod + " Data for GUID: " + GUID +  " Creating it now");
                    } else {
                        log("Can't find "+ mod + " Data for GUID: " + GUID, "warn");
                    }
                    res.status(203);
                    res.json(RawData);
                }
            }
        }catch(err){
            res.status(203);
            res.json(req.body);
            log("ERROR: " + err, "warn");
        }
    } else {
        res.status(401);
        res.json(req.body);
    }
};
async function runSave(req, res, GUID, mod, auth) {
    if ( CheckServerAuth(auth) || ((await CheckPlayerAuth(GUID, auth)) && global.config.AllowClientWrite) ){
        try{
            let RawData = req.body;
            const db = await getDb();
            let collection = db.collection("Players");
            let query = { GUID: GUID };
            const options = { upsert: true };
            const updateDocValue = { GUID: GUID };
            updateDocValue[mod] = RawData;
            const updateDoc = { $set: updateDocValue, };
            const result = await collection.updateOne(query, updateDoc, options);
            if (result.matchedCount === 1 || result.upsertedCount >= 1){
                log("Updated "+ mod + " Data for GUID: " + GUID);
                res.status(200);
                res.json(RawData);
            } else {
                log("Error with Updating "+ mod + " Data for GUID: " + GUID, "warn");
                res.status(203);
                res.json(req.body);
            }
        }catch(err){
            res.status(203);
            res.json(req.body);
            log("ERROR: " + err, "warn");
        }
    } else {
        res.status(401);
        res.json(req.body);
        log("AUTH ERROR: " + req.url, "warn");
    }
};


async function runUpdate(req, res, GUID, mod, auth) {
    if ( CheckServerAuth(auth) || ((await CheckPlayerAuth(GUID, auth)) && global.config.AllowClientWrite) ){
        let RawData = req.body;
        try{
            let element = RawData.Element;
            let operation = RawData.Operation || "set";
            // Injection-safe: value coerced to its native type, field path used as
            // a computed key (never concatenated into a parsed JSON string).
            let value = CoerceUpdateValue(RawData.Value);
            const db = await getDb();
            let collection = db.collection("Players");
            let query = { GUID: GUID };
            const options = { upsert: false };
            let updateDocValue = {};
            updateDocValue[`${mod}.${element}`] = value;

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
                log("Updated " + element +" for "+ mod + " Data for GUID: " + GUID);
                res.status(200);
                res.json({ Status: "Success", Element: element, Mod: mod, ID: GUID});
            } else {
                log("Error with Updating " + element +" for "+ mod + " Data for GUID: " + GUID, "warn");
                res.status(203);
                res.json({ Status: "NotFound", Element: element, Mod: mod, ID: GUID});
            }
        }catch(err){
            res.status(203);
            res.json({ Status: "Error", Element: RawData.Element, Mod: mod, ID: GUID});
            log("ERROR: " + err, "warn");
        }
    } else {
        res.status(401);
        res.json({ Status: "Error", Error: "Invalid Auth", Element: "", Mod: mod, ID: GUID});
        log("AUTH ERROR: " + req.url, "warn");
    }
};

async function runGetPublic(req, res, GUID, mod, auth) {
    try{
        const db = await getDb();
        let collection = db.collection("Players");
        let query = { GUID: GUID };
        let results = collection.find(query);
        let RawData = req.body;

        if ((await collection.countDocuments(query)) == 0){
            if (auth !== "null" && (CheckServerAuth(auth) || ((await CheckPlayerAuth(GUID, auth)) && global.config.AllowClientWrite))){
                log("Can't find Player with ID " + GUID + " Creating it now");
                const doc = { GUID: GUID, Public: {} };
                doc.Public[mod] = `${RawData.Value}`;
                await collection.insertOne(doc);
            } else {
                 log("Can't find Player with ID " + GUID, "warn");
            }
            res.status(201);
            res.json(RawData);
        } else {
            let dataarr = await results.toArray();
            let data = dataarr[0];
            let sent = false;
            if (data !== undefined && data.Public !== undefined)
            for (const [key, value] of Object.entries(data.Public)) {
                if(key === mod){
                    sent = true;
                    res.json({ "Value": value });
                    log("Retrieving "+ mod + " Data for GUID: " + GUID);
                }
            }
            if (sent !== true){
                if (auth !== "null" && (CheckServerAuth(auth) || ((await CheckPlayerAuth(GUID, auth)) && global.config.AllowClientWrite))){
                    const updateDocValue = {};
                    updateDocValue[`Public.${mod}`] = `${RawData.Value}`;
                    const updateDoc = { $set: updateDocValue, };
                    const options = { upsert: false };
                    await collection.updateOne(query, updateDoc, options);
                    log("Can't find "+ mod + " Data for GUID: " + GUID +  " Creating it now");
                } else {
                    log("Can't find "+ mod + " Data for GUID: " + GUID, "warn");
                }
                res.status(203);
                res.json(RawData);
            }
        }
    }catch(err){
        res.status(203);
        res.json({Value: "Error"});
        log("ERROR: " + err, "warn");
    }
};
async function runSavePublic(req, res, GUID, mod, auth) {
    if ( CheckServerAuth(auth) || ((await CheckPlayerAuth(GUID, auth)) && global.config.AllowClientWrite) ){
        try{
            let RawData = req.body;
            const db = await getDb();
            let collection = db.collection("Players");
            let query = { GUID: GUID };
            const options = { upsert: true };
            const updateDocValue = { GUID: GUID };
            updateDocValue[`Public.${mod}`] = `${RawData.Value}`;
            const updateDoc = { $set: updateDocValue, };
            const result = await collection.updateOne(query, updateDoc, options);
            if ( result.matchedCount === 1 || result.upsertedCount === 1 ){
                log("Updated "+ mod + " Data for GUID: " + GUID);
                res.status(200);
                res.json(RawData);
            } else {
                log("Error with Updating "+ mod + " Data for GUID: " + GUID, "warn");
                res.status(203);
                res.json(RawData);
            }
        }catch(err){
            res.status(203);
            res.json(req.body);
            log("ERROR: " + err, "warn");
        }
    } else {
        res.status(401);
        res.json(req.body);
        log("AUTH ERROR: " + req.url, "warn");
    }
};

module.exports = router;
