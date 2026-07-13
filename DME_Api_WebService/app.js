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
 *
 * TLS/deployment: TLS termination has been moved OUT of Node. Run this service
 * as plain HTTP behind a reverse proxy (nginx / Caddy / Cloudflare) that
 * terminates TLS + ACME. Direct HTTPS is still supported if you point
 * Certificate/CertificateKey at real cert files on disk. The old greenlock
 * (LetsEncypt) auto-ACME path and the embedded self-signed key have been removed.
 * IMPORTANT: configure a reverse proxy that does NOT rewrite the Content-Type
 * header, since the client transports its auth token as the Content-Type value.
 */

if (global.APIVERSION === undefined) {
  global.APIVERSION = process.env.npm_package_version || require('./package.json').version;
}
global.STABLEVERSION = '0.0.0';
global.NEWVERSIONDOWNLOAD = `https://github.com/daemonforge/DayZ-UniveralApi/releases`;
if (global.SAVEPATH === undefined){
  global.SAVEPATH = "./";
}
/* Config File */
global.config = require('./configLoader');

const express = require('express');
const favicon = require("serve-favicon");
const {existsSync,readFileSync} = require('fs');
const http = require('http');
const https = require('https');
const cluster = require('cluster');

const {isArray, CheckRecentVersion, CheckIndexes, ExtractAuthKey, GenerateLimiter} = require('./utils');

// node-fetch removed — Node >=18 has a global fetch(). Nothing to assign.

let totalCPUs = global.config.cpuCount || 1;

if (totalCPUs < 1){
  totalCPUs = require('os').cpus().length
}

const log = require("./log");

const RouterItem = require('./Object');
const RouterPlayer = require('./player');
const RouterGlobals = require('./globals');
const RouterAuth = require('./Auth');
const RouterStatus = require('./Status');
const RouterQnA = require('./QnAMaker');
const RouterFowarder = require("./apiFowarder");
const RouterLogger = require("./logger");
const RouterDiscordConnector = require("./discordConnector");
const RouterWit = require("./witConnector");
const RouterLUIS = require("./luisConnector");
const RouterTranslate = require("./TranslateConnector");
const RouterServerQuery = require("./serverquery");
const RouterToxicity = require("./toxicityConnector");
const RouterTrueRandom = require("./TrueRandom");
const RouterCrypto = require("./crypto");

function startWebServer() {
  const webapp = express();

  // SECURITY: honour X-Forwarded-For only from proxies you trust. Configure
  // TrustProxy in config.json to match your topology (false | true | hop-count |
  // subnet/CIDR string). This makes req.ip the real client IP so rate limiting
  // and the IP whitelist can't be bypassed by spoofing forwarded headers.
  webapp.set('trust proxy', global.config.TrustProxy !== undefined ? global.config.TrustProxy : false);

  // apply rate limiter to all requests
  webapp.use(GenerateLimiter(global.config.RequestLimit || 500, 10));

  webapp.use(ExtractAuthKey);

  webapp.use((req, res, next) => {
    express.json({
        limit: '64mb'
    })(req, res, (err) => {
        if (err) {
            log(`Bad Request Sent to "${req.url}" Error: ${err}`);
            res.status(400);
            res.json({Status: "error", Error: `Bad Request ${err}`});
            return;
        }
        next();
    });
  });
  webapp.use(favicon(__dirname + '/public/favicon.ico'));
  webapp.use('/Object', RouterItem);
  webapp.use('/Player', RouterPlayer);
  webapp.use('/Gobals', RouterGlobals); //For Backwards Compatblity
  webapp.use('/Globals', RouterGlobals);
  webapp.use('/GetAuth', RouterAuth);
  webapp.use('/Status', RouterStatus);
  webapp.use('/QnAMaker', RouterQnA);
  webapp.use('/QnA', RouterQnA); //Switching to /QnA for new ai interface
  webapp.use('/Forward', RouterFowarder);
  webapp.use('/Logger', RouterLogger);
  webapp.use('/Discord', RouterDiscordConnector);
  webapp.use('/Wit', RouterWit);
  webapp.use('/LUIS', RouterLUIS);
  webapp.use('/Translate', RouterTranslate);
  webapp.use('/ServerQuery', RouterServerQuery);
  webapp.use('/Toxicity', RouterToxicity);
  webapp.use('/Random', RouterTrueRandom);
  webapp.use('/Crypto', RouterCrypto);

  webapp.use('/', (req,res)=>{
    if (req.url != '/'){
      log("Error invalid or is not a post Requested URL is:" + req.url);
    }
    res.status(501);
    res.json({Status: "Error", Error: "Reqested bad URL"});
  });

  let Port = process.env.PORT || global.config.Port || 8443;
  let ip = global.config.IP || "0.0.0.0";

  // Direct HTTPS is only used when real cert files are configured AND exist.
  // Otherwise we serve plain HTTP behind a reverse proxy (see header note).
  let useHttps = false;
  let ServerKey;
  let ServerCert;
  if (global.config.Certificate && global.config.CertificateKey && global.config.Certificate != "" && global.config.CertificateKey != ""){
    if (existsSync(global.config.Certificate) && existsSync(global.config.CertificateKey)){
      ServerKey = readFileSync(global.config.Certificate);
      ServerCert = readFileSync(global.config.CertificateKey);
      useHttps = true;
    } else {
      log(`Configured Certificate/CertificateKey were not found on disk — starting in HTTP mode (terminate TLS at your reverse proxy).`, "warn");
    }
  }

  if (global.config.LetsEncypt !== undefined && global.config.LetsEncypt.Enabled === true){
    log(`LetsEncypt is enabled in config, but the built-in greenlock/ACME path has been removed. Terminate TLS at a reverse proxy, or set Certificate/CertificateKey. Continuing without built-in TLS.`, "warn");
  }

  let server;
  if (useHttps){
    server = https.createServer({ key: ServerKey, cert: ServerCert }, webapp);
    server.listen(Port, ip, function () {
      log('API Webservice (HTTPS) started and is now listening on port "' + Port +'"!')
    });
  } else {
    server = http.createServer(webapp);
    server.listen(Port, ip, function () {
      log('API Webservice (HTTP) started and is now listening on port "' + Port +'"! Terminate TLS at your reverse proxy.')
    });
  }
  server.on('error', function (e) {
    log(e, "warn");
  });

}

function Start(isElectron){
  if (cluster.isMaster && totalCPUs > 1 && !isElectron) {
    // Fork workers.
    for (let i = 0; i < totalCPUs; i++) {
      cluster.fork();
    }
    cluster.on('exit', (worker, code, signal) => {
      cluster.fork();
    });
    if (global.config?.CheckForNewVersion){
      CheckRecentVersion();
    }
    setTimeout(CheckIndexes, 1000);
  } else {
    startWebServer();
    if (totalCPUs <= 1){

      if (global.config?.CheckForNewVersion){
        CheckRecentVersion();
      }

      setTimeout(CheckIndexes, 1000);

    }
  }
}

module.exports = Start;

Start();
