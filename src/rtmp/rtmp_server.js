const Net = require("net")
const rtmp_session = require("./rtmp_session")
const context = require("../core/core_ctx")

const RTMP_PORT = 1935

class rtmp_server{
    constructor(config){
        config.rtmp.port = this.port = config.rtmp.port ? config.rtmp.port : RTMP_PORT
        this.tcp_server = Net.createServer((socket) =>{
            let session = new rtmp_session(config, socket)
            session.run()
        })
    }

    run(){
        this.tcp_server.listen(this.port, () => {
            console.log("[RTMP] Listen on port " + this.port)
        })

        this.tcp_server.on('error', (e) => {
            console.log("[RTMP] error " + e)
        })

        this.tcp_server.on('close', () => {
            console.log("[RTMP] close")
        })
    }

    stop() {
        this.tcp_server.close();

        context.sessions.forEach((session, id) => {
            if (session instanceof rtmp_session)
                session.stop();
        });
    }
}

module.exports = rtmp_server