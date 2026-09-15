// Slim static file server for the Cube Playback viewer -- no dependencies,
// just Node's built-in http/fs. Serves everything under public/ on a local
// port so the page can fetch .jsonl recordings (opening index.html directly
// via file:// blocks that fetch in most browsers due to CORS).
//
// Usage: node sim/viewer/server.js [port]

const http = require('http');
const fs = require('fs');
const path = require('path');

const port = Number(process.argv[2]) || 8420;
const publicDir = path.join(__dirname, 'public');

const contentTypes = {
  '.html': 'text/html; charset=utf-8',
  '.js': 'application/javascript; charset=utf-8',
  '.json': 'application/json; charset=utf-8',
  '.jsonl': 'application/json; charset=utf-8',
  '.css': 'text/css; charset=utf-8',
};

const server = http.createServer((req, res) => {
  let reqPath = decodeURIComponent(req.url.split('?')[0]);
  if (reqPath === '/') reqPath = '/index.html';

  const filePath = path.join(publicDir, reqPath);

  // Keep requests inside public/ -- reject any path that escapes it.
  if (!filePath.startsWith(publicDir)) {
    res.writeHead(403);
    res.end('Forbidden');
    return;
  }

  fs.readFile(filePath, (err, data) => {
    if (err) {
      res.writeHead(404, { 'Content-Type': 'text/plain' });
      res.end('Not found: ' + reqPath);
      return;
    }
    const ext = path.extname(filePath);
    res.writeHead(200, { 'Content-Type': contentTypes[ext] || 'application/octet-stream' });
    res.end(data);
  });
});

server.listen(port, () => {
  console.log(`Cube Playback kører på http://localhost:${port}`);
});
