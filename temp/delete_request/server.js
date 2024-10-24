const http = require('http');
const fs = require('fs');

const server = http.createServer((req, res) => {
    console.log('[' + new Date().toISOString() + '] ' + req.method + ' ' + req.url + ' HTTP/' + req.httpVersion + ' ' + req.headers.host + ' ' + req.headers.cookie);
    req.on('data', (chunk) => {
        console.log('body ' + chunk.toString());
    });
    try {
        let file_path = (req.url === "/script.js" ? "script.js" : "index.html");
        let content = fs.readFileSync(file_path);
        res.writeHead(200, {
            'Content-Type': 'text/html; charset=utf-8',
            'Set-Cookie': "hoge=hoge" 
        });
        res.write(content);
        res.end();
    } catch (e) {
        console.log("fail");
        res.writeHead(404, {
            'Content-Type': 'text/plain; charset=utf-8'
        });
        res.write("hello at " + new Date());
        res.end();
    }
});

const port = 8080;
server.listen(port, () => {
    console.log("listen on " + port);
})

