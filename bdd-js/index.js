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

let InsertArtists = () => {
  
    // Query to insert multiple rows
    let query = `INSERT INTO artiste 
        (nom_artiste) VALUES ?;`;
  
    // Values to be inserted
    let values = [
        ['The Weekend'],
        ['Drake'],
        ['DJ Snake'],
        ['Damso'],
        ['Travis Scott']
    ];
  
    // Executing the query
    db_con.query(query, [values], (err, rows) => {
        if (err) throw err;
        console.log("All Artists Inserted");
    });
};

let InsertMusics = () => {
  
    // Query to insert multiple rows
    let query = `INSERT INTO musique 
        (nom_music,date_ajout,durée) VALUES ?;`;
  
    // Values to be inserted
    let values = [
        ['God\'s Plan','05-01-2022','3:18'],
        ['Blinding Light','01-11-2021','3:22'],
        ['Save Your Tears','21-12-2021','3:35'],
        ['Propaganda','06-02-2022','3:33'],
        ['Magenta Riddim','26-10-2021','3:40'],
        ['Trust Nobody','29-09-2021','3:33'],
        ['911','29-09-2021','2:52'],
        ['Antidote','19-09-2021','4:22'],
        ['Escape Plan','01-03-2022','2:29']
    ];
  
    // Executing the query
    db_con.query(query, [values], (err, rows) => {
        if (err) throw err;
        console.log("All Musics Inserted");
    });
};

let InsertArtistMusic = () => {
  
    // Query to insert multiple rows
    let query = `INSERT INTO creer 
        (music_id, artiste_id) VALUES ?;`;
  
    // Values to be inserted
    let values = [
        [1, 2],
        [2, 1],
        [3, 1],
        [4, 3],
        [5, 3],
        [6, 3],
        [7, 4],
        [8, 5],
        [9, 5]
    ];
  
    // Executing the query
    db_con.query(query, [values], (err, rows) => {
        if (err) throw err;
        console.log("All Connections between Musics and Artists Inserted");
    });
};

let InsertTags = () => {
  
    // Query to insert multiple rows
    let query = `INSERT INTO tag 
        (nom_tag) VALUES ?;`;
  
    // Values to be inserted
    let values = [
        ['House'],
        ['Rap']
    ];
  
    // Executing the query
    db_con.query(query, [values], (err, rows) => {
        if (err) throw err;
        console.log("All Tags Inserted");
    });
};

let InsertTagMusic = () => {
  
    // Query to insert multiple rows
    let query = `INSERT INTO appartient 
        (music_id, tag_id) VALUES ?;`;
  
    // Values to be inserted
    let values = [
        [4, 1],
        [5, 1],
        [6, 1],
        [1, 2],
        [7, 2],
        [8, 2],
        [9, 2]
    ];
  
    // Executing the query
    db_con.query(query, [values], (err, rows) => {
        if (err) throw err;
        console.log("All Connections between Musics and Tags Inserted");
    });
};

let InsertPlaylists = () => {
  
    // Query to insert multiple rows
    let query = `INSERT INTO playlist 
        (nom_playlist, date_creation) VALUES ?;`;
  
    // Values to be inserted
    let values = [
        ['Américain','01-11-2021'],
        ['Francais','29-09-2021']
    ];
  
    // Executing the query
    db_con.query(query, [values], (err, rows) => {
        if (err) throw err;
        console.log("All PLaylists Inserted");
    });
};

let InsertPlaylistMusic = () => {
  
    // Query to insert multiple rows
    let query = `INSERT INTO possede 
        (music_id, tag_id) VALUES ?;`;
  
    // Values to be inserted
    let values = [
        [1, 1],
        [2, 1],
        [3, 1],
        [4, 2],
        [5, 2],
        [6, 2],
        [7, 2],
        [8, 1],
        [9, 1]
    ];
  
    // Executing the query
    db_con.query(query, [values], (err, rows) => {
        if (err) throw err;
        console.log("All Connections between Musics and Playlists Inserted");
    });
};

// Requête pour afficher toutes les musiques

con.connect(function(err) {
  if (err) throw err;
  con.query("SELECT * FROM musique", function (err, result, fields) {
    if (err) throw err;
    console.log(result);
  });
});

// Requête pour afficher un tag et son contenu

con.connect(function(err) {
  if (err) throw err;
  con.query("SELECT t.nom_tag, m.nom_music " +
				"FROM appartient " +
				"INNER JOIN tag as t ON appartient.tag_id = t.tag_id " +
				"INNER JOIN musique as m ON appartient.music_id = m.music_id " +
				"WHERE t.tag_id = "2";", function (err, result, fields) {
    if (err) throw err;
    console.log(result);
  });
});

// Requête pour afficher une playlist et son contenu

con.connect(function(err) {
  if (err) throw err;
  con.query("SELECT p.nom_playlist, m.nom_music " +
				"FROM possede " +
				"INNER JOIN playlist as p ON possede.playlist_id = p.playlist_id " +
				"INNER JOIN musique as m ON possede.music_id = m.music_id " +
				"WHERE p.playlist_id = "1";", function (err, result, fields) {
    if (err) throw err;
    console.log(result);
  });
});
