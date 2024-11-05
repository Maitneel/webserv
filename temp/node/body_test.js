const http = require('http');
const fs = require('fs');

const server = http.createServer((req, res) => {
    if (req.url == '/favicon.ico') {
        res.writeHead(404);    
        res.end();
        return;
    }
    console.log('[' + new Date().toISOString() + '] ' + req.method + ' ' + req.url + ' HTTP/' + req.httpVersion + ' ' + req.headers.host);  
    let resBody;
    if (req.url == '/cgi') {
        try {
            resBody = fs.readFileSync('./cgi/image_form.html');
        } catch (err) {
            console.log(err);
            resBody = "error";
        }
    } else {
        resBody = "hello at " + new Date()
    }
    res.writeHead(200, {
        'Content-Type': 'text/html; charset=utf-8'
    });
    res.write(resBody);
    res.end();
});

server.on('connection', (socket) => {
    socket.on('data', (chunk) => {
        console.log(chunk.toString());
    });
});

const port = 8080;
server.listen(port, () => {
    console.log("listen on " + port);
});
