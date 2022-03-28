const db = require("../config/mysql")

exports.get_all_artistes = (callback) =>{
    db.query("SELECT * FROM artiste;", (error, results) => {
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.get_artiste_by_id = (data, callback) =>{
    db.query("SELECT * FROM artiste WHERE id = ?;", [data], (error, results) => {
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.get_artiste_by_nom = (data, callback) =>{
    db.query("SELECT * FROM artiste WHERE nom LIKE ?;", ['%' +data+ '%'], (error, results) => {
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.add_artiste = (data, callback) =>{
    db.query("INSERT INTO artiste (nom) VALUES (?);", [data], (error, results) => {
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.delete_artiste = (data, callback) =>{
    db.query("DELETE FROM artiste WHERE id = ?;", [data], (error, results) => {
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}
exports.update_artiste_nom = (data, callback) =>{
    db.query("UPDATE artiste SET nom = ? WHERE id = ?;", [data.nom, data.id], (error, results) => {
        if(error){
            return callback(error)
        }
        return callback(null, results)
    })
}