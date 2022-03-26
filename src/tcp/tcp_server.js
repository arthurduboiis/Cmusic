const Net = require("net")
const context = require("../core/core_ctx");
const tcp_session = require("./tcp_session");

const TCP_PORT = 8080

class tcp_server{
    constructor(config){
        config.tcp.port = this.port = config.tcp.port ? config.tcp.port : TCP_PORT
        this.tcp_server = Net.createServer((socket => {
            let session = new tcp_session(config, socket)
            session.run()
        }))

    }

    run(){
        this.tcp_server.listen(this.port, () => {
            console.log("[TCP] Listen on port " + this.port)
        })

        this.tcp_server.on('error', (e) => {
            console.log("[TCP]  " + e)
        })

        this.tcp_server.on('close', () => {
            console.log("[TCP] close")
        })
    }
    stop() {
        this.tcp_server.close();

        context.sessions.forEach((session, id) => {
            if (session instanceof tcp_session)
                session.stop();
        });
    }
}

module.exports = tcp_server