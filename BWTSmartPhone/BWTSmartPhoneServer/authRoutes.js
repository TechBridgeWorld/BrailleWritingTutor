
var User = require('./User');
var Word = require('./Words');


// ******************************

module.exports = function (app) {
  // saves word to server from request
  app.post('/words', function(req, res) {
        if (req.user) {
            var word = new Word();
            word['word'] = req.body.word;
            word['def'] = req.body.def;
            word['assns'] = req.body.assns;
            word.save(function(err) {
                if (err) {
                    return res.send({'err': err});
                }
                return res.send({
                  result : "success",
                  data: word
                });
            });
        }
        console.log(word);
  });
  // finds and returns all words saved on server
  app.get('/words', function(req, res){
      if (req.secret !=='U84gAFW3QrJ7B56GLf2EzT2n2hY1vVZu'){
          res.status(401);
      }
      if (req.user || (req.secret ==='U84gAFW3QrJ7B56GLf2EzT2n2hY1vVZu')){
          Word.find({}, function (err, words){
              if (err) {
                  res.send(err);
              }
              else {
                  res.send(words);
              }});
      }
  });
  // finds and returns all words from request made by android
  app.get('/droid/words', function(req, res){
      if (req.secret !=='U84gAFW3QrJ7B56GLf2EzT2n2hY1vVZu'){
          res.status(401);
      }
      if (req.user || (req.secret ==='U84gAFW3QrJ7B56GLf2EzT2n2hY1vVZu')){
          Word.find({}, function (err, words){
              if (err) {
                  res.send(err);
              }
              else {
                var t = words.map(function(i){
                  return i.word;
                });
                res.send(t);
              }});
      }
  });

  // finds and returns specified word
  app.get('/words/:id', function(req, res) {
         var id = req.params.word;
        if (!req.user){
            res.status(401);
        }
        if ((req.user)){
            Word.findOne({'_id':new BSON.ObjectID(id)}, function (err, user){
                if (err) {
                    res.send(err);
                }
                else {
                    res.send(user);
                }});
        }
    });

  // lets user edit and modify existing words on server
  app.put('/words/:word', function(req, res) {
    var word = req.params.word;
    if ((req.user)){
      Word.find({'word': word}, null, function(err, success){
          if (err) {
            res.send(err);
          }
          else {
              Word.remove({'word': word}, function(err){
                if(err)
                var word = new Word();
                for (var key in req.body) {
                   var value=req.body[key];
                   word[key]=value;
                }
                word.save(function(err){
                  if(err){
                    res.send({
                      result: 'fail',
                      error: err
                    });
                  }
                  else{
                    res.send({
                      result: 'success'
                    });
                  }
                });
                res.send({
                  result: 'success',
                });
              });

          }});
    }
  });
  // lets user delete word form server
  // never delete all words
  app.delete('/words/:word' , function(req,res) {
    var word = req.params.word;
    console.log('Updating word: ' + word);
    if ((req.user)){
      Word.remove({'word' : word}, function(err){
        if(err){
          res.send({
            'result':'error',
            'message':err
          });
        }
        else{
          res.send({
            'result':'success'
          });
        }
      });

    }
  });
};
