const db = require("../config/mysql")

exports.get_all_playlists = (callback) =>{
    db.query("SELECT * FROM playlist;", (error, results) => {
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.get_playlist_by_id = (data, callback) =>{
    db.query("SELECT *  FROM playlist WHERE id = ?;", [data], (error, results) => {
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.get_playlist_by_nom = (data, callback) =>{
    db.query("SELECT * FROM playlist WHERE nom LIKE ?", ['%' + data + '%'], (error, results) => {
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.get_playlist_by_musique = (data, callback) =>{
    db.query("SELECT * FROM playlist WHERE ? MEMBER OF(playlist.idMusiques);", [data], (error, results) => {
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.add_playlist = (data, callback) =>{
    db.query("INSERT INTO playlist (nom, idMusiques) VALUES(?, ?);", [data.nom, data.idMusiques],(error, results) => {
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.delete_playlist = (data, callback) =>{
    db.query("DELETE playlist WHERE id = ?;", [data], (error, results) => {
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.update_playlist_nom = (data, callback) =>{
    db.query("UPDATE playlist SET nom = ? WHERE id = ?;", [data.nom, data.id], (error, results) => {
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.add_musics_to_playlist = (data, callback) => {
    db.query("UPDATE playlist SET idMusiques = JSON_MERGE_PATCH(idMusiques, ?) WHERE id = ?;", [data.idMusiques, data.id],(error, results) =>{
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.delete_music_in_playlist = (data, callback) => {
    db.query("UPDATE playlist SET idMusiques = JSON_REMOVE(idMusiques, JSON_SEARCH(idMusiques, 'one', ?)) WHERE id = ?;", [data.idMusiques, data.id],(error, results) =>{
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.delete_all_musics_in_playlist = (data, callback) => {
    db.query("UPDATE playlist SET idMusiques = NULL WHERE id = ?;", [data], (error, results) =>{
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}