const musique_ctrl = require("../mysql/controllers/musique")
const artiste_ctrl = require("../mysql/controllers/artiste")
const playlist_ctrl = require("../mysql/controllers/playlist")
const child = require("child_process")
const ffmpeg = require("ffmpeg")

class tcp_session{
    constructor(config, socket){
        this.config = config
        this.socket = socket
        this.ip = socket.remoteAddress
        this.tag = 'tcp'
    }

    run(){
        this.socket.on('data', this.onSocketData.bind(this))
    }

    onSocketData(data){     //la plupart des fonctions qui sont appellé par cette fonction ont été ecrite à 3-4h du mat
        let decomp = data.toString().split('/')
        if(decomp[0] !== 'C' || decomp[1] !== 'TCP') {
            this.error("wrong request")
        }
        switch(decomp[2]) {
            case("START"):
                musique_ctrl.music_by_id(decomp[3], (rep) =>{
                    if (!rep.success) {
                        console.log("[TCP] error start music " + rep.data)
                    } else {
                        this.socket.write("S/INFO/music_starting")
                        this.launch_FFmpeg(rep.data[0].path)
                    }
                })
                break
            case("STOP"):
            case("SEEK"):
            case("GET"):
            case("POST"):
                this.create_data(decomp[3], decomp[4], decomp[5])
                break
            case("DELETE"):
                this.delete_data(decomp[3], decomp[4], decomp[5], decomp[6])
                break
            case("UPDATE"):
                this.delete_data(decomp[3], decomp[4], decomp[5], decomp[6])
            default:
                break
        }
        this.socket.write("donnée reçus")
        console.log(decomp)
    }
    error(err){
        console.log("[TCP] error " + err)
    }

    launch_FFmpeg(path){
        let process = new ffmpeg("./src/musics/"+path)
        process.then((audio) => {
            audio.setDisableVideo()
            audio.addCommand("-f", "flv")
            audio.save("rtmp://localhost/live/STREAM_NAME")
        })
    }

    create_data(table, type, id, data){
        switch(table){
            case("musique"):
                this.create_musique(type, id, data)
                break
            case("artiste"):
                this.create_artiste(data)
                break
            case("playlist"):
                this.create_playlist(type, id, data)
                break
            default:
                console.log("table doesn't exist")

        }
    }
    create_musique(type, id, data){
        switch (type){
            case(0):
                musique_ctrl.insert_music(data, (rep) =>{
                    if (!rep.success) {
                        console.log("[TCP] error create music " + rep.data)
                    }
                    else{
                        this.socket.write("S/INFO/music_created")
                    }
                })
                break
            case(1):
                musique_ctrl.add_tag({tags: data, id: id}, (rep) =>{
                    if (!rep.success) {
                        console.log("[TCP] error add tag to music " + rep.data)
                    }
                    else{
                        this.socket.write("S/INFO/tag_added")
                    }
                })
        }
    }
    create_artiste(data){
        artiste_ctrl.insert_artiste(data, (rep) =>{
            if (!rep.success) {
                console.log("[TCP] error create artiste " + rep.data)
            }
            else{
                this.socket.write("S/INFO/artiste_added")
            }
        })
    }
    create_playlist(type, id, data){
        switch (type){
            case(0):
                playlist_ctrl.insert_playlist(data, (rep) =>{
                    if (!rep.success) {
                        console.log("[TCP] error create playlist " + rep.data)
                    }
                    else{
                        this.socket.write("S/INFO/playlist_added")
                    }
                })
                break
            case(1):
                playlist_ctrl.insert_musics_in_playlist({idMusiques: data, id: id}, (rep) =>{
                    if (!rep.success) {
                        console.log("[TCP] error add music to playlist " + rep.data)
                    }
                    else{
                        this.socket.write("S/INFO/music_added_to_playlist")
                    }
                })
        }
    }

    update_data(table, type, id, data){     //TODO TU T'ES ARRETÉ LÀ TANGUY
        switch(table){
            case("musique"):
                break
            case("playlist"):
                break
            case("artiste"):
                break
            default:
                console.log("table doesn't exist")
        }
    }

    delete_data(table, type, id, data){
        switch (table) {
            case("musique"):
                this.delete_musique(type, id, data)
                break
            case("playlist"):
                this.delete_playlist(type, id)
                break
            case("artiste"):
                this.delete_artiste(id)
                break
            case("album"):
                break
            default:
                console.log("table doesn't exist")
                break
        }
    }
    delete_musique(type, id, data){
        switch(type){
            case(0):
                musique_ctrl.delete_music(id, (rep) =>{
                    if (!rep.success) {
                        console.log("[TCP] error delete music " + rep.data)
                    }
                    else{
                        this.socket.write("S/INFO/musique_deleted")
                    }
                })
                break
            case(1):
                musique_ctrl.delete_tag({id: id, tags: data}, (rep) =>{
                    if (!rep.success) {
                        console.log("[TCP] error delete music tag " + rep.data)
                    }
                    else{
                        this.socket.write("S/INFO/tag_deleted")
                    }
                })
                break
            case(2):
                musique_ctrl.delete_all_tags(id, (rep) =>{
                    if (!rep.success) {
                        console.log("[TCP] error delete all music tag " + rep.data)
                    }
                    else{
                        this.socket.write("S/INFO/all_tags_deleted")
                    }
                })
                break
            default:
                console.log("type doesn't exist")
        }
    }
    delete_playlist(type, id, data){
        switch(type){
            case(0):
                playlist_ctrl.delete_playlist(id, (rep) =>{
                    if (!rep.success) {
                        console.log("[TCP] error delete playlist " + rep.data)
                    }
                    else{
                        this.socket.write("S/INFO/playlist_deleted")
                    }
                })
                break
            case(1):
                playlist_ctrl.delete_music_in_playlist({id: id, idMusiques: data}, (rep) =>{
                    if (!rep.success) {
                        console.log("[TCP] error delete playlist music " + rep.data)
                    }
                    else{
                        this.socket.write("S/INFO/musiques_in_playlist_deleted")
                    }
                })
                break
            case(2):
                playlist_ctrl.delete_all_musics_in_playlist(id, (rep) =>{
                    if (!rep.success) {
                        console.log("[TCP] error delete all playlist music " + rep.data)
                    }
                    else{
                        this.socket.write("S/INFO/all_musiques_in_playlist_deleted")
                    }
                })
                break
            default:
                console.log("type doesn't exist")
        }
    }
    delete_artiste(id){
        artiste_ctrl.delete_artiste(id, (rep) =>{
            if (!rep.success) {
                console.log("[TCP] error delete artiste " + rep.data)
            }
            else{
                this.socket.write("S/INFO/artiste_deleted")
            }
        })
    }
}


module.exports = tcp_session