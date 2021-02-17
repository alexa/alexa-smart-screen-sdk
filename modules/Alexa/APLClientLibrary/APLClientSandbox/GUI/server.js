#!/usr/bin/env node

const path = require('path');
const express = require('express');
const fs = require('fs')
const app = express();

const INTERFACE = '127.0.0.1';
const PORT = 8000;

const LIB_PATH = path.join(__dirname, 'node_modules', 'apl-client');
const STATIC_PATH = path.join(__dirname);

const getExtraPath = () => {
    const args = process.argv.slice(2);
    return  (args.length > 0) ? args[0] : undefined;
};

const logRequest = (req, res, next) => {
    console.log(`${req.method} ${req.originalUrl}`);
    next();
};

// routes
app.use(logRequest);

app.use(function(req, res, next) {
    res.header("Access-Control-Allow-Origin", "*");
    res.header("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
    next();
});

app.use(express.static(LIB_PATH));
app.use(express.static(STATIC_PATH));

// startup
const extraPath = getExtraPath();
if (extraPath) {
    app.use(express.static(extraPath));
    console.log('Adding ' + extraPath + ' to /');
}

app.get('/templates', (req, res) => {
    var examples = [];
    var root = "public"
    var documents = new Set();
    fs.readdirSync(path.join(root, "templates")).forEach(file => documents.add(file));
    documents.forEach(filename => {
        if (path.extname(filename) === '.json') {
            examples.push({
                name: filename
            })
        }
    });
    res.set('Cache-Control', 'no-cache, no-store, must-revalidate').send(examples);
});

app.get("/templates/*.json", async (req, res) => {
    var root = "public"
    fs.readFile(path.join(root, req.originalUrl), 'utf8', (err, template) => {
        if (err) {
            return res.status(404).send(`Could not load template at ${req.originalUrl}`);
        }
        res.set('Cache-Control', 'no-cache, no-store, must-revalidate').send(template);
    });
});

console.log('Listening on ' + INTERFACE  + ':' + PORT);
app.listen(PORT, INTERFACE);

