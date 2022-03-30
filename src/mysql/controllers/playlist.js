const playlist_mdl = require("../models/playlist")

exports.all_playlists = (res) =>{
    playlist_mdl.get_all_playlists((error, results) => {
        if(error){
            console.log("[MYSQL] error get all playlists" + error)
                        return res ({
                success: 0,
                data: error
            })
        }
        return res({
            success: 1,
            data: results
        })
    })
}
exports.playlist_by_id = (req, res) =>{
    playlist_mdl.get_playlist_by_id(req, (error, results) => {
        if(error){
            console.log("[MYSQL] error get playlist by id" + error)
                        return res ({
                success: 0,
                data: error
            })
        }
        return res({
            success: 1,
            data: results
        })
    })
}
exports.playlist_by_nom = (req, res) =>{
    playlist_mdl.get_playlist_by_nom(req, (error, results) => {
        if(error){
            console.log("[MYSQL] error get playlists by nom" + error)
                        return res ({
                success: 0,
                data: error
            })
        }
        return res({
            success: 1,
            data: results
        })
    })
}
exports.playlist_by_musique = (req, res) =>{
    playlist_mdl.get_playlist_by_musique(req, (error, results) => {
        if(error){
            console.log("[MYSQL] error get playlist by musique" + error)
                        return res ({
                success: 0,
                data: error
            })
        }
        return res({
            success: 1,
            data: results
        })
    })
}
exports.insert_playlist = (req, res) =>{
    const data = {
        nom: req.nom,
        idMusiques: req.idMusiques
    }
    playlist_mdl.add_playlist(data,(error, results) => {
        if(error){
            console.log("[MYSQL] error add playlist" + error)
                        return res ({
                success: 0,
                data: error
            })
        }
        return res({
            success: 1,
            data: results
        })
    })
}
exports.delete_playlist = (req, res) =>{
    playlist_mdl.delete_playlist(req, (error, results) => {
        if(error){
            console.log("[MYSQL] error delete playlist" + error)
                        return res ({
                success: 0,
                data: error
            })
        }
        return res({
            success: 1,
            data: results
        })
    })
}
exports.modify_playlist_nom = (req, res) =>{
    const data = {
        nom: req.nom,
        id: req.id
    }
    playlist_mdl.update_playlist_nom(data, (error, results) => {
        if(error){
            console.log("[MYSQL] error update playlist" + error)
                        return res ({
                success: 0,
                data: error
            })
        }
        return res({
            success: 1,
            data: results
        })
    })
}
exports.insert_musics_in_playlist = (req, res) =>{
    const data = {
        idMusiques: req.idMusiques,
        id: req.id
    }
    playlist_mdl.add_musics_to_playlist(data,(error, results) => {
        if(error){
            console.log("[MYSQL] error add musics to playlist" + error)
                        return res ({
                success: 0,
                data: error
            })
        }
        return res({
            success: 1,
            data: results
        })
    })
}
exports.delete_music_in_playlist = (req, res) =>{
    const data = {
        idMusiques: req.idMusiques,
        id: req.id
    }
    playlist_mdl.delete_music_in_playlist(data, (error, results) => {
        if(error){
            console.log("[MYSQL] error delete musics in playlist" + error)
                        return res ({
                success: 0,
                data: error
            })
        }
        return res({
            success: 1,
            data: results
        })
    })
}
exports.delete_all_musics_in_playlist = (req, res) =>{
    playlist_mdl.delete_all_musics_in_playlist(req, (error, results) => {
        if(error){
            console.log("[MYSQL] error delete all musics" + error)
                        return res ({
                success: 0,
                data: error
            })
        }
        return res({
            success: 1,
            data: results
        })
    })
}