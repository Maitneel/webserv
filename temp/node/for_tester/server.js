const { create } = require('domain');
const http = require('http');
const port = 8080;

const server = http.createServer((req, res) => {
    routing(req, res);
})

server.listen(port, () => {
    console.log("listen on " + port);
})

let tests = [
    {req: {method: "GET", url: "/"}, responceStatus: 200},
    {req: {method: "HEAD", url: "/"}, responceStatus: 405},
    {req: {method: "POST", url: "/"}, responceStatus: 405},
    {req: {method: "GET", url: "/directory"}, responceStatus: 200},
    {req: {method: "GET", url: "/directory/youpi.bad_extension"}, responceStatus: 200},
    {req: {method: "GET", url: "/directory/youpi.bla"}, responceStatus: 200},
    {req: {method: "GET", url: "/directory/oulalala"}, responceStatus: 404},
    {req: {method: "GET", url: "/directory/nop/"}, responceStatus: 200},
    {req: {method: "GET", url: "/directory/nop"}, responceStatus: 200},
    {req: {method: "GET", url: "/directory/nop/other.pouic"}, responceStatus: 200},
    {req: {method: "GET", url: "/directory/nop/other.pouac"}, responceStatus: 404},
    {req: {method: "GET", url: "/directory/Yeah"}, responceStatus: 404},
    {req: {method: "GET", url: "/directory/Yeah/not_happy.bad_extension"}, responceStatus: 200},
    {req: {method: "GET", url: "/directory/Yeah/not_happy.bad_extension"}, responceStatus: 200},
]


function createRequestObject(req) {
    return {method: req.method, url: req.url, header: req.headers};
}

let checkingRequest = null;

let flag = false;
function routing(req, res) {
    for (let i = 0; i < tests.length; i++) {
        const test_para = tests[i].req;
        if (test_para.method == req.method && test_para.url == req.url) {
            res.writeHead(tests[i].responceStatus);
            res.end();
            return;
        }
    }
    if (checkingRequest === null) {
        checkingRequest = createRequestObject(req);
    } else if (!(JSON.stringify(checkingRequest) == JSON.stringify(createRequestObject(req)))) {
        console.log("----------------------------");
        console.log("req: ", checkingRequest);
        console.log("status: ", forceCheckResCode - 1);
        // process.exit(0);
    }
    // forceCheck(req, res);
    echoBody(req, res);
}

function echoBody(req, res) {
    res.writeHead(200);
    let body = "";
    req.on('data', (buf) => {
        body += (buf.toString().toUpperCase());
    })
    req.on('end', () => {
        res.write(body);
        res.end()
    });
}

let forceCheckResCode = 200;

function forceCheck(req, res) {
    console.log(req.method, req.url, forceCheckResCode);
    res.writeHead(forceCheckResCode);
    res.end();
    forceCheckResCode++;
}