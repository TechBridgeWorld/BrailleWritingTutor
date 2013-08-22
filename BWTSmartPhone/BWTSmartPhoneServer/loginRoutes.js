var passport = require('passport');
var User = require('./User');

module.exports = function (app) {
    
    app.get('/', function (req, res) {
        if (req.user === undefined){
            res.sendfile('html/unregistered.html');
        }
        else if (req.user.superuser){
            res.sendfile('html/index.html');
        }
        else {
            res.sendfile('html/registered.html');
        }
    });

    // not currently implemented but tested and functional
    // not implemented because fucntionality of multuple users not yet       // determined
    app.post('/register', function(req, res) {
        var username = req.body.username;
        
        User.findOne({username : username }, function(err, existingUser) {
            if (err){
                return res.send({'err': err});
            }
            if (existingUser) {
                return res.send('user exists');
            }

            var user = new User({ username : req.body.username });
            user.registeredTimestamp = new Date();
            user.setPassword(req.body.password, function(err) {
                if (err) {
                    return res.send({'err': err});
                }

                user.save(function(err) {
                    if (err) {
                        return res.send({'err': err});
                    }
                    return res.send('success');
                });
            });  
        });
    });
    // authenticates user with passport
    app.post('/login', passport.authenticate('local'), function(req, res) {
        console.log(req)
        req.user.lastUserAgent = req.headers['user-agent'];
        req.user.lastIp = req.ip;
        req.user.lastHost = req.host;
        req.user.lastLoginTimestamp = new Date();
        req.user.save();
        return res.send('success');
    });

    app.get('/logout', function(req, res) {
        req.logout();
        res.redirect('/');
    });
}
