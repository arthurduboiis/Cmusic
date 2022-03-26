const rtmp_server = require("./rtmp/rtmp_server")
const tcp_server = require("./tcp/tcp_server")
const context = require("./core/core_ctx")

class server{
    constructor(config){
        this.config = config
    }

    run(){
        if(this.config.rtmp.port){
            this.rtmp_s = new rtmp_server(this.config)
            this.rtmp_s.run()
        }
        if(this.config.tcp.port){
            this.tcp_s = new tcp_server(this.config)
            this.tcp_s.run()
        }
        process.on('uncaughtException', err => {
                console.log("[SERVER] UncaughtException : " + err)
            }
        )
        process.on('SIGINT', () => {
            process.exit()
        })
    }
    on(eventName, listener) {
        context.nodeEvent.on(eventName, listener);
    }
    stop() {
        if (this.rtmp_s) {
            this.rtmp_s.stop()
        }
        if(this.tcp_s){
            this.tcp_s.stop()
        }
    }
}

module.exports = server