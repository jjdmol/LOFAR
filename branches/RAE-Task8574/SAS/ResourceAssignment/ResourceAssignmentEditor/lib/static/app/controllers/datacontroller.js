// $Id$


var dataControllerMod = angular.module('DataControllerMod', []);

dataControllerMod.controller('DataController', ['$http', '$scope', 'dataService', function($http, $scope, dataService) {
    var self = this;
    self.dataService = dataService;

    function getTasks() {
        $http.get('/rest/tasks').success(function(result) {
            self.dataService.tasks = result.tasks;
        });
    };

    function getResources() {
        $http.get('/rest/resourceitems').success(function(result) {

//             for(var key in result.resourceitems) {
//                 var elem = result.tasks[key];
//                 elem.from = new Date(elem.from);
//                 elem.to = new Date(elem.to);
//             }

            self.dataService.resources = result.resourceitems;
        });
    };

    getTasks();
    getResources();

  }
]);
