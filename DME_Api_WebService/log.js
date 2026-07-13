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

const log4js = require('log4js');
let datetime = new Date();
let date = datetime.toISOString().slice(0,10)
let logfilename = global.SAVEPATH + "logs/api-warnings-" + date + ".log";
if (global.config?.LogToFile){
    log4js.configure({
        appenders: { logs: { type: "file", filename: logfilename } },
        categories: { default: { appenders: ["logs"], level: "warn" } }
    });
} 
let logger = log4js.getLogger('logs'); 

module.exports = function(message, type = "info"){

    if (global.config?.LogToFile && type === "warn") logger.warn(`${message}`);

    //For Desktop Version, Easier to maintain one version of the API
    if (global.mainWindow !== undefined) global.mainWindow.send("log",{type: type, message: message})
    if (global.logs !== undefined) global.logs.push({type: type, message: message});

    if (type === "warn")
        console.log("\x1b[33m", `${message}`,'\x1b[0m')
    else if (type === "info")
        console.log("\x1b[36m", `${message}`,'\x1b[0m')
    else 
        console.log(`${message}`)
}