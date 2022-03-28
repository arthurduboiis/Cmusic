CREATE DATABASE IF NOT EXISTS cmusic;
USE cmusic;

DROP TABLE IF EXISTS musique;
DROP TABLE IF EXISTS artiste;
DROP TABLE IF EXISTS album;
DROP TABLE IF EXISTS playlist;

CREATE TABLE artiste(
    id INT AUTO_INCREMENT,
    nom VARCHAR(256),
    PRIMARY KEY(id)
);

CREATE TABLE album(
    id INT AUTO_INCREMENT,
    nom VARCHAR(256),
    date DATE,
    thumbnail VARCHAR(1024),
    PRIMARY KEY(id)
);

CREATE TABLE playlist(
    id INT AUTO_INCREMENT,
    nom VARCHAR(256),
    idMusiques JSON,
    PRIMARY KEY(id)
);

CREATE TABLE musique(
    id INT AUTO_INCREMENT,
    nom VARCHAR(256),
    duree TIME,
    date DATE,
    style VARCHAR(64),
    tags JSON,
    thumbnail VARCHAR(1024),
    idArtiste INT,
    idAlbum INT,
    PRIMARY KEY(id),
    FOREIGN KEY (idAlbum) REFERENCES album(id),
    FOREIGN KEY (idArtiste) REFERENCES artiste(id)
);