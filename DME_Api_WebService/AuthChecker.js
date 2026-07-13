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

const {verify, sign} = require('jsonwebtoken');
const {createHash} = require('crypto');
const {isArray} = require('./utils');
const {getDb} = require('./db');

const log = require("./log");

// SECURITY: pin the accepted algorithm on verify (prevents algorithm-confusion).
// Tokens are signed with HS256, so verification only accepts HS256.
const JWT_ALGORITHMS = ['HS256'];

module.exports = {
    CheckAuth,
    CheckAuthAgainstGUID,
    CheckPlayerAuth,
    AuthPlayerGuid,
    CheckServerAuth,
    GetSigningAuth,
    makeAuthToken
}

function LogVerifyError(err, ignoreError){
    if (err.name == "TokenExpiredError"){
        log("Error: Auth Token is expired, it expired at " + err.expiredAt, "warn");
    } else if (ignoreError){ //Used in the status check to avoid the logs from filling up
        return;
    } else if (err.name == "JsonWebTokenError") {
        log("Auth Token is not valid", "warn");
    } else {
        log(err, "warn");
    }
}

// NOTE: previously these used the verify(token, secret, callback) form, whose
// return value jsonwebtoken discards -> the function always resolved falsy and
// the client-JWT auth path was effectively dead. Now uses synchronous verify()
// inside try/catch so the boolean actually propagates.
async function CheckAuth(auth, ignoreError = false){
    try {
        verify(auth, GetSigningAuth(), { algorithms: JWT_ALGORITHMS });
        return true;
    } catch (err){
        LogVerifyError(err, ignoreError);
        return false;
    }
}
async function CheckAuthAgainstGUID(auth, guid, ignoreError = false){
    try {
        let decoded = verify(auth, GetSigningAuth(), { algorithms: JWT_ALGORITHMS });
        return (guid === decoded.GUID);
    } catch (err){
        LogVerifyError(err, ignoreError);
        return false;
    }
}

function AuthPlayerGuid(auth, ignoreError = false){
    try {
        let decoded = verify(auth, GetSigningAuth(), { algorithms: JWT_ALGORITHMS });
        return decoded.GUID;
    } catch (err){
        LogVerifyError(err, ignoreError);
        return "";
    }
}

async function CheckPlayerAuth(guid, auth){
    let isAuth = false;
    if ((await CheckAuthAgainstGUID(auth, guid, true))){
        try{
            const db = await getDb();
            let collection = db.collection("Players");
            let SavedAuth = createHash('sha256').update(auth).digest('base64');
            let query = { GUID: guid, AUTH: SavedAuth };
                if ((await collection.countDocuments(query)) != 0){
                    isAuth = true;
                }
        } catch(err){
            log("ID " + guid + " err" + err, "warn");
        }
    }
    return isAuth;
}

function CheckServerAuth(auth){
    if (auth === undefined || auth === null) return false;
    if (isArray(global.config.ServerAuth) && (global.config.ServerAuth.find(element => element === auth) === auth)) return true;
    if (!isArray(global.config.ServerAuth) && global.config.ServerAuth === auth) return true;
    return false;
}

function GetSigningAuth(){
    // SECURITY (optional decoupling): if a dedicated JwtSecret is configured, use
    // it as the HMAC signing/verify secret so the server-auth *bearer* token is no
    // longer identical to the JWT signing key. Defaults to ServerAuth for wire and
    // rolling-update compatibility (running client tokens stay valid). See README.
    if(typeof global.config.JwtSecret === 'string' && global.config.JwtSecret !== ''){
        return global.config.JwtSecret;
    }
    if(isArray(global.config.ServerAuth)){
        return global.config.ServerAuth[0];
    } else {
        return global.config.ServerAuth;
    }
}



function makeAuthToken(GUID) {
    const player = { GUID: GUID };
    //Token expires in 46.5 minutes, tokens renew every 21-23 Minutes ensuring that if the API is down at the time of the renewal token will last till next retry
    let result = sign(player, GetSigningAuth(), { algorithm: 'HS256', expiresIn: 2800 });
    return result;
 }
