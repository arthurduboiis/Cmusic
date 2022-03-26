DROP TABLE IF EXISTS artiste, musique, playlist, tag;

CREATE TABLE `artiste` (
  `artiste_id` int PRIMARY KEY AUTO_INCREMENT,
  `nom_artiste` varchar(255)
);

CREATE TABLE `musique` (
  `music_id` int PRIMARY KEY AUTO_INCREMENT,
  `nom_music` varchar(255),
  `date_ajout` date,
  `durée` time
);

CREATE TABLE `playlist` (
  `playlist_id` int PRIMARY KEY AUTO_INCREMENT,
  `nom_playlist` varchar(255),
  `date_creation` date
);

CREATE TABLE `possede` (
  `music_id` int,
  `playlist_id` int
);

CREATE TABLE `tag` (
  `tag_id` int PRIMARY KEY AUTO_INCREMENT,
  `nom_tag` varchar(255)
);

CREATE TABLE `appartient` (
  `music_id` int,
  `tag_id` int
);

CREATE TABLE `creer` (
  `music_id` int,
  `artiste_id` int
);

ALTER TABLE `possede` ADD FOREIGN KEY (`music_id`) REFERENCES `musique` (`music_id`);

ALTER TABLE `possede` ADD FOREIGN KEY (`playlist_id`) REFERENCES `playlist` (`playlist_id`);

ALTER TABLE `appartient` ADD FOREIGN KEY (`music_id`) REFERENCES `musique` (`music_id`);

ALTER TABLE `appartient` ADD FOREIGN KEY (`tag_id`) REFERENCES `tag` (`tag_id`);

ALTER TABLE `creer` ADD FOREIGN KEY (`music_id`) REFERENCES `musique` (`music_id`);

ALTER TABLE `creer` ADD FOREIGN KEY (`artiste_id`) REFERENCES `artiste` (`artiste_id`);

ALTER TABLE `tag` ADD FOREIGN KEY (`nom_tag`) REFERENCES `tag` (`tag_id`);


#INSERT INTO à completer plus tard 
INSERT INTO musique (nom_music,date_ajout,durée) values (?,?,?);

#Requête affichage des musiques:
SELECT * FROM musique;

#Requête affichage d'une playlist et de son contenu:
SELECT p.nom_playlist, m.nom_music
FROM possede
INNER JOIN playlist as p ON possede.playlist_id = p.playlist_id
INNER JOIN musique as m ON possede.music_id = m.music_id
WHERE p.playlist_id = "2";

#Requête affichage d'un tag et de son contenu:
SELECT t.nom_tag, m.nom_music
FROM appartient
INNER JOIN tag as t ON appartient.tag_id = t.tag_id
INNER JOIN musique as m ON appartient.music_id = m.music_id
WHERE t.tag_id = "3";



CREATE DATABASE IF NOT EXISTS bdd_CMusic;
USE bdd_CMusic;

DROP TABLE IF EXISTS artiste;
DROP TABLE IF EXISTS musique;
DROP TABLE IF EXISTS playlist;
DROP TABLE IF EXISTS tag;


CREATE TABLE artiste (
                         id_Artiste INT AUTO_INCREMENT PRIMARY KEY NOT NULL
                        ,nom_artiste VARCHAR(100)
                        ,prenom_artiste VARCHAR(100)
                        ,sexe_artiste VARCHAR(10)
                        ,date_naissance DATE
                        ,id_Musique             INT
                        ,FOREIGN KEY (id_Musique) REFERENCES musique(id_Musique)
);



CREATE TABLE tag (
                        id_tag             INT AUTO_INCREMENT PRIMARY KEY NOT NULL
                        ,CategorieTag       VARCHAR(150)

);

CREATE TABLE playlist (
                          id_Playlist          INT AUTO_INCREMENT PRIMARY KEY NOT NULL
    ,NomPlaylist          VARCHAR(200)
    ,CategoriePlaylist     VARCHAR(200)
    ,id_Tag            INT
    ,FOREIGN KEY (id_tag) REFERENCES tag(id_tag)

);

CREATE TABLE musique (
                        id_Musique            INT AUTO_INCREMENT PRIMARY KEY NOT NULL
                        ,titreMusique          VARCHAR(200)
                        ,DureeMusique          TIME
                        ,DateMusique           DATE
                        ,id_tag                INT
                        ,id_Artiste             INT
                        ,id_Playlist             INT
                        ,FOREIGN KEY (id_tag) REFERENCES tag(id_tag)
                        ,FOREIGN KEY (id_Artiste) REFERENCES artiste(id_Artiste)
                        ,FOREIGN KEY (id_Playlist) REFERENCES playlist(id_Playlist)
);


INSERT INTO musique (titreMusique,DureeMusique,DateMusique) values (?,?,?,?);

#Requête affichage des musiques:
SELECT * FROM musique;



INSERT INTO playlist (url, etat) VALUES (?, ?, ?);













