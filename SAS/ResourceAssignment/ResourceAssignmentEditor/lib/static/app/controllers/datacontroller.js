// $Id$

angular.module('raeApp').factory("dataService", ['$http', '$q', function($http, $q){
    var self = this;
    self.tasks = [];
    self.resources = [];
    self.resourceGroups = [];
    self.resourceClaims = [];
    self.tasktypes = [];
    self.taskstatustypes = [];
    self.editableTaskStatusIds = [];

    self.taskDict = {};
    self.resourceDict = {};
    self.resourceGroupsDict = {};
    self.resourceGroupMemberships = {};
    self.resourceClaimDict = {};
    self.resourceUsagesDict = {};
    self.tasktypesDict = {};

    self.momProjects = [];
    self.momProjectsDict = {};

    self.resourcesWithClaims = [];

    self.filteredTasks = [];
    self.filteredTaskDict = {};

    self.taskTimes = {};
    self.resourceClaimTimes = {};

    self.selected_resource_id;
    self.selected_resourceGroup_id;
    self.selected_task_id;
    self.selected_project_id;
    self.selected_resourceClaim_id;

    self.initialLoadComplete = false;

    self.resourceClaimStatusColors = {'claimed':'#ffa64d',
                                      'conflict':'#ff0000',
                                      'allocated': '#66ff66',
                                      'mixed': '#bfbfbf'}

    self.taskStatusColors = {'prepared':'#cccccc',
                             'approved':'#8cb3d9',
                             'on_hold':'#b34700',
                             'conflict':'#ff0000',
                             'prescheduled': '#6666ff',
                             'scheduled': '#0000ff',
                             'queued': '#6666ff',
                             'active': '#ffff00',
                             'completing': '#99ff33',
                             'finished': '#00ff00',
                             'aborted': '#cc0000',
                             'error': '#990033',
                             'obsolete': '#555555'};



    //start with local client time
    //lofarTime will be synced with server,
    //because local machine might have incorrect clock
    self.lofarTime = new Date(Date.now());

    self.toIdBasedDict = function(list) {
        var dict = {}
        for(var i = list.length-1; i >=0; i--) {
            var item = list[i];
            dict[item.id] = item;
        }
        return dict;
    };

    self.applyChanges = function(existingObj, changedObj) {
        for(var prop in changedObj) {
            if(existingObj.hasOwnProperty(prop) &&
            changedObj.hasOwnProperty(prop) &&
            existingObj[prop] != changedObj[prop]) {
                if(existingObj[prop] instanceof Date && typeof changedObj[prop] === "string") {
                    existingObj[prop] = new Date(changedObj[prop]);
                } else {
                    existingObj[prop] = changedObj[prop];
                }
            }
        }
    };

    self.getTasks = function() {
        var defer = $q.defer();

        $http.get('/rest/tasks').success(function(result) {
            //convert datetime strings to Date objects
            for(var i in result.tasks) {
                var task = result.tasks[i];
                task.starttime = new Date(task.starttime);
                task.endtime = new Date(task.endtime);
            }

            self.tasks = result.tasks;
            self.taskDict = self.toIdBasedDict(self.tasks);

            self.filteredTasks = self.tasks;
            self.filteredTaskDict = self.taskDict;

            self.computeMinMaxTaskTimes();

            defer.resolve();
        });

        return defer.promise;
    };

    self.putTask = function(task) {
        $http.put('/rest/tasks/' + task.id, task).error(function(result) {
            console.log("Error. Could not update task. " + result);
            //TODO: revert to old state
        })
    };

    self.computeMinMaxTaskTimes = function() {
        var starttimes = self.filteredTasks.map(function(t) { return t.starttime;});
        var endtimes = self.filteredTasks.map(function(t) { return t.endtime;});

        var minStarttime = new Date(Math.min.apply(null, starttimes));
        var maxEndtime = new Date(Math.max.apply(null, endtimes));
        var fullTimespanInMinutes = (maxEndtime - minStarttime) / (60 * 1000);

        self.taskTimes = {
            min: minStarttime,
            max: maxEndtime,
            fullTimespanInMinutes: fullTimespanInMinutes
        };
    };

    self.getResources = function() {
        var defer = $q.defer();
        $http.get('/rest/resources').success(function(result) {
            self.resources = result.resources;
            self.resourceDict = self.toIdBasedDict(self.resources);

            self.selected_resource = self.resources[0];

            defer.resolve();
        });

        return defer.promise;
    };

    self.getResourceUsages = function() {
        var defer = $q.defer();
        $http.get('/rest/resourceusages').success(function(result) {
            //convert datetime strings to Date objects
            for(var i in result.resourceusages) {
                var resource_usages = result.resourceusages[i].usages;

                for(var status in resource_usages) {
                    var usages = resource_usages[status];
                    for(var usage of usages) {
                        usage.timestamp = new Date(usage.timestamp);
                    }
                }
                self.resourceUsagesDict[result.resourceusages[i].resource_id] = result.resourceusages[i];
            }

            defer.resolve();
        });

        return defer.promise;
    };

    self.getResourceClaims = function() {
        var defer = $q.defer();
        $http.get('/rest/resourceclaims').success(function(result) {
            //convert datetime strings to Date objects
            for(var i in result.resourceclaims) {
                var resourceclaim = result.resourceclaims[i];
                resourceclaim.starttime = new Date(resourceclaim.starttime);
                resourceclaim.endtime = new Date(resourceclaim.endtime);
            }

            self.resourceClaims = result.resourceclaims;
            self.resourceClaimDict = self.toIdBasedDict(self.resourceClaims);

            self.computeMinMaxResourceClaimTimes();

            defer.resolve();
        });

        return defer.promise;
    };

    self.computeMinMaxResourceClaimTimes = function() {
        var starttimes = self.resourceClaims.map(function(rc) { return rc.starttime;});
        var endtimes = self.resourceClaims.map(function(rc) { return rc.endtime;});

        var minStarttime = new Date(Math.min.apply(null, starttimes));
        var maxEndtime = new Date(Math.max.apply(null, endtimes));
        var fullTimespanInMinutes = (maxEndtime - minStarttime) / (60 * 1000);

        self.resourceClaimTimes = {
            min: minStarttime,
            max: maxEndtime,
            fullTimespanInMinutes: fullTimespanInMinutes
        };
    };

    self.getResourceGroups = function() {
        var defer = $q.defer();
        $http.get('/rest/resourcegroups').success(function(result) {
            self.resourceGroups = result.resourcegroups;
            self.resourceGroupsDict = self.toIdBasedDict(self.resourceGroups);

            defer.resolve();
        });

        return defer.promise;
    };

    self.getResourceGroupMemberships = function() {
        var defer = $q.defer();
        $http.get('/rest/resourcegroupmemberships').success(function(result) {
            self.resourceGroupMemberships = result.resourcegroupmemberships;

            defer.resolve();
        });

        return defer.promise;
    };

    self.getTaskTypes = function() {
        var defer = $q.defer();
        $http.get('/rest/tasktypes').success(function(result) {
            self.tasktypes = result.tasktypes;
            self.tasktypesDict = self.toIdBasedDict(self.tasktypes);

            defer.resolve();
        });

        return defer.promise;
    };

    self.getTaskStatusTypes = function() {
        var defer = $q.defer();
        $http.get('/rest/taskstatustypes').success(function(result) {
            self.taskstatustypes = result.taskstatustypes;

            self.editableTaskStatusIds = [];
            for(var taskstatustype of self.taskstatustypes) {
                if(taskstatustype.name == 'approved' || taskstatustype.name == 'conflict' || taskstatustype.name == 'prescheduled') {
                    self.editableTaskStatusIds.push(taskstatustype.id);
                }
            }

            defer.resolve();
        });

        return defer.promise;
    };

    self.getMoMProjects = function() {
        var defer = $q.defer();
        $http.get('/rest/momprojects').success(function(result) {
            //convert datetime strings to Date objects
            var dict = {};
            var list = [];
            for(var i in result.momprojects) {
                var momproject = result.momprojects[i];
                momproject.statustime = new Date(momproject.statustime);
                dict[momproject.mom_id] = momproject;
                list.push(momproject);
            }

            list.sort(function(a, b) { return ((a.name < b.name) ? -1 : ((a.name > b.name) ? 1 : 0)); });

            self.momProjects = list;
            self.momProjectsDict = dict;

            defer.resolve();
        });

        return defer.promise;
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

    self._syncLofarTimeWithServer = function() {
        $http.get('/rest/lofarTime', {timeout:1000}).success(function(result) {
            self.lofarTime = new Date(result.lofarTime);
        });

        setTimeout(self._syncLofarTimeWithServer, 60000);
    };
    self._syncLofarTimeWithServer();


    self.lastUpdateChangeNumber = undefined;

    self.initialLoad = function() {
        $http.get('/rest/mostRecentChangeNumber').success(function(result) {
            if(result.mostRecentChangeNumber >= 0) {
                self.lastUpdateChangeNumber = result.mostRecentChangeNumber;
            }

            var nrOfItemsToLoad = 8;
            var nrOfItemsLoaded = 0;
            var checkInitialLoadCompleteness = function() {
                nrOfItemsLoaded += 1;
                if(nrOfItemsLoaded >= nrOfItemsToLoad) {
                    self.initialLoadComplete = true;
                }
            };

            self.getMoMProjects().then(checkInitialLoadCompleteness);
            self.getTaskTypes().then(checkInitialLoadCompleteness);
            self.getTaskStatusTypes().then(checkInitialLoadCompleteness);
            self.getTasks().then(checkInitialLoadCompleteness);
            self.getResourceGroups().then(checkInitialLoadCompleteness);
            self.getResources().then(checkInitialLoadCompleteness);
            self.getResourceGroupMemberships().then(checkInitialLoadCompleteness);
            self.getResourceClaims().then(checkInitialLoadCompleteness);

            self.getResourceUsages();

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

                var anyResourceClaims = false;
                for(var i in result.changes) {
                    try {
                        var change = result.changes[i];

                        if(change.objectType == 'task') {
                            var changedTask = change.value;
                            if(change.changeType == 'update') {
                                var task = self.taskDict[changedTask.id];
                                if(task) {
                                    self.applyChanges(task, changedTask);
                                }
                            } else if(change.changeType == 'insert') {
                                var task = self.taskDict[changedTask.id];
                                if(!task) {
                                    changedTask.starttime = new Date(changedTask.starttime);
                                    changedTask.endtime = new Date(changedTask.endtime);
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

                            self.computeMinMaxTaskTimes();
                        } else if(change.objectType == 'resourceClaim') {
                            anyResourceClaims = true;
                            var changedClaim = change.value;
                            if(change.changeType == 'update') {
                                var claim = self.resourceClaimDict[changedClaim.id];
                                if(claim) {
                                    self.applyChanges(claim, changedClaim);
                                }
                            } else if(change.changeType == 'insert') {
                                var claim = self.resourceClaimDict[changedClaim.id];
                                if(!claim) {
                                    changedClaim.starttime = new Date(changedClaim.starttime);
                                    changedClaim.endtime = new Date(changedClaim.endtime);
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
                            
                            self.computeMinMaxResourceClaimTimes();
                        } else if(change.objectType == 'resourceCapacity') {
                            if(change.changeType == 'update') {
                                var changedCapacity = change.value;
                                var resource = self.resourceDict[changedCapacity.resource_id];
                                if(resource) {
                                    resource.available_capacity = changedCapacity.available;
                                    resource.total_capacity = changedCapacity.total;
                                }
                            }
                        } else if(change.objectType == 'resourceAvailability') {
                            if(change.changeType == 'update') {
                                var changedAvailability = change.value;
                                var resource = self.resourceDict[changedAvailability.resource_id];
                                if(resource) {
                                    resource.active = changedAvailability.total;
                                }
                            }
                        }
                    } catch(err) {
                        console.log(err)
                    }
                }
                if(anyResourceClaims) {
                    self.getResourceUsages();
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
    $scope.dataService = dataService;

    $scope.$watch('dataService.filteredTasks', dataService.computeMinMaxTaskTimes);

    dataService.initialLoad();

    //clock ticking every second
    //updating current lofarTime by the elapsed time since previous tick
    //lofarTime is synced every minute with server utc time.
    self._prevTick = Date.now();
    self._doTimeTick = function() {
        var tick = Date.now();
        var elapsed = tick - self._prevTick;
        self._prevTick = tick;
        //evalAsync, so lofarTime will be seen by watches
        $scope.$evalAsync(function() { dataService.lofarTime = new Date(dataService.lofarTime.getTime() + elapsed); });

        setTimeout(self._doTimeTick, 1000);
    };
    self._doTimeTick();
}
]);
