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
                task.starttime = new Date(task.starttime);
                task.endtime = new Date(task.endtime);
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
        });
    };

    self.getResourceClaims = function() {
        $http.get('/rest/resourceclaims').success(function(result) {
            //convert datetime strings to Date objects
            for(var i = result.resourceclaims.length-1; i >=0; i--) {
                var resourceclaim = result.resourceclaims[i];
                resourceclaim.starttime = new Date(resourceclaim.starttime);
                resourceclaim.endtime = new Date(resourceclaim.endtime);
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
        });
    };

    self.getResourceGroupClaims = function() {
        $http.get('/rest/resourcegroupclaims').success(function(result) {
            //convert datetime strings to Date objects
            for(var i = result.resourcegroupclaims.length-1; i >=0; i--) {
                var resourcegroupclaim = result.resourcegroupclaims[i];
                resourcegroupclaim.starttime = new Date(resourcegroupclaim.starttime);
                resourcegroupclaim.endtime = new Date(resourcegroupclaim.endtime);
            }

            self.resourceGroupClaims = result.resourcegroupclaims;
            self.resourceGroupClaimDict = self.toIdBasedDict(self.resourceGroupClaims);
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

    self.lastUpdateTimestamp = undefined;

    self.subscribeToUpdates = function() {
        var url = '/rest/updates';
        if(self.lastUpdateTimestamp) {
            url += '/' + self.lastUpdateTimestamp;
        }
        $http.get(url, {timeout:300000}).success(function(result) {

            try {
                var changeTimestamps = result.changes.map(function(item) { return item.timestamp; });
                self.lastUpdateTimestamp = changeTimestamps.reduce(function(a, b, idx, arr) { return a > b ? a : b; }, undefined);

                for(var i = result.changes.length-1; i >=0; i--) {
                    try {
                        var change = result.changes[i];

                        if(change.objectType == 'task') {
                            var changedTask = change.value;
                            if(change.changeType == 'update') {
                                var task = self.taskDict[changedTask.id];
                                task.status = changedTask.status;
                                task.starttime = new Date(changedTask.starttime);
                                task.endtime = new Date(changedTask.endtime);
                            } else if(change.changeType == 'insert') {
                                self.tasks.push(changedTask);
                                self.taskDict[changedTask.id] = changedTask;
                            } else if(change.changeType == 'delete') {
                                delete self.taskDict[changedTask.id]
                                for(var k = self.tasks.length-1; k >= 0; k--) {
                                    if(self.tasks[k].id == changedTask.id) {
                                        self.tasks.splice(k, 1);
                                        break;
                                    }
                                }
                            }
                        } else if(change.objectType == 'resourceClaim') {
                            var changedClaim = change.value;
                            if(change.changeType == 'update') {
                                var claim = self.resourceClaimDict[changedClaim.id];
                                claim.status = changedClaim.status;
                                claim.starttime = new Date(changedClaim.starttime);
                                claim.endtime = new Date(changedClaim.endtime);
                            } else if(change.changeType == 'insert') {
                                self.resourceClaims.push(changedClaim);
                                self.resourceClaimDict[changedClaim.id] = changedClaim;
                            } else if(change.changeType == 'delete') {
                                delete self.resourceClaimDict[changedClaim.id]
                                for(var k = self.resourceClaims.length-1; k >= 0; k--) {
                                    if(self.resourceClaims[k].id == changedClaim.id) {
                                        self.resourceClaims.splice(k, 1);
                                        break;
                                    }
                                }
                            }
                        } else if(change.objectType == 'resourceGroupClaim') {
                            var changedGroupClaim = change.value;
                            var claim = self.resourceGroupClaimDict[changedGroupClaim.id];
                            claim.starttime = new Date(changedGroupClaim.starttime);
                            claim.endtime = new Date(changedGroupClaim.endtime);
                        }
                    } catch(err) {
                        console.log(err)
                    }
                }
            } catch(err) {
                console.log(err)
            }

            //and update again
            self.subscribeToUpdates();
        }).error(function() {
            setTimeout(self.subscribeToUpdates, 1000);
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
//     dataService.getResourceGroups();
//     dataService.getResourceGroupClaims();
    dataService.getResources();
    dataService.getResourceClaims();

    dataService.subscribeToUpdates();
}
]);
