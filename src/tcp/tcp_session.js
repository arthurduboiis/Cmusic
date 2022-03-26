
class tcp_session{
    constructor(config, socket){
        this.config = config
        this.socket = socket
        this.ip = socket.remoteAddress
        this.tag = 'tcp'
    }

    run(){
        this.socket.on('data', this.onSocketData.bind(this))
    }

    onSocketData(data){
        console.log(data.toString())
    }
}


module.exports = tcp_session