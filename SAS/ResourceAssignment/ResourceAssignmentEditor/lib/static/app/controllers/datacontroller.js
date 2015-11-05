// $Id$

angular.module('raeApp').factory("dataService", function(){
    var self = this;
    self.tasks = [];
    self.resources = [];
    self.resourceclaims = [];

    self.taskDict = {};
    self.resourceDict = {};
    self.resourceclaimDict = {};

    self.resourcesWithClaims = [];

    return self;
});

var dataControllerMod = angular.module('DataControllerMod', []);

dataControllerMod.controller('DataController', ['$http', '$scope', 'dataService', function($http, $scope, dataService) {
    var self = this;
    self.dataService = dataService;

    function toIdBasedDict(list) {
        var dict = {}
        for(var i = list.length-1; i >=0; i--) {
            var item = list[i];
            dict[item.id] = item;
        }
        return dict;
    };

    function getTasks() {
        $http.get('/rest/tasks').success(function(result) {
            self.dataService.tasks = result.tasks;

            self.dataService.taskDict = toIdBasedDict(self.dataService.tasks);
        });
    };

    function getResources() {
        $http.get('/rest/resourceitems').success(function(result) {
            self.dataService.resources = result.resourceitems;

            self.dataService.resourceDict = toIdBasedDict(self.dataService.resources);
            groupResourceClaims();
        });
    };

    function getResourceClaims() {
        $http.get('/rest/resourceclaims').success(function(result) {
            self.dataService.resourceclaims = result.resourceclaims;

            self.dataService.resourceclaimDict = toIdBasedDict(self.dataService.resourceclaims);
            groupResourceClaims();
        });
    };

    function groupResourceClaims() {
        $http.get('/rest/resourceclaims').success(function(result) {
            self.dataService.resourceclaims = result.resourceclaims;

            var grouped = {};
            var resources = self.dataService.resources;

            for(var i = self.dataService.resources.length-1; i >=0; i--)
            {
                var resource = resources[i];
                grouped[resource.id] = {
                    'id': resource.id,
                    'name': resource.name,
                    'tasks': []
                };
            }

            var resourceclaims = result.resourceclaims;
            for(var i = result.resourceclaims.length-1; i >=0; i--)
            {
                var claim = resourceclaims[i];
                var task = self.dataService.taskDict[claim.taskId];

                var row = grouped[claim.resourceId];
                row.tasks.push({
                    name: task ? task.name : '<unknown>',
                    'from': claim.startTime,
                    'to': claim.endTime
                });
            }

            var groupedArray = [];

            for (var groupId in grouped)
                groupedArray.push(grouped[groupId]);

            self.dataService.resourcesWithClaims = groupedArray;
        });
    };

    getTasks();
    getResources();
    getResourceClaims();
  }
]);
