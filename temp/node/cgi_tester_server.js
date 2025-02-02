const http = require('http');

const port = 8080;

const server = http.createServer((req, res) => {
    console.log(new Date() + ' ' + req.method + ' ' + req.url);

    if (req.methdo == "GET") {
        getMethodHandler(req, res);
    } else if (req.method == "HEAD")  {
        // headMethodHandler(req, res);
        methodNotAllow(req, res);
    } else if (req.method == "POST") {
        // postMethodHandler(req, res);
        methodNotAllow(req, res);
    }
    res.end();
});

function getMethodHandler(req, res) {
    if (res.url == "/") {
        res.writeHead(200, "OK", {})
        res.write("ok\n");
        res.end();
    } else {
        res.writeHead(404);
        res.end();
    }
}

function headMethodHandler(req, res) {
    res.writeHead(200, "OK", {
        "Content-Length": "3"
    });
    res.end()
}

function postMethodHandler(req, res) {
    res.writeHead(201, "Created", {
        "Location": "/posted/"
    });
    res.write("ok\n");
    res.end();
}

function methodNotAllow(req, res) {
    res.writeHead(405);
    res.end();
}


server.listen(port, () => {
    console.log("listen on " + port);
});