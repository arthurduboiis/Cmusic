const port = 46500;
var net = require('net');
var server = net.createServer(function(connection) {
    console.log('client connected');

    connection.on('close', function (){
        console.log('client disconnected');
     });

    connection.on('data', function (data) {
    data = data.toString();
    console.log('client sended the folowing string:'+data);
    //connection.write("Response");
    console.log('Sended responst to client');
    
    connection.end();
    console.log('Disconnected the client.');
        
        
   });

});


server.listen(port, function () {
    console.log('server is listening');
});




