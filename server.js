// Imports
var express = require('express');
var app = express();
var server = require('http').createServer(app);
var io = require('socket.io').listen(server);
var serialPort = require("serialport");
var SerialPort = serialPort.SerialPort;
var redis = require("redis"),
    client = redis.createClient();

String.prototype.format = function() {
  var args = arguments;
  return this.replace(/{(\d+)}/g, function(match, number) { 
  return typeof args[number] != 'undefined'
    ? args[number]
    : match
  ;
  });
};

// Redis
var presetDefault = [[90,90],[90,90],[90,90],[90,90]]; //default value
var presetsHash = "presetsHash";
var presetsData = null;
client.on("error", function(err){
  console.log(err);
});
client.get(presetsHash,function(err,reply){
  if(err){
    console.log("error");
  } else {
    try{
      presetsData = JSON.parse(reply);
    } catch(e) {
      presetsData = presetDefault;
    }
    console.log("redis data: " + presetsData);
  }
});

var info = {
	name:'arduinode',
	brand:'powered by tanuki'
};

var port = 5005
var spport = '/dev/ttyUSB0';

// Expres config
app.configure(function(){
  app.set('views', __dirname + '/views');
  app.set('view engine', 'jade');
  app.set('view options', { layout: false });
  app.use(express.cookieParser());
  app.use(express.bodyParser());
  //app.use(express.methodOverride());
  //app.use(express.session({ secret: 'your secret here' }));
  //app.use(app.router);
  app.use(express.static(__dirname + '/public'));
});

// 'index'
app.get('/', function(req, res) {
	res.render('index',info);
});

// Start webserver
server.listen(port, function() {
  console.log(info.name + " app deployed and listening on port " + port);
});



// Sockets
io.set('log level',1); //reduce logging

io.sockets.on('connection', function (socket) {

  //Listen for FE Events

  socket.on('onSendCommand', function (data) {
    sp.write(data+'\n');
  });

  socket.on('onRedisData', function(data){
    if(client) {
      console.log('got: ' + data);
      client.set(presetsHash,JSON.stringify(data),redis.print);
    }
  });
  
  socket.on('disconnect', function (data) {
  });

  // start
  socket.emit('onConnect',{socketid:socket.id});
  socket.emit('redisData',{data:presetsData});

});



// Serial
console.log('connecting to serial port %s',spport);
var sp = new SerialPort(spport, {
  parser: serialPort.parsers.readline("\n")
});

sp.on("data", function (data) {
  console.log('serial data: ' + data);
});

serialPort.list(function (err, ports) {
  ports.forEach(function(port) {
    console.log(port.comName);
    console.log(port.pnpId);
    console.log(port.manufacturer);
  });
});


