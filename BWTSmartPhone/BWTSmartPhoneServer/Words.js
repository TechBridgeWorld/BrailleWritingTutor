var mongoose = require('mongoose');
var Schema = mongoose.Schema;

var Word = new Schema({
    word: String,
    def: String,
    pos: String,
    assns: Array
});

module.exports = mongoose.model('Word', Word);

