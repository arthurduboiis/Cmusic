const musique_ctrl = require("../mysql/controllers/musique")
const artiste_ctrl = require("../mysql/controllers/artiste")
const playlist_ctrl = require("../mysql/controllers/playlist")
const child = require("child_process")
const ffmpeg = require("fluent-ffmpeg")

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
                if(decomp[3] === "-1")
                    decomp[3] = Math.floor((Math.random()*12)+1)
                musique_ctrl.music_by_id(decomp[3], (rep) =>{
                    if (!rep.success) {
                        console.log("[TCP] error start music " + rep.data)
                        this.socket.write("S/INFO/couldnt_start_music")
                    } else {
                        this.socket.write("S/INFO/music_starting")
                        this.launch_FFmpeg(rep.data[0].path, null, decomp[5])
                    }
                })
                break
            case("STOP"):
            case("SEEK"):
                musique_ctrl.music_by_id(decomp[3], (rep) =>{
                    if (!rep.success) {
                        console.log("[TCP] error start music " + rep.data)
                        this.socket.write("S/INFO/couldnt_start_music")
                    } else {
                        this.socket.write("S/INFO/music_seek_starting")
                        this.launch_FFmpeg(rep.data[0].path, decomp[4], decomp[5])
                    }
                })
                break
            case("GET"):
                this.get_data(decomp[3], decomp[4], decomp[5])
                break
            case("POST"):
                this.create_data(decomp[3], decomp[4], decomp[5], decomp[6])
                break
            case("DELETE"):
                this.delete_data(decomp[3], decomp[4], decomp[5], decomp[6])
                break
            case("UPDATE"):
                this.update_data(decomp[3], decomp[4], decomp[5], decomp[6])
                break
            default:
                break
        }
        console.log(decomp)
    }
    error(err){
        console.log("[TCP] error " + err)
    }

    launch_FFmpeg(path, seek, loop){
        let process = ffmpeg("./src/musics/"+path)

        if(seek)
            process.seekInput(seek)
        // if(loop)
        //     process.inputOptions("-stream_loop 3")
        process.noVideo()
            .format("flv")
            .save("rtmp://localhost/live/STREAM_NAME")

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
                this.socket.write("S/ERROR/table_doesnt_exist")
                break

        }
    }
    create_musique(type, id, data){
        switch (parseInt(type)){
            case(0):
                musique_ctrl.insert_music(data, (rep) =>{
                    if (!rep.success) {
                        console.log("[TCP] error create music " + rep.data)
                        this.socket.write("S/ERROR/create_musique")
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
                        this.socket.write("S/ERROR/add_tag")
                    }
                    else{
                        this.socket.write("S/INFO/tag_added")
                    }
                })
                break
            default:
                this.socket.write("S/ERROR/type_doesnt_exist")
                break
        }
    }
    create_artiste(data){
        artiste_ctrl.insert_artiste(data, (rep) =>{
            if (!rep.success) {
                console.log("[TCP] error create artiste " + rep.data)
                this.socket.write("S/ERROR/create_artiste")
            }
            else{
                this.socket.write("S/INFO/artiste_added")
            }
        })
    }
    create_playlist(type, id, data){
        switch (parseInt(type)){
            case(0):
                playlist_ctrl.insert_playlist(data, (rep) =>{
                    if (!rep.success) {
                        console.log("[TCP] error create playlist " + rep.data)
                        this.socket.write("S/ERROR/create_playlist")
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
                        this.socket.write("S/ERROR/add_music_to_playlist")
                    }
                    else{
                        this.socket.write("S/INFO/music_added_to_playlist")
                    }
                })
                break
            default:
                this.socket.write("S/ERROR/type_doesnt_exist")
                break
        }
    }

    get_data(table, type, id){
        switch(table){
            case("musique"):
                this.get_musique(type, id)
                break
            case("playlist"):
                this.get_playlist()
                break
            case("artiste"):
                this.get_artiste()
                break
            default:
                this.socket.write("S/ERROR/table_doesnt_exist")
                break
        }
    }
    get_musique(type, id){

        switch(parseInt(type)){
            case(0):
                musique_ctrl.all_musics((rep) =>{
                    if (!rep.success) {
                        console.log("[TCP] error get all musics " + rep.data)
                        this.socket.write("S/ERROR/get_musics")
                    }
                    else{
                        this.socket.write("S/POST/" + rep.data)
                    }
                })
                break
            case(1):
                musique_ctrl.music_by_id(id, (rep) =>{
                    if (!rep.success) {
                        console.log("[TCP] error get music " + rep.data)
                        this.socket.write("S/ERROR/get_musics")
                    }
                    else{
                        this.socket.write("S/POST/" + rep.data)
                    }
                })
                break
            case(2):
                break
            case(3):
                break
            case(4):
                break
            case(5):
                break
            case(6):
                break
            case(7):
                break
            default:
                this.socket.write("S/ERROR/type_doesnt_exist")
                break
        }
    }
    get_playlist(){

    }
    get_artiste(){

    }

    update_data(table, type, id, data){
        switch(table){
            case("musique"):
                this.update_musique(type, id, data)
                break
            case("playlist"):
                this.update_playlist(id, data)
                break
            case("artiste"):
                this.update_artiste(id, data)
                break
            default:
                this.socket.write("S/ERROR/table_doesnt_exist")
                break
        }
    }
    update_musique(type, id, data){
        switch(parseInt(type)){
            case(0):
                musique_ctrl.modify_music_nom({id: id, nom: data}, (rep) =>{
                    if (!rep.success) {
                        console.log("[TCP] error modify musique nom " + rep.data)
                        this.socket.write("S/ERROR/update_music")
                    }
                    else{
                        this.socket.write("S/INFO/musique_name_updated")
                    }
                })
                break
            case(1):
                musique_ctrl.modify_music_date({id: id, date: data}, (rep) =>{
                    if (!rep.success) {
                        console.log("[TCP] error modify musique date" + rep.data)
                        this.socket.write("S/ERROR/update_music")
                    }
                    else{
                        this.socket.write("S/INFO/musique_name_updated")
                    }
                })
                break
            case(2):
                musique_ctrl.modify_music_style({id: id, style: data}, (rep) =>{
                    if (!rep.success) {
                        console.log("[TCP] error modify musique style " + rep.data)
                        this.socket.write("S/ERROR/update_music")
                    }
                    else{
                        this.socket.write("S/INFO/musique_name_updated")
                    }
                })
                break
            case(3):
                musique_ctrl.modify_music_thumbnail({id: id, thumbnail: data}, (rep) =>{
                    if (!rep.success) {
                        console.log("[TCP] error modify musique thumbnail " + rep.data)
                        this.socket.write("S/ERROR/update_music")
                    }
                    else{
                        this.socket.write("S/INFO/musique_name_updated")
                    }
                })
                break
            case(4):
                musique_ctrl.modify_music_artiste({id: id, artiste: data}, (rep) =>{
                    if (!rep.success) {
                        console.log("[TCP] error modify musique artiste " + rep.data)
                        this.socket.write("S/ERROR/update_music")
                    }
                    else{
                        this.socket.write("S/INFO/musique_name_updated")
                    }
                })
                break
            case(5):
                musique_ctrl.modify_music_album({id: id, album: data}, (rep) =>{
                    if (!rep.success) {
                        console.log("[TCP] error modify musique album " + rep.data)
                        this.socket.write("S/ERROR/update_music")
                    }
                    else{
                        this.socket.write("S/INFO/musique_name_updated")
                    }
                })
                break
            case(6):
                musique_ctrl.replace_tags({id: id, tags: data}, (rep) =>{
                    if (!rep.success) {
                        console.log("[TCP] error modify musique nom " + rep.data)
                        this.socket.write("S/ERROR/update_music_tags")
                    }
                    else{
                        this.socket.write("S/INFO/musique_name_updated")
                    }
                })
                break
            default:
                this.socket.write("S/ERROR/type_doesnt_exist")
                break
        }
    }
    update_playlist(id, data){
        playlist_ctrl.modify_playlist_nom({id: id, nom: data}, (rep) =>{
            if (!rep.success) {
                console.log("[TCP] error modify playlist nom " + rep.data)
                this.socket.write("S/ERROR/update_playlist")
            }
            else{
                this.socket.write("S/INFO/playlist_name_updated")
            }
        })
    }
    update_artiste(id, data){
        artiste_ctrl.modify_artiste_nom({id: id, nom: data}, (rep) =>{
            if (!rep.success) {
                console.log("[TCP] error modify artiste nom " + rep.data)
                this.socket.write("S/ERROR/update_artiste")
            }
            else{
                this.socket.write("S/INFO/artiste_name_updated")
            }
        })
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
                this.socket.write("S/ERROR/table_doesnt_exist")
                break
        }
    }
    delete_musique(type, id, data){
        switch(parseInt(type)){
            case(0):
                musique_ctrl.delete_music(id, (rep) =>{
                    if (!rep.success) {
                        console.log("[TCP] error delete music " + rep.data)
                        this.socket.write("S/ERROR/delete_musique")
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
                        this.socket.write("S/ERROR/delete_tag")
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
                        this.socket.write("S/ERROR/delete_all_tag")
                    }
                    else{
                        this.socket.write("S/INFO/all_tags_deleted")
                    }
                })
                break
            default:
                this.socket.write("S/ERROR/type_doesnt_exist")
        }
    }
    delete_playlist(type, id, data){
        switch(parseInt(type)){
            case(0):
                playlist_ctrl.delete_playlist(id, (rep) =>{
                    if (!rep.success) {
                        console.log("[TCP] error delete playlist " + rep.data)
                        this.socket.write("S/ERROR/delete_playlist")
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
                        this.socket.write("S/ERROR/delete_music_in_playlist")
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
                        this.socket.write("S/ERROR/delete_all_music_in_playlist")
                    }
                    else{
                        this.socket.write("S/INFO/all_musiques_in_playlist_deleted")
                    }
                })
                break
            default:
                this.socket.write("S/ERROR/type_doesnt_exist")
        }
    }
    delete_artiste(id){
        artiste_ctrl.delete_artiste(id, (rep) =>{
            if (!rep.success) {
                console.log("[TCP] error delete artiste " + rep.data)
                this.socket.write("S/ERROR/delete_artiste")
            }
            else{
                this.socket.write("S/INFO/artiste_deleted")
            }
        })
    }
}


module.exports = tcp_session