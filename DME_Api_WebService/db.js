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

// Shared, pooled MongoClient (mongodb 6). The original code created a brand new
// MongoClient (handshake + auth + TLS) for every single request and closed it
// afterwards. mongodb 6 pools connections internally, so we create exactly ONE
// client per process and reuse it. Never call close() per request.

const { MongoClient } = require("mongodb");
const log = require("./log");

let sharedClient;
let connectPromise;

function GetMongoClient(){
    if (sharedClient === undefined){
        // useUnifiedTopology was removed in the mongodb 4+ driver (no-op) — omitted.
        sharedClient = new MongoClient(global.config.DBServer);
    }
    return sharedClient;
}

async function getDb(){
    let client = GetMongoClient();
    if (connectPromise === undefined){
        connectPromise = client.connect().catch((e) => {
            // Allow a later request to retry the initial connection.
            connectPromise = undefined;
            log("MongoDB connection error: " + e, "warn");
            throw e;
        });
    }
    await connectPromise;
    return client.db(global.config.DB);
}

module.exports = { getDb, GetMongoClient };
