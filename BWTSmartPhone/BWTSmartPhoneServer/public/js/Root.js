//FROM 15237 F2011 - Cross platform mobile web apps

var App = function(){
    this.usersDiv = $('#usersList');
    $('#listUsers').onButtonTap(this.listUsers.bind(this));
    $('#logout').onButtonTap(function(){
        window.location = '/logout';   
    });

    $('#setMsg').onButtonTap(this.setMsg.bind(this));
}

App.prototype.listUsers = function(){
    var req = $.ajax({
        url: '/db/root/users',
        type: 'POST',
        data: {}});
    req.done(this.setUsers.bind(this));

    req.fail(function(jqXHR, status, err){
        alert(err);
    });
}

App.prototype.setUsers = function(users){
    this.usersDiv.children().remove();
    var title = $('<h4>');
    title.text('Users:');
    this.usersDiv.append(title);
    for (var i = 0; i < users.length; i++){
        this.appendUser(users[i]);
    }
}

App.prototype.appendUser = function(user){
    var li = $('<li>');
    li.text(user.username);

    var props = $('<ul>');

    props.append(this.createProp(user, 'words'));
    props.append(this.createProp(user, 'lastMsgTimestamp'));
    props.append(this.createProp(user, 'lastIp'));
    props.append(this.createProp(user, 'lastHost'));
    props.append(this.createProp(user, 'lastUserAgent'));
    props.append(this.createProp(user, 'lastLoginTimestamp'));
    props.append(this.createProp(user, 'superuser'));
    props.append(this.createProp(user, '_id'));
    props.append(this.createProp(user, 'registeredTimestamp'));

    li.append(props);

    this.usersDiv.append(li);
}

App.prototype.createProp = function(obj, key){
    if (key in obj){
        var li = $('<li>');
        li.text(key + ': ' + obj[key]);
        return li;
    }
}

// App.prototype.setMsg = function(){
//     var msg = $('#msg').val();
//     var req = $.ajax({
//         url: '/db/me/setMsg',
//         type: 'POST',
//         data: {'msg': msg}
//     });
//     req.done(this.listUsers.bind(this));
// }

