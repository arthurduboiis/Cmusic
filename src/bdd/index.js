const mysql = require('mysql');
const express = require('express');
var app = express();
const path=require('path');

var con = mysql.createConnection({
  host: "localhost",
  user: "root",
  password: "",
  database: "base_ptut"
});

con.connect(function(err) {
  if (err) throw err;
  console.log("Connected!");
});

app.listen(8080,()=>{
    console.log('Serveur demarre au port :8080');
});

function insertArtists(nomartiste) {
  
    // Query to insert multiple rows
    var query = "INSERT INTO artiste(nom_artiste) VALUES ?;";
  
    // Values to be inserted
    let values = ['' + nomartiste + ''];
  
    // Executing the query
    con.query(query, [values], (err, rows) => {
        if (err) throw err;
        console.log("All Artists Inserted");
    });
};

function insertMusics(music_name, ajout, duree) {
  
    // Query to insert multiple rows
    var query = "INSERT INTO musique(nom_music,date_ajout,durée) VALUES ?;";
  
    // Values to be inserted
    let values = ['' + music_name + '','' + ajout + '','' + duree + ''];
  
    // Executing the query
    con.query(query, [values], (err, rows) => {
        if (err) throw err;
        console.log("All Musics Inserted");
    });
};

function insertArtistMusic(idmusic, idartist) {
  
    // Query to insert multiple rows
    var query = "INSERT INTO creer(music_id, artiste_id) VALUES ?;";
  
    // Values to be inserted
    let values = [idmusic, idartist];
  
    // Executing the query
    con.query(query, [values], (err, rows) => {
        if (err) throw err;
        console.log("All Connections between Musics and Artists Inserted");
    });
};

function insertTags(tagname) {
  
    // Query to insert multiple rows
    var query = "INSERT INTO tag(nom_tag) VALUES ?;";
  
    // Values to be inserted
    let values = ['' + tagname + ''];
  
    // Executing the query
    con.query(query, [values], (err, rows) => {
        if (err) throw err;
        console.log("All Tags Inserted");
    });
};

function insertTagMusic(idmusic, idtag) {
  
    // Query to insert multiple rows
    var query = "INSERT INTO appartient(music_id, tag_id) VALUES ?;";
  
    // Values to be inserted
    let values = [idmusic, idtag];
  
    // Executing the query
    con.query(query, [values], (err, rows) => {
        if (err) throw err;
        console.log("All Connections between Musics and Tags Inserted");
    });
};

function insertPlaylists(playlistname, creation) {
  
    // Query to insert multiple rows
    var query = "INSERT INTO playlist(nom_playlist, date_creation) VALUES ?;";
  
    // Values to be inserted
    let values = ['' + playlistname + '', '' + creation + ''];
  
    // Executing the query
    con.query(query, [values], (err, rows) => {
        if (err) throw err;
        console.log("All PLaylists Inserted");
    });
};

function insertPlaylistMusic(idmusic, idplaylist) {
  
    // Query to insert multiple rows
    var query = "INSERT INTO possede(music_id, tag_id) VALUES ?;";
  
    // Values to be inserted
    let values = [idmusic, idplaylist];
  
    // Executing the query
    con.query(query, [values], (err, rows) => {
        if (err) throw err;
        console.log("All Connections between Musics and Playlists Inserted");
    });
};

// Requête pour afficher toutes les musiques

function musicsList() {

  con.connect(function(err) {
  	if (err) throw err;
  	con.query("SELECT * FROM musique", function (err, result, fields) {
      if (err) throw err;
      console.log(result);
  	});
  });

}

// Requête pour afficher un tag et son contenu

function contenuTag(idtag) {

  con.connect(function(err) {
  	if (err) throw err;
  	con.query("SELECT t.nom_tag, m.nom_music " +
				  "FROM appartient " +
				  "INNER JOIN tag as t ON appartient.tag_id = t.tag_id " +
				  "INNER JOIN musique as m ON appartient.music_id = m.music_id " +
				  "WHERE t.tag_id = "idtag";", function (err, result, fields) {
    	if (err) throw err;
      console.log(result);
  	});
  });

}

// Requête pour afficher une playlist et son contenu

function contenuPLaylist(idplaylist) {

  con.connect(function(err) {
  	if (err) throw err;
  	con.query("SELECT p.nom_playlist, m.nom_music " +
				  "FROM possede " +
				  "INNER JOIN playlist as p ON possede.playlist_id = p.playlist_id " +
				  "INNER JOIN musique as m ON possede.music_id = m.music_id " +
				  "WHERE p.playlist_id = "idplaylist";", function (err, result, fields) {
      if (err) throw err;
      console.log(result);
  	});
  });
  
}
