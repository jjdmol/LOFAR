// $Id$

angular.module('raeApp').factory("dataService", ['$http', function($http){
    var self = this;
    self.tasks = [];
    self.resources = [];
    self.resourceGroups = [];
    self.resourceClaims = [];
    self.tasktypes = [];
    self.taskstatustypes = [];

    self.taskDict = {};
    self.resourceDict = {};
    self.resourceGroupsDict = {};
    self.resourceGroupMemberships = {};
    self.resourceClaimDict = {};
    self.tasktypesDict = {};

    self.momProjects = [];
    self.momProjectsDict = {};

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
        $http.get('/rest/resources').success(function(result) {
            self.resources = result.resources;
            self.resourceDict = self.toIdBasedDict(self.resources);
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
        });
    };

    self.getResourceGroupMemberships = function() {
        $http.get('/rest/resourcegroupmemberships').success(function(result) {
            self.resourceGroupMemberships = result.resourcegroupmemberships;
        });
    };

    self.getTaskTypes = function() {
        $http.get('/rest/tasktypes').success(function(result) {
            self.tasktypes = result.tasktypes;
            self.tasktypesDict = self.toIdBasedDict(self.tasktypes);
        });
    };

    self.getTaskStatusTypes = function() {
        $http.get('/rest/taskstatustypes').success(function(result) {
            self.taskstatustypes = result.taskstatustypes;
        });
    };

    self.getMoMProjects = function() {
        $http.get('/rest/momprojects').success(function(result) {
            //convert datetime strings to Date objects
            var dict = {};
            var list = [];
            for(var i = result.momprojects.length-1; i >=0; i--) {
                var momproject = result.momprojects[i];
                momproject.statustime = new Date(momproject.statustime);
                dict[momproject.mom_id] = momproject;
                list.push(momproject);
            }

            list.sort(function(a, b) { return ((a.name < b.name) ? -1 : ((a.name > b.name) ? 1 : 0)); });

            self.momProjects = list;
            self.momProjectsDict = dict;
        });
    };

    self.getMoMObjectDetailsForTask = function(task) {
        $http.get('/rest/momobjectdetails/'+task.mom_id).success(function(result) {
            if(result.momobjectdetails) {
                task.name = result.momobjectdetails.object_name;
                task.project_name = result.momobjectdetails.project_name;
                task.project_id = result.momobjectdetails.project_mom_id;
            }
        });
    };



    self.lastUpdateChangeNumber = undefined;

    self.initialLoad = function() {
        $http.get('/rest/mostRecentChangeNumber').success(function(result) {
            if(result.mostRecentChangeNumber >= 0) {
                self.lastUpdateChangeNumber = result.mostRecentChangeNumber;
            }

            self.getMoMProjects();
            self.getTaskTypes();
            self.getTaskStatusTypes();
            self.getTasks();
            self.getResourceGroups();
            self.getResources();
            self.getResourceGroupMemberships();
            self.getResourceClaims();

            self.subscribeToUpdates();
        });
    };

    self.subscribeToUpdates = function() {
        var url = '/rest/updates';
        if(self.lastUpdateChangeNumber) {
            url += '/' + self.lastUpdateChangeNumber;
        }
        $http.get(url, {timeout:300000}).success(function(result) {

            try {
                var changeNumbers = result.changes.map(function(item) { return item.changeNumber; });
                self.lastUpdateChangeNumber = changeNumbers.reduce(function(a, b, idx, arr) { return a > b ? a : b; }, undefined);

                for(var i = result.changes.length-1; i >=0; i--) {
                    try {
                        var change = result.changes[i];

                        if(change.objectType == 'task') {
                            var changedTask = change.value;
                            if(change.changeType == 'update') {
                                var task = self.taskDict[changedTask.id];
                                if(task) {
                                    task.status = changedTask.status;
                                    task.mom_id = changedTask.mom_id;
                                    task.otdb_id = changedTask.otdb_id;
                                    task.starttime = new Date(changedTask.starttime);
                                    task.endtime = new Date(changedTask.endtime);
                                    task.predecessor_ids = changedTask.predecessor_ids;
                                    task.successor_ids = changedTask.successor_ids;
                                }
                            } else if(change.changeType == 'insert') {
                                var task = self.taskDict[changedTask.id];
                                if(!task) {
                                    self.tasks.push(changedTask);
                                    self.taskDict[changedTask.id] = changedTask;
                                }
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
                                if(claim) {
                                    claim.status = changedClaim.status;
                                    claim.starttime = new Date(changedClaim.starttime);
                                    claim.endtime = new Date(changedClaim.endtime);
                                }
                            } else if(change.changeType == 'insert') {
                                var claim = self.resourceClaimDict[changedClaim.id];
                                if(!claim) {
                                    self.resourceClaims.push(changedClaim);
                                    self.resourceClaimDict[changedClaim.id] = changedClaim;
                                }
                            } else if(change.changeType == 'delete') {
                                delete self.resourceClaimDict[changedClaim.id]
                                for(var k = self.resourceClaims.length-1; k >= 0; k--) {
                                    if(self.resourceClaims[k].id == changedClaim.id) {
                                        self.resourceClaims.splice(k, 1);
                                        break;
                                    }
                                }
                            }
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

    dataService.initialLoad();
}
]);
