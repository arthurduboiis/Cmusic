const mysql = require("mysql2")

const con = mysql.createConnection({
    host: process.env.HOST,
    user: process.env.USER,
    password: process.env.PASSWORD,
    database: process.env.DATABASE
});

con.connect(function (err){
    if(err) throw err;
    console.log('Connecté à la base de donnée MySQL');
})

module.exports = con;