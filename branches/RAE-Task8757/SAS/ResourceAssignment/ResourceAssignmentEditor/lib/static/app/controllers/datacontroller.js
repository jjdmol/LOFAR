// $Id$

angular.module('raeApp').factory("dataService", ['$http', function($http){
    var self = this;
    self.tasks = [];
    self.resources = [];
    self.resourceGroups = [];
    self.resourceClaims = [];
    self.resourceGroupClaims = [];
    self.tasktypes = [];
    self.taskstatustypes = [];

    self.taskDict = {};
    self.resourceDict = {};
    self.resourceGroupsDict = {};
    self.resourceClaimDict = {};
    self.resourceGroupClaimDict = {};
    self.resourceIdToGroupIdsDict = {};

    self.resourcesWithClaims = [];

    self.filteredTasks = [];
    self.filteredTaskDict = {};

    self.toIdBasedDict = function(list) {
        var dict = {}
        for(var i = list.length-1; i >=0; i--) {
            var item = list[i];
            dict[item.id] = item;
        }
        return dict;
    };

    self.mapResourcesToGroups = function () {
        var dict = {}
        var resources = self.resources;
        var resourceGroups = self.resourceGroups;

        if(resources.length > 0) {
            for(var i = resources.length-1; i >=0; i--)
                dict[resources[i].id] = []

                for(var i = resourceGroups.length-1; i >=0; i--) {
                    var group = resourceGroups[i];
                    var childResourceIds = group.resourceIds;

                    for(var j = childResourceIds.length-1; j >=0; j--) {
                        var childResourceId = childResourceIds[j];
                        dict[childResourceId].push(group.id);
                    }
                }
        }

        self.resourceIdToGroupIdsDict = dict;
    };

    self.getTasks = function() {
        $http.get('/rest/tasks').success(function(result) {
            //convert datetime strings to Date objects
            for(var i = result.tasks.length-1; i >=0; i--) {
                var task = result.tasks[i];
                task.from = new Date(task.from);
                task.to = new Date(task.to);
            }

            self.tasks = result.tasks;
            self.taskDict = self.toIdBasedDict(self.tasks);

            self.filteredTasks = self.tasks;
            self.filteredTaskDict = self.taskDict;
        });
    };

    self.putTask = function(task) {
        $http.put('/rest/tasks/' + task.id, task).error(function(result) {
            console.log("Error. Could not update task. " + result);
        })
    };

    self.getResources = function() {
        $http.get('/rest/resourceitems').success(function(result) {
            self.resources = result.resourceitems;
            self.resourceDict = self.toIdBasedDict(self.resources);
            self.mapResourcesToGroups();

            self.getResourceGroupClaims();
        });
    };

    self.getResourceClaims = function() {
        $http.get('/rest/resourceclaims').success(function(result) {
            //convert datetime strings to Date objects
            for(var i = result.resourceclaims.length-1; i >=0; i--) {
                var resourceclaim = result.resourceclaims[i];
                resourceclaim.startTime = new Date(resourceclaim.startTime);
                resourceclaim.endTime = new Date(resourceclaim.endTime);
            }

            self.resourceClaims = result.resourceclaims;
            self.resourceClaimDict = self.toIdBasedDict(self.resourceClaims);
        });
    };

    self.getResourceGroups = function() {
        $http.get('/rest/resourcegroups').success(function(result) {
            self.resourceGroups = result.resourcegroups;
            self.resourceGroupsDict = self.toIdBasedDict(self.resourceGroups);
            self.mapResourcesToGroups();

            self.getResources();
        });
    };

    self.getResourceGroupClaims = function() {
        $http.get('/rest/resourcegroupclaims').success(function(result) {
            //convert datetime strings to Date objects
            for(var i = result.resourcegroupclaims.length-1; i >=0; i--) {
                var resourcegroupclaim = result.resourcegroupclaims[i];
                resourcegroupclaim.startTime = new Date(resourcegroupclaim.startTime);
                resourcegroupclaim.endTime = new Date(resourcegroupclaim.endTime);
            }

            self.resourceGroupClaims = result.resourcegroupclaims;
            self.resourceGroupClaimDict = self.toIdBasedDict(self.resourceGroupClaims);

            setTimeout(function() { self.getResourceClaims() }, 100);
        });
    };

    self.getTaskTypes = function() {
        $http.get('/rest/tasktypes').success(function(result) {
            self.tasktypes = result.tasktypes;
        });
    };

    self.getTaskStatusTypes = function() {
        $http.get('/rest/taskstatustypes').success(function(result) {
            self.taskstatustypes = result.taskstatustypes;
        });
    };

    self.subscribeToUpdates = function() {
        $http.get('/rest/updates', {timeout:300000}).success(function(result) {
            //as demo, get updated tasks
            //we should however parse the results for changes
            self.getTasks();

            //and update again
            //TODO: implement update since previous update timestamp so we don't get any gaps.
            self.subscribeToUpdates();
        }).error(function() {
            console.log("update timeout!");
            self.subscribeToUpdates();
        });
    };

    return self;
}]);

var dataControllerMod = angular.module('DataControllerMod', ['ngResource']);

dataControllerMod.controller('DataController',
                            ['$scope', 'dataService',
                            function($scope, dataService) {
    var self = this;
    self.dataService = dataService;


    dataService.getTaskTypes();
    dataService.getTaskStatusTypes();
    dataService.getTasks();
    dataService.getResourceGroups();

    dataService.subscribeToUpdates();
}
]);
