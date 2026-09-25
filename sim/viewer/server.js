// Slim static file server for the Cube Playback viewer -- no dependencies,
// just Node's built-in http/fs. Serves everything under public/ on a local
// port so the page can fetch .jsonl recordings (opening index.html directly
// via file:// blocks that fetch in most browsers due to CORS).
//
// It also bridges the live frame stream: /live connects to bin/sim-socket's
// --stream-port and forwards every frame to the page as Server-Sent Events,
// so the viewer can watch the cube in real time while cube-client drives it.
// SSE rather than a WebSocket purely to keep this file dependency-free.
//
// Usage: node sim/viewer/server.js [port] [streamPort]

const http = require('http');
const fs = require('fs');
const net = require('net');
const path = require('path');

const port = Number(process.argv[2]) || 8420;
const streamPort = Number(process.argv[3]) || 8421;
const publicDir = path.join(__dirname, 'public');

const contentTypes = {
  '.html': 'text/html; charset=utf-8',
  '.js': 'application/javascript; charset=utf-8',
  '.json': 'application/json; charset=utf-8',
  '.jsonl': 'application/json; charset=utf-8',
  '.css': 'text/css; charset=utf-8',
};

// One SSE connection per watching page, each with its own TCP connection to
// the frame stream. Frames are newline-delimited JSON upstream and SSE
// frames downstream, so this only has to re-split on newlines: a frame is
// ~4 KB and will not arrive in one piece.
function serveLive(res, req) {
  res.writeHead(200, {
    'Content-Type': 'text/event-stream; charset=utf-8',
    'Cache-Control': 'no-cache',
    Connection: 'keep-alive',
  });

  const upstream = net.connect(streamPort, '127.0.0.1');
  let buffer = '';

  upstream.on('data', (chunk) => {
    buffer += chunk;
    let newline;
    while ((newline = buffer.indexOf('\n')) !== -1) {
      const line = buffer.slice(0, newline);
      buffer = buffer.slice(newline + 1);
      if (line) {
        res.write('data: ' + line + '\n\n');
      }
    }
  });

  // Nothing listening on streamPort is the normal case (no sim-socket
  // running), not a crash -- tell the page so it can say so.
  upstream.on('error', (err) => {
    res.write('event: offline\ndata: ' + JSON.stringify({ message: err.message }) + '\n\n');
  });
  upstream.on('close', () => res.end());
  req.on('close', () => upstream.destroy());
}

const server = http.createServer((req, res) => {
  let reqPath = decodeURIComponent(req.url.split('?')[0]);
  if (reqPath === '/') reqPath = '/index.html';

  if (reqPath === '/live') {
    serveLive(res, req);
    return;
  }

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
  console.log(`Live-feed hentes fra sim-socket på port ${streamPort}`);
});
