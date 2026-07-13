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
const {CheckAuth,CheckServerAuth} = require('./AuthChecker');
const log = require("./log");
const {isArray,GenerateLimiter} = require('./utils');
const {getDb} = require('./db');

const router = Router();


router.use(GenerateLimiter(global.config.RequestLimitStatus || 100, 10));

router.post('', (req, res)=>{
    runStatusCheck(req, res, req.headers['auth-key']);
});

router.post('/:Auth', (req, res)=>{
    runStatusCheck(req, res, req.params.Auth);
});

router.get('', (req, res)=>{
    runStatusCheck(req, res, req.headers['auth-key']);
});

router.get('/:Auth', (req, res)=>{
    runStatusCheck(req, res, req.params.Auth);
});

let WitsEnabled = [];
let TranslatesEnabled  = "Disabled";
let QnAEnabled = [];
let LUISEnabled = [];
if (global.config.Wit !== undefined){
    Object.keys(global.config.Wit).forEach(function (k) {
        WitsEnabled.push(k);
    })
}
if (global.config.LUIS !== undefined){
    Object.keys(global.config.LUIS).forEach(function (k) {
        if (global.config.LUIS[k].SubscriptionKey !== ""){
            LUISEnabled.push(k);
        }
    })
}
if (global.config.QnA !== undefined){
    Object.keys(global.config.QnA).forEach(function (k) {
        if (global.config.QnA[k].EndpointKey !== ""){
            QnAEnabled.push(k);
        }
    })
}
if (global.config.Translate !== undefined){
    if ( (global.config.Translate.Type === "LibreTranslate" && global.config.Translate.Endpoint !== "" ) || (global.config.Translate.SubscriptionKey !== "" &&  global.config.Translate.SubscriptionRegion !== "") ){
        TranslatesEnabled = "Enabled";
    }
}


async function runStatusCheck(req, res, auth) {
    var returnError = "noauth"
    if (CheckServerAuth(auth) || (await CheckAuth(auth, true))){
        returnError = "noerror"
    }
    try{
        const db = await getDb();
        var collection = db.collection("Globals");
        var query = { Mod: "UniversalApiStatus"};
        const options = { upsert: true };
        var TestValue = Math.random();
        const updateDocValue  = { Mod: "UniversalApiStatus", Description: "This Object Exsits as a test when ever the status url is called to make sure the database is writeable", TestVar: TestValue }
        const updateDoc = { $set: updateDocValue, };
        const result = await collection.updateOne(query, updateDoc, options);
        if (result.modifiedCount >= 1 || result.upsertedCount >= 1 ){
            res.json({Status: "Success", Error: returnError, Version: global.APIVERSION, Discord: global.DISCORDSTATUS, Translate: TranslatesEnabled, Wit: WitsEnabled, QnA: QnAEnabled, LUIS: LUISEnabled });
           // log("Status Check Called", "info");
        } else {
            res.status(500);
            res.json({Status: "Error", Error: "Database Write Error", Version: global.APIVERSION, Discord: global.DISCORDSTATUS, Translate: TranslatesEnabled, Wit: WitsEnabled, QnA: QnAEnabled, LUIS: LUISEnabled });
            log("ERROR: Database Write Error", "warn");
        }
    }catch(err){
        console.log(err);
        res.status(500);
        res.json({Status: "Error", Error: `Error: ${err}`, Version: global.APIVERSION, Discord: global.DISCORDSTATUS, Translate: TranslatesEnabled, Wit: WitsEnabled, QnA: QnAEnabled, LUIS: LUISEnabled });
        log("ERROR: " + err, "warn");
    }
}

module.exports = router;
