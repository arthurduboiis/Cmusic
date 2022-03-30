const musique_mdl = require("../models/musique")

exports.all_musics = (res) =>{
    musique_mdl.get_all_musics((error, results) => {
        if(error){
            console.log("[MYSQL] error get all musics" + error)
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

exports.music_by_id = (req, res) =>{
    musique_mdl.get_music_by_id(req, (error, results) =>{
        if(error){
            console.log("[MYSQL] error get musics by id" + error)
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
exports.music_by_nom = (req, res) =>{
    musique_mdl.get_music_by_nom(req, (error, results) =>{
        if(error){
            console.log("[MYSQL] error get musics by nom" + error)
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
exports.music_by_date = (req, res) =>{
    musique_mdl.get_music_by_date(req, (error, results) =>{
        if(error){
            console.log("[MYSQL] error get musics by date" + error)
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
exports.music_by_style = (req, res) =>{
    musique_mdl.get_music_by_style(req, (error, results) =>{
        if(error){
            console.log("[MYSQL] error get musics by style" + error)
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
exports.music_by_tag = (req, res) =>{
    musique_mdl.get_music_by_tag(req, (error, results) =>{
        if(error){
            console.log("[MYSQL] error get musics by tag" + error)
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
exports.music_by_artiste = (req, res) =>{
    musique_mdl.get_music_by_artiste(req, (error, results) =>{
        if(error){
            console.log("[MYSQL] error get musics by artiste" + error)
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
exports.music_by_album = (req, res) =>{
    musique_mdl.get_music_by_album(req, (error, results) =>{
        if(error){
            console.log("[MYSQL] error get musics by album" + error)
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
exports.insert_music = (req, res) =>{
    const data = {
        nom: req.nom,
        duree: req.duree,
        date: req.date,
        style: req.style,
        path: req.path,
        tags: req.tags,
        thumbnail: req.thumbnail,
        idArtiste: req.idArtiste,
        idAlbum: req.idAlbum
    }
    musique_mdl.add_music(data, (error, results) =>{
        if(error){
            console.log("[MYSQL] error add music" + error)
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
exports.delete_music = (req, res) =>{
    musique_mdl.delete_music(req, (error, results) =>{
        if(error){
            console.log("[MYSQL] error delete music" + error)
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

exports.modify_music_nom = (req, res) =>{
    const data = {
        nom: req.nom,
        id: req.id
    }
    musique_mdl.update_music_nom(data, (error, results) =>{
        if(error){
            console.log("[MYSQL] error update music nom" + error)
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
exports.modify_music_date = (req, res) =>{
    const data = {
        date: req.date,
        id: req.id
    }
    musique_mdl.update_music_date(data, (error, results) =>{
        if(error){
            console.log("[MYSQL] error update music date" + error)
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
exports.modify_music_style = (req, res) =>{
    const data = {
        style: req.style,
        id: req.id
    }
    musique_mdl.update_music_style(data, (error, results) =>{
        if(error){
            console.log("[MYSQL] error update music style" + error)
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
exports.modify_music_thumbnail = (req, res) =>{
    const data = {
        thumbnail: req.thumbnail,
        id: req.id
    }
    musique_mdl.update_music_thumbnail(data, (error, results) =>{
        if(error){
            console.log("[MYSQL] error update music thumbnail" + error)
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
exports.modify_music_artiste = (req, res) =>{
    const data = {
        artiste: req.artiste,
        id: req.id
    }
    musique_mdl.update_music_artiste(data, (error, results) =>{
        if(error){
            console.log("[MYSQL] error update music artiste" + error)
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
exports.modify_music_album = (req, res) =>{
    const data = {
        album: req.album,
        id: req.id
    }
    musique_mdl.update_music_album(data, (error, results) =>{
        if(error){
            console.log("[MYSQL] error update music album" + error)
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
exports.add_tag = (req, res) =>{
    const data = {
        tags: req.tags,
        id: req.id
    }
    musique_mdl.add_music_tag(data, (error, results) =>{
        if(error){
            console.log("[MYSQL] error add music tag" + error)
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
exports.delete_all_tags = (req, res) =>{
    musique_mdl.delete_all_music_tags(req, (error, results) =>{
        if(error){
            console.log("[MYSQL] error delete all music tags" + error)
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
exports.replace_tags = (req, res) =>{
    const data ={
        tags: req.tags,
        id: req.id
    }
    musique_mdl.replace_music_tags(data, (error, results) =>{
        if(error){
            console.log("[MYSQL] error replace music tags" + error)
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
exports.delete_tag = (req, res) =>{
    const data = {
        tag: req.tag,
        id: req.id
    }
    musique_mdl.delete_music_tag(data, (error, results) =>{
        if(error){
            console.log("[MYSQL] error delete music tag" + error)
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