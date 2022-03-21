const rtmp_server = require("./rtmp_server")
const context = require("./core_ctx")

class server{
    constructor(config){

        this.config = config
    }

    run(){
        if(this.config.rtmp.port){
            this.rtmp_s = new rtmp_server(this.config)
            this.rtmp_s.run()
        }
        process.on('uncaughtException', err => {
                console.log("UncaughtException : " + err)
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
            this.rtmp_s.stop();
        }
    }
}

module.exports = server