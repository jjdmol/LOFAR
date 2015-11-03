// $Id$

var app = angular.module('raeApp',
                         ['ngTouch',
                         'gantt',
                         'gantt.table',
                         'gantt.movable',
                         'gantt.tooltips',
                         'DataControllerMod',
                         'GridControllerMod']);

app.factory("dataService", function(){
    var self = this;
    self.tasks = [];
    self.resources = [];
    return self;
});
