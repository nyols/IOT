const WebSocket = require('ws');
const SerialPort = require('serialport');

const port = new SerialPort('COM4', { // Ganti dengan nama Serial Port yang sesuai
    baudRate: 9600
});

const wss = new WebSocket.Server({ port: 8080 }); // Ganti port WebSocket dengan yang sesuai

wss.on('connection', function connection(ws) {
    console.log('WebSocket connected.');

    port.on('data', function (data) {
        const message = data.toString();
        ws.send(message);
    });

    ws.on('close', function () {
        console.log('WebSocket disconnected.');
    });
});
