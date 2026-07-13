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

// TensorFlow toxicity is now OPTIONAL and behind the Toxicity.Enabled config flag.
// The ~250 MB @tensorflow deps are lazy-required only when the flag is on, and the
// model is loaded ONCE and cached (the original reloaded it on every request).
// When disabled, the route answers 501 without pulling any TF dependency.

const {Router} = require('express');
const log = require("./log");

const {CheckAuth,CheckServerAuth} = require('./AuthChecker')
const {isArray,GenerateLimiter} = require('./utils');

const router = Router();

router.use(GenerateLimiter(global.config.RequestLimitStatus || 100, 10));

// The minimum prediction confidence.
const threshold = 0.8;

let toxicityModel;      // cached, loaded once
let toxicityLoading;    // in-flight load promise

function IsToxicityEnabled(){
    return (global.config.Toxicity !== undefined && global.config.Toxicity.Enabled === true);
}

async function GetToxicityModel(){
    if (toxicityModel !== undefined) return toxicityModel;
    if (toxicityLoading === undefined){
        toxicityLoading = (async () => {
            // Lazy require so the TF native deps are only loaded when actually used.
            require('@tensorflow/tfjs');
            const toxicity = require('@tensorflow-models/toxicity');
            let model = await toxicity.load(threshold);
            toxicityModel = model;
            return model;
        })().catch((e) => {
            toxicityLoading = undefined; // allow a later retry
            throw e;
        });
    }
    return toxicityLoading;
}

/**
 *  Toxicity Checker
 *  Post: /Toxicity
 *
 *  Description: This uses TensorsFlows Toxicity Classifier to check the Toxicity of a given Text
 *
 *  Accepts: `{ "Text": "|TEXTTOCLASSIFY|" }`
 *
 *  Returns: `{
 *               "Status": "|STATUSOFSERVER|",
 *               "Error": "|ANYERRORMESSAGE|",
 *               "Toxicity": |0-1SCALE|,
 *               "Threat": |0-1SCALE|,
 *               "SexualExplicit": |0-1SCALE|,
 *               "SevereToxicity": |0-1SCALE|,
 *               "Insult": |0-1SCALE|,
 *               "IdentityAttack": |0-1SCALE|
 *            }`
 *
 */
router.post('', (req, res)=>{
    runToxicity(req, res, req.headers['auth-key']);
});

async function runToxicity(req, res, auth, key){
    if (!IsToxicityEnabled()){
        res.status(501);
        res.json({Status: "Disabled", Error: "Toxicity is disabled (set Toxicity.Enabled = true to use it)" });
        return;
    }
    if ( CheckServerAuth( auth ) || (await CheckAuth( auth )) ){
        try {
            let model = await GetToxicityModel();
            let query = req.body.Question || req.body.Text;
            const sentences = [query];
            let predictions = await model.classify(sentences);
            let response = {Status: "Success"};
            predictions.forEach(e => {
                let label = e["label"];
                switch (label){
                    case "identity_attack":
                        label = "IdentityAttack";
                        break;
                    case "insult":
                        label = "Insult";
                        break;
                    case "obscene":
                        label = "Obscene";
                        break;
                    case "severe_toxicity":
                        label = "SevereToxicity";
                        break;
                    case "sexual_explicit":
                        label = "SexualExplicit";
                        break;
                    case "threat":
                        label = "Threat";
                        break;
                    case "toxicity":
                        label = "Toxicity";
                        break;
                }
                response[label] = Math.round(e.results[0].probabilities[1] * 1000) / 1000;
            })
            res.status(200);
            res.json(response);
        } catch (e){
            console.log(e)
            log(e, "warn")
            res.status(203);
            res.json({Status: "Error", Error: `${e}` });
        }
    } else {
        res.status(401);
        res.json({Status: "Error", Error: "Invalid Auth" });
    }

}

module.exports = router;
