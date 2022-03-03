INSERT INTO artiste (nom_artiste) VALUES ('The Weekend')
INSERT INTO artiste (nom_artiste) VALUES ('Drake')
INSERT INTO artiste (nom_artiste) VALUES ('DJ Snake')
INSERT INTO artiste (nom_artiste) VALUES ('Damso')
INSERT INTO artiste (nom_artiste) VALUES ('Travis Scott')

INSERT INTO musique (nom_music,date_ajout,durée) VALUES ('God\'s Plan','05-01-2022','3:18');
INSERT INTO musique (nom_music,date_ajout,durée) VALUES ('Blinding Light','01-11-2021','3:22');
INSERT INTO musique (nom_music,date_ajout,durée) VALUES ('Save Your Tears','21-12-2021','3:35');
INSERT INTO musique (nom_music,date_ajout,durée) VALUES ('Propaganda','06-02-2022','4:09');
INSERT INTO musique (nom_music,date_ajout,durée) VALUES ('Magenta Riddim','26-10-2021','3:14');
INSERT INTO musique (nom_music,date_ajout,durée) VALUES ('Trust Nobody','29-09-2021','3:31');
INSERT INTO musique (nom_music,date_ajout,durée) VALUES ('911','29-09-2021','2:52');
INSERT INTO musique (nom_music,date_ajout,durée) VALUES ('Antidote','19-09-2021','4:22');
INSERT INTO musique (nom_music,date_ajout,durée) VALUES ('Escape Plan','01-03-2022','2:29');

INSERT INTO creer (music_id, artiste_id) VALUES (1,2)
INSERT INTO creer (music_id, artiste_id) VALUES (2,1)
INSERT INTO creer (music_id, artiste_id) VALUES (3,1)
INSERT INTO creer (music_id, artiste_id) VALUES (4,3)
INSERT INTO creer (music_id, artiste_id) VALUES (5,3)
INSERT INTO creer (music_id, artiste_id) VALUES (6,3)
INSERT INTO creer (music_id, artiste_id) VALUES (7,4)
INSERT INTO creer (music_id, artiste_id) VALUES (8,5)
INSERT INTO creer (music_id, artiste_id) VALUES (9,5)

INSERT INTO tag (nom_tag) VALUES ('House')
INSERT INTO tag (nom_tag) VALUES ('Rap')

INSERT INTO appartient (music_id, tag_id) VALUES (4, 1)
INSERT INTO appartient (music_id, tag_id) VALUES (5, 1)
INSERT INTO appartient (music_id, tag_id) VALUES (6, 1)
INSERT INTO appartient (music_id, tag_id) VALUES (1, 2)
INSERT INTO appartient (music_id, tag_id) VALUES (7, 2)
INSERT INTO appartient (music_id, tag_id) VALUES (8, 2)
INSERT INTO appartient (music_id, tag_id) VALUES (9, 2)

INSERT INTO playlist (nom_playlist, date_creation) VALUES ('Américain','01-11-2021')
INSERT INTO playlist (nom_playlist, date_creation) VALUES ('Francais','29-09-2021')

INSERT INTO possede (music_id, playlist_id) VALUES (1, 1)
INSERT INTO possede (music_id, playlist_id) VALUES (2, 1)
INSERT INTO possede (music_id, playlist_id) VALUES (3, 1)
INSERT INTO possede (music_id, playlist_id) VALUES (4, 2)
INSERT INTO possede (music_id, playlist_id) VALUES (5, 2)
INSERT INTO possede (music_id, playlist_id) VALUES (6, 2)
INSERT INTO possede (music_id, playlist_id) VALUES (7, 2)
INSERT INTO possede (music_id, playlist_id) VALUES (8, 1)
INSERT INTO possede (music_id, playlist_id) VALUES (9, 1)