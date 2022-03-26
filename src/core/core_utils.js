const context = require("./core_ctx")

function generateNewSessionID() {
    let sessionID = ''
    const possible = 'ABCDEFGHIJKLMNOPQRSTUVWKYZ0123456789'
    const numPossible = possible.length
    do {
        for (let i = 0; i < 8; i++) {
            sessionID += possible.charAt((Math.random() * numPossible) | 0)
        }
    } while (context.sessions.has(sessionID))
    return sessionID
}

module.exports = {
    generateNewSessionID,
}
