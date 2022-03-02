// Dans le terminal faire: npm install mysql2

const {Sequelize} = require('sequelize');

const sequelize = new Sequelize("base_ptut", "root", "", {
  dialect: "mysql",
  host: "localhost"
});

try {
   sequelize.authenticate();
   console.log('Connecté à la base de données MySQL!');
 } catch (error) {
   console.error('Impossible de se connecter, erreur suivante :', error);
 }

 // Utiliser sequelize.query() pour faire des requêtes
