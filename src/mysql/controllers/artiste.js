const artiste_mdl = require("../models/artiste")

exports.all_artistes = (res) =>{
    artiste_mdl.get_all_artistes((error, results) => {
        if(error){
            console.log("[MYSQL] error get all artistes" + error)
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
exports.artistes_by_id = (req, res) =>{
    artiste_mdl.get_artiste_by_id(req, (error, results) => {
        if(error){
            console.log("[MYSQL] error get artiste by id" + error)
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
exports.artistes_by_nom = (req, res) =>{
    artiste_mdl.get_artiste_by_nom(req, (error, results) => {
        if(error){
            console.log("[MYSQL] error get artistes by nom" + error)
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
exports.insert_artiste= (req, res) =>{
    artiste_mdl.add_artiste(req, (error, results) => {
        if(error){
            console.log("[MYSQL] error add artiste" + error)
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
exports.delete_artiste = (req, res) =>{
    artiste_mdl.delete_artiste(req, (error, results) => {
        if(error){
            console.log("[MYSQL] error delete artiste" + error)
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
exports.modify_artiste_nom = (req, res) =>{
    const data = {
        id: req.id,
        nom: req.nom
    }
    artiste_mdl.update_artiste_nom(data, (error, results) => {
        if(error){
            console.log("[MYSQL] error update artiste" + error)
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