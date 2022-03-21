const server = require("./src/server")

const config = {
    rtmp: {
        port: 1935,
        chunk_size: 60000,
        ping: 30,
        ping_timeout: 60
    }
};
let s = new server(config)
s.run()