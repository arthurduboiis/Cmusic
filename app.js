const server = require("./src/server")
const dotenv = require("dotenv")

dotenv.config()

const config = {
    rtmp: {
        port: 1935,
        chunk_size: 60000,
        ping: 30,
        ping_timeout: 60
    },
    tcp: {
        port: 8080,
    },
};
let s = new server(config)
s.run()