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

var dataControllerMod = angular.module('DataControllerMod', ['ngResource']);

dataControllerMod.factory('taskService', function($resource){
    return $resource("/rest/tasks/:id", {}, {
        query: { method: "GET", isArray: false }
    });
});

dataControllerMod.factory('resourceService', function($resource){
    return $resource("/rest/resourceitems/:id", {}, {
        query: { method: "GET", isArray: false }
    });
});

dataControllerMod.factory('resourceClaimsService', function($resource){
    return $resource("/rest/resourceclaims/:id", {}, {
        query: { method: "GET", isArray: false }
    });
});

dataControllerMod.controller('DataController',
                             ['$scope', 'dataService', 'taskService',  'resourceService',  'resourceClaimsService',
                             function($scope, dataService, taskService, resourceService, resourceClaimsService) {
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

    taskService.query(function(results) {
        self.dataService.tasks = results.tasks;
        self.dataService.taskDict = toIdBasedDict(self.dataService.tasks);
    });

    resourceService.query(function(results) {
        self.dataService.resources = results.resourceitems;
        self.dataService.resourceDict = toIdBasedDict(self.dataService.resources);
        groupResourceClaims();
    });

    resourceClaimsService.query(function(results) {
        self.dataService.resourceclaims = results.resourceclaims;
        self.dataService.resourceclaimDict = toIdBasedDict(self.dataService.resourceclaims);
        groupResourceClaims();
    });

    function groupResourceClaims() {
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

        var resourceclaims = self.dataService.resourceclaims;
        for(var i = resourceclaims.length-1; i >=0; i--)
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
    };

//     getTasks();
//     getResources();
//     getResourceClaims();
  }
]);
