const http = require('http');

const server = http.createServer((req, res) => {
    console.log('[' + new Date().toISOString() + '] ' + req.headers['user-agent'] + ' ' + req.url);
    res.writeHead(200, {
        'Content-Type': 'text/plain; charset=utf-8'
    });
    res.write("hello at " + new Date());
    res.end();
});

const port = 8080;
server.listen(port, () => {
    console.log("listen on " + port);
})

