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

const {readFileSync,writeFileSync} = require('fs');
const {makeAuthToken} = require('./utils')

const Defaultconfig = require('./sample-config.json');
const ConfigPath = "config.json"
let config;
try{
  config = JSON.parse(readFileSync(global.SAVEPATH + ConfigPath));
} catch (err){
  // SECURITY: first-run ServerAuth (also the default JWT signing secret) is now
  // generated with a CSPRNG (crypto.randomBytes via utils.makeAuthToken).
  Defaultconfig.ServerAuth = makeAuthToken();
  config = Defaultconfig;
  writeFileSync(global.SAVEPATH + ConfigPath, JSON.stringify(Defaultconfig, undefined, 4))
  console.log("Installing for the first time the default config \"" + ConfigPath + "\" was created with the following values");
  console.log("   DBServer: "  + config.DBServer);
  console.log("   DB: "  + config.DB);
  console.log("   ServerAuth: "  + config.ServerAuth);
  console.log("   AllowClientWrite: "  + config.AllowClientWrite);
  console.log("   Port: "  + config.Port);

  if (global.mainWindow !== undefined){
    global.mainWindow.send("log",{type: "warn", message: "Installing for the first time the default config \"" + ConfigPath + "\" was created with the following values"})
    global.mainWindow.send("log",{type: "warn", message: "   DBServer: "  + config.DBServer})
    global.mainWindow.send("log",{type: "warn", message: "   DB: "  + config.DB})
    global.mainWindow.send("log",{type: "warn", message: "   ServerAuth: "  + config.ServerAuth})
    global.mainWindow.send("log",{type: "warn", message: "   AllowClientWrite: "  + config.AllowClientWrite})
    global.mainWindow.send("log",{type: "warn", message: "   Port: "  + config.Port})
  }
  if (global.logs !== undefined){
    global.logs.push({type: "warn", message: "Installing for the first time the default config \"" + ConfigPath + "\" was created with the following values"})
    global.logs.push({type: "warn", message: "   DBServer: "  + config.DBServer})
    global.logs.push({type: "warn", message: "   DB: "  + config.DB})
    global.logs.push({type: "warn", message: "   ServerAuth: "  + config.ServerAuth})
    global.logs.push({type: "warn", message: "   AllowClientWrite: "  + config.AllowClientWrite})
    global.logs.push({type: "warn", message: "   Port: "  + config.Port})
  }
}

if (config.IP === undefined || config.IP === null){
  config.IP = "0.0.0.0";
  try {
   writeFileSync(global.SAVEPATH + ConfigPath, JSON.stringify(config, undefined, 4))
  } catch(e) {
    console.log(e)
  }
}

// LetsEncypt (sic — Wire/config key spelling is preserved) is kept for config
// compatibility but the built-in greenlock/ACME TLS path has been removed. It is
// treated as disabled; terminate TLS at a reverse proxy instead.
if (config.LetsEncypt === undefined || config.LetsEncypt === null){
  config.LetsEncypt = {Enabled: false, Domain: "yourdomain.com", Email: "jon@example.com", AltNames: []};
  try {
   writeFileSync(global.SAVEPATH + ConfigPath, JSON.stringify(config, undefined, 4))
  } catch(e) {
    console.log(e)
  }
}

if (config.CreateIndexes === undefined || config.CreateIndexes === null){
  config.CreateIndexes = true;
  try {
   writeFileSync(global.SAVEPATH + ConfigPath, JSON.stringify(config, undefined, 4))
  } catch(e) {
    console.log(e)
  }
}

module.exports = config;
