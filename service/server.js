var EventEmitter = require('events').EventEmitter,
    events = new EventEmitter(),
    boundary = "videostream12345";


/********************** HTTP SERVER ********************************/
var send404 = function(response){
	response.writeHead(404,{'Content-Type':'text/html'});
	response.write('No encontrado');
	response.end();
}

var sended=false;

var express = require('express'),
        fs  = require('fs'),
        app = express.createServer();
    
    app.get('/',function(req,response){
        fs.readFile(__dirname + '/index.html',function(err,data){
           if(err) return send404(response);
           response.writeHead(200,{'Content-Type':'text/html'});
           response.write(data,'utf8');
           response.end(); 
        });
    });
    
    app.get('/videoStream',function(req,response){   
        response.writeHead(200,{
            'Content-Type': 'multipart/x-mixed-replace;boundary="' + boundary + '"',
            'Connection': 'keep-alive',
            'Expires': 'Fri, 01 Jan 1990 00:00:00 GMT',
            'Cache-Control': 'no-cache, no-store, max-age=0, must-revalidate',
            'Pragma': 'no-cache'
        });
        response.write('--'+boundary+'\n');
        
        events.addListener('imagen_recibida',function(){
            fs.readFile(__dirname + '/image.jpeg',function(err,data){
                 if(err) return send404(response);
                 response.write('Content-Type: image/jpeg\n Content-Length: '+data.length+'\n\n');
                 response.write(data);
               	response.write('\n--'+boundary+'\n');
               });
        });
    });
   

app.listen(8080,function(){
    console.log('Escuchando en el puerto 8080');
});
/*****************************************************************************


/********************* THRIFT SERVER CONFIGURATION ***************************/

var thrift = require('thrift'),
    Buffer = require('buffer').Buffer,
    ImageService = require('./ImageService.js'),
	ttypes = require('./service_types.js');


//funcion llamada al recibir una imagen desde el client thrift (c++/opencv)
var receiveImage = function(image,success){
     buf = new Buffer(image.data.toString('base64'),'base64');
     fs.writeFile('image.jpeg',buf,function(err){
        if(err) console.log(err);
     });
     events.emit('imagen_recibida');
     success(true);
}



var Tserver = thrift.createServer(ImageService,{
	receiveImage: receiveImage
});

Tserver.listen(9090);





    




