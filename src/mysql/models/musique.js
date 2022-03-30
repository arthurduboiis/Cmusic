const db = require("../config/mysql")

exports.get_all_musics = (callback) =>{
    db.query("SELECT * FROM musique;", (error, results) => {
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.get_music_by_id = (data, callback) =>{
    db.query("SELECT * FROM musique WHERE musique.id = ?;", [data], (error, results) => {
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.get_music_by_nom = (data, callback) =>{
    db.query("SELECT * FROM musique WHERE musique.nom LIKE ?;", ['%' +data+ '%'], (error, results) => {
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.get_music_by_date = (data, callback) =>{
    db.query("SELECT * FROM musique WHERE musique.date = ?;", [data], (error, results) => {
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.get_music_by_style = (data, callback) =>{
    db.query("SELECT * FROM musique WHERE musique.style = ?;", [data], (error, results) => {
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.get_music_by_tag = (data, callback) =>{
    db.query("SELECT * FROM musique WHERE ? MEMBER OF(musique.tags);", [data], (error, results) =>{
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.get_music_by_artiste = (data, callback) =>{
    db.query("SELECT * FROM musique JOIN artiste ON artiste.id = musique.idArtiste WHERE artiste.nom LIKE ?;", ['%' +data+ '%'], (error, results) =>{
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.get_music_by_album = (data, callback) =>{
    db.query("SELECT * FROM musique JOIN album ON album.id = musique.idAlbum WHERE album.nom LIKE ?;", ['%' +data+ '%'], (error, results) =>{
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.add_music = (data, callback) =>{
    db.query("INSERT INTO musique (nom, duree, date, style, path, tags, thumbnail, idArtiste, idAlbum) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?);", [data.nom, data.duree, data.date, data.style, data.path, data.tags, data.thumbnail, data.idArtiste, data.idAlbum], (error, results) =>{
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.delete_music = (data, callback) =>{
    db.query("DELETE FROM musique WHERE id = ?;", [data], (error, results) =>{
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.update_music_nom = (data, callback) => {
    db.query("UPDATE musique SET nom = ? WHERE id = ?;", [data.nom, data.id], (error, results) =>{
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.update_music_date = (data, callback) => {
    db.query("UPDATE musique SET date = ? WHERE id = ?;", [data.date, data.id], (error, results) =>{
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.update_music_style = (data, callback) => {
    db.query("UPDATE musique SET style = ? WHERE id = ?;", [data.style, data.id], (error, results) =>{
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.update_music_thumbnail = (data, callback) => {
    db.query("UPDATE musique SET thumbnail = ? WHERE id = ?;", [data.thumbnail, data.id], (error, results) =>{
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.update_music_artiste = (data, callback) => {
    db.query("UPDATE musique SET idArtiste = ? WHERE id = ?;", [data.idArtiste, data.id], (error, results) =>{
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.update_music_album = (data, callback) => {
    db.query("UPDATE musique SET idAlbum = ? WHERE id = ?;", [data.idAlbum, data.id], (error, results) =>{
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.add_music_tag = (data, callback) => {
    db.query("UPDATE musique SET tags = JSON_MERGE_PRESERVE(tags, ?) WHERE id = ?;", [data.tags, data.id],(error, results) =>{
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.delete_music_tag = (data, callback) => {
    db.query("UPDATE musique SET tags = JSON_REMOVE(tags, JSON_SEARCH(tags, 'one', ?)) WHERE id = ?;", [data.tag, data.id],(error, results) =>{
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.delete_all_music_tags = (data, callback) => {
    db.query("UPDATE musique SET tags = NULL WHERE id = ?;", [data], (error, results) =>{
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.replace_music_tags = (data, callback) =>{
    db.query("UPDATE musique SET tags = ? WHERE id = ?;", [data.tags, data.id], (error, results) =>{
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}