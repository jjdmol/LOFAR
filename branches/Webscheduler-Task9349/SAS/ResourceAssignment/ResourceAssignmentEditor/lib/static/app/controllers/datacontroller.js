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

    self.config = {};

    self.selected_resource_id;
    self.selected_resourceGroup_id;
    self.selected_task_id;
    self.selected_project_id;
    self.selected_resourceClaim_id;

    self.initialLoadComplete = false;
    self.taskChangeCntr = 0;
    self.claimChangeCntr = 0;

    self.loadedHours = {};

    self.viewTimeSpan = {from: new Date(), to: new Date() };

    self.floorDate = function(date, hourMod=1, minMod=1) {
        var min = date.getMinutes();
        min = date.getMinutes()/minMod;
        min = Math.floor(date.getMinutes()/minMod);
        min = minMod*Math.floor(date.getMinutes()/minMod);
        return new Date(date.getFullYear(), date.getMonth(), date.getDate(), hourMod*Math.floor(date.getHours()/hourMod), minMod*Math.floor(date.getMinutes()/minMod));
    };

    self.ceilDate = function(date, hourMod=1, minMod=1) {
        return new Date(date.getFullYear(), date.getMonth(), date.getDate(), hourMod*Math.ceil(date.getHours()/hourMod), minMod*Math.ceil(date.getMinutes()/minMod));
    };

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
                             'queued': '#ccffff',
                             'active': '#ffff00',
                             'completing': '#ffdd99',
                             'finished': '#00ff00',
                             'aborted': '#cc0000',
                             'error': '#990033',
                             'obsolete': '#555555'};



    //-- IMPORTANT REMARKS ABOUT UTC/LOCAL DATE --
    //Dates (datetimes) across javascript/angularjs/3rd party modules are a mess!
    //every module uses their own parsing and displaying
    //some care about utc/local date, others don't
    //So, to be consistent in this schedular client app, we chose the following conventions:
    //1) All received/sent timestamps are strings in iso format 'yyyy-MM-ddTHH:mm:ssZ'
    //2) All displayed timestamps should display the time in UTC, regardless of the timezone of the client.
    //All javascript/angularjs/3rd party modules display local times correct and the same...
    //So, to make 1&2 happen, we convert all received timestamps to 'local-with-utc-diff-correction', and then treat them as local.
    //And if we send them to the web server, we convert them back to real utc.
    //It's a stupid solution, but it works.
    //-- IMPORTANT REMARKS ABOUT UTC/LOCAL DATE --

    convertDatestringToLocalUTCDate = function(dateString) {
        //first convert the dateString to proper Date
        var date = new Date(dateString)
        //then do our trick to offset the timestamp with the utcOffset, see explanation above.
        return new Date(date.getTime() - self.utcOffset)
    };

    convertLocalUTCDateToISOString = function(local_utc_date) {
        //reverse trick to offset the timestamp with the utcOffset, see explanation above.
        var real_utc = new Date(local_utc_date.getTime() + self.utcOffset)
        return real_utc.toISOString();
    };

    //local client time offset to utc in milliseconds
    self.utcOffset = moment().utcOffset()*60000;

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
                    existingObj[prop] = convertDatestringToLocalUTCDate(changedObj[prop]);
                } else {
                    existingObj[prop] = changedObj[prop];
                }
            }
        }
    };

    self.getTasksAndClaimsForViewSpan = function() {
        var from = self.floorDate(self.viewTimeSpan.from, 1, 60);
        var until = self.ceilDate(self.viewTimeSpan.to, 1, 60);
        var lowerTS = from.getTime();
        var upperTS = until.getTime();

        for (var timestamp = lowerTS; timestamp < upperTS; ) {
            if(self.loadedHours.hasOwnProperty(timestamp)) {
                timestamp += 3600000;
            }
            else {
                var chuckUpperLimit = Math.min(upperTS, timestamp + 24*3600000);
                for (var chunkTimestamp = timestamp; chunkTimestamp < chuckUpperLimit; chunkTimestamp += 3600000) {
                    if(self.loadedHours.hasOwnProperty(chunkTimestamp))
                        break;

                    self.loadedHours[chunkTimestamp] = null;
                }

                var hourLower = new Date(timestamp);
                var hourUpper = new Date(chunkTimestamp);
                if(hourUpper > hourLower) {
                    self.getTasks(hourLower, hourUpper);
                    self.getResourceClaims(hourLower, hourUpper);
                }
                timestamp = chunkTimestamp;
            }
        }
    };

    self.clearTasksAndClaimsOutsideViewSpan = function() {
        var from = self.floorDate(self.viewTimeSpan.from, 1, 60);
        var until = self.ceilDate(self.viewTimeSpan.to, 1, 60);

        var numTasks = self.tasks.length;
        var visibleTasks = [];
        for(var i = 0; i < numTasks; i++) {
            var task = self.tasks[i];
            if(task.endtime >= from && task.starttime <= until)
                visibleTasks.push(task);
        }

        self.tasks = visibleTasks;
        self.taskDict = self.toIdBasedDict(self.tasks);
        self.filteredTasks = self.tasks;
        self.filteredTaskDict = self.taskDict;

        self.computeMinMaxTaskTimes();


        var numClaims = self.resourceClaims.length;
        var visibleClaims = [];
        for(var i = 0; i < numClaims; i++) {
            var claim = self.resourceClaims[i];
            if(claim.endtime >= from && claim.starttime <= until)
                visibleClaims.push(claim);
        }

        self.resourceClaims = visibleClaims;
        self.resourceClaimDict = self.toIdBasedDict(self.resourceClaims);

        self.computeMinMaxResourceClaimTimes();

        var newLoadedHours = {};
        var fromTS = from.getTime();
        var untilTS = until.getTime();
        for(var hourTS in self.loadedHours) {
            if(hourTS >= fromTS && hourTS <= untilTS)
                newLoadedHours[hourTS] = null;
        }
        self.loadedHours = newLoadedHours;
    };

    self.getTasks = function(from, until) {
        var defer = $q.defer();
        var url = '/rest/tasks';
        if(from) {
            url += '/' + convertLocalUTCDateToISOString(from);

            if(until) {
                url += '/' + convertLocalUTCDateToISOString(until);
            }
        }

        $http.get(url).success(function(result) {
            //convert datetime strings to Date objects
            for(var i in result.tasks) {
                var task = result.tasks[i];
                task.starttime = convertDatestringToLocalUTCDate(task.starttime);
                task.endtime = convertDatestringToLocalUTCDate(task.endtime);

            }

            var initialTaskLoad = self.tasks.length == 0;

            var newTaskDict = self.toIdBasedDict(result.tasks);
            var newTaskIds = Object.keys(newTaskDict);

            for(var i = newTaskIds.length-1; i >= 0; i--) {
                var task_id = newTaskIds[i];
                if(!self.taskDict.hasOwnProperty(task_id)) {
                    var task = newTaskDict[task_id];
                    self.tasks.push(task);
                    self.taskDict[task_id] = task;
                }
            }

            self.filteredTasks = self.tasks;
            self.filteredTaskDict = self.taskDict;
            self.taskChangeCntr++;

            self.computeMinMaxTaskTimes();

            if(initialTaskLoad && self.tasks.length > 0) {
                setTimeout(function() {
                    //try to select current task
                    var currentTasks = self.tasks.filter(function(t) { return t.starttime <= self.lofarTime && t.endtime >= self.lofarTime; });

                    if(currentTasks.length > 0) {
                        self.selected_task_id = currentTasks[0].id;
                    } else {
                        //try to select next task
                        var nextTasks = self.tasks.filter(function(t) { return t.starttime >= self.lofarTime; }).sort();

                        if(nextTasks.length > 0) {
                            self.selected_task_id = nextTasks[0].id;
                        } else {
                            //try to select most recent task
                            var prevTasks = self.tasks.filter(function(t) { return t.endtime <= self.lofarTime; }).sort();

                            if(prevTasks.length > 0) {
                                self.selected_task_id = prevTasks[prevTasks.length-1].id;
                            } else {
                                self.selected_task_id = self.tasks[0].id;
                            }
                        }
                    }
                }, 1000);
            }

            defer.resolve();
        });

        return defer.promise;
    };

    self.putTask = function(task) {
        task.starttime = convertLocalUTCDateToISOString(task.starttime);
        task.endtime = convertLocalUTCDateToISOString(task.endtime);
        $http.put('/rest/tasks/' + task.id, task).error(function(result) {
            console.log("Error. Could not update task. " + result);
            //TODO: revert to old state
        })
    };

    self.copyTask = function(task) {
        $http.put('/rest/tasks/' + task.id + '/copy').error(function(result) {
            console.log("Error. Could not copy task. " + result);
            alert("Error: Could not copy task with mom id " + task.mom_id);
        })
    };

    self.computeMinMaxTaskTimes = function() {
        var starttimes = self.filteredTasks.map(function(t) { return t.starttime;});
        var endtimes = self.filteredTasks.map(function(t) { return t.endtime;});

        var minStarttime = new Date(Math.min.apply(null, starttimes));
        var maxEndtime = new Date(Math.max.apply(null, endtimes));

        self.taskTimes = {
            min: minStarttime,
            max: maxEndtime
        };
    };

    self.getResources = function() {
        var defer = $q.defer();
        $http.get('/rest/resources').success(function(result) {
            self.resources = result.resources;
            self.resourceDict = self.toIdBasedDict(self.resources);

            //try to select first storage resource as default selected_resource_id
            var storageResources = self.resources.filter(function(r) { return r.type_name == 'storage'; });
            if(storageResources.length > 0) {
                self.selected_resource_id = storageResources[0].id;
            } else {
                //else, just the first resource
                self.selected_resource_id = self.resources[0].id;
            }

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
                        usage.timestamp = convertDatestringToLocalUTCDate(usage.timestamp);
                    }
                }
                self.resourceUsagesDict[result.resourceusages[i].resource_id] = result.resourceusages[i];
            }

            defer.resolve();
        });

        return defer.promise;
    };

    self.getResourceClaims = function(from, until) {
        var defer = $q.defer();
        var url = '/rest/resourceclaims';
        if(from) {
            url += '/' + convertLocalUTCDateToISOString(from);

            if(until) {
                url += '/' + convertLocalUTCDateToISOString(until);
            }
        }

        $http.get(url).success(function(result) {
            //convert datetime strings to Date objects
            for(var i in result.resourceclaims) {
                var resourceclaim = result.resourceclaims[i];
                resourceclaim.starttime = convertDatestringToLocalUTCDate(resourceclaim.starttime);
                resourceclaim.endtime = convertDatestringToLocalUTCDate(resourceclaim.endtime);
            }

            var newClaimDict = self.toIdBasedDict(result.resourceclaims);
            var newClaimIds = Object.keys(newClaimDict);

            for(var i = newClaimIds.length-1; i >= 0; i--) {
                var claim_id = newClaimIds[i];
                if(!self.resourceClaimDict.hasOwnProperty(claim_id)) {
                    var claim = newClaimDict[claim_id];
                    self.resourceClaims.push(claim);
                    self.resourceClaimDict[claim_id] = claim;
                }
            }

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

        self.resourceClaimTimes = {
            min: minStarttime,
            max: maxEndtime
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

    self.getConfig = function() {
        var defer = $q.defer();
        $http.get('/rest/config').success(function(result) {
            self.config = result.config;
            defer.resolve();
        });

        return defer.promise;
    };


    //start with local client time
    //lofarTime will be synced with server,
    //because local machine might have incorrect clock
    //take utcOffset into account, see explanation above.
    self.lofarTime = new Date(Date.now() - self.utcOffset);

    self._syncLofarTimeWithServer = function() {
        $http.get('/rest/lofarTime', {timeout:1000}).success(function(result) {
            self.lofarTime = convertDatestringToLocalUTCDate(result.lofarTime);

            //check if local to utc offset has changed
            self.utcOffset = moment().utcOffset()*60000;
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

            var nrOfItemsToLoad = 7;
            var nrOfItemsLoaded = 0;
            var checkInitialLoadCompleteness = function() {
                nrOfItemsLoaded += 1;
                if(nrOfItemsLoaded >= nrOfItemsToLoad) {
                    self.initialLoadComplete = true;
                }
            };

            self.getConfig().then(checkInitialLoadCompleteness);
            self.getMoMProjects().then(checkInitialLoadCompleteness);
            self.getTaskTypes().then(checkInitialLoadCompleteness);
            self.getTaskStatusTypes().then(checkInitialLoadCompleteness);
            self.getResourceGroups().then(checkInitialLoadCompleteness);
            self.getResources().then(checkInitialLoadCompleteness);
            self.getResourceGroupMemberships().then(checkInitialLoadCompleteness);

            self.getTasksAndClaimsForViewSpan();

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

                            self.taskChangeCntr++;
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
                            
                            self.claimChangeCntr++;
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

    $scope.dateOptions = {
        formatYear: 'yyyy',
        startingDay: 1
    };

    $scope.viewFromDatePopupOpened = false;
    $scope.viewToDatePopupOpened = false;

    $scope.openViewFromDatePopup = function() { $scope.viewFromDatePopupOpened = true; };
    $scope.openViewToDatePopup = function() { $scope.viewToDatePopupOpened = true; };
    $scope.jumpTimespanWidths = [{value:30, name:'30 Minutes'}, {value:60, name:'1 Hour'}, {value:3*60, name:'3 Hours'}, {value:6*60, name:'6 Hours'}, {value:12*60, name:'12 Hours'}, {value:24*60, name:'1 Day'}, {value:2*24*60, name:'2 Days'}, {value:3*24*60, name:'3 Days'}, {value:5*24*60, name:'5 Days'}, {value:7*24*60, name:'1 Week'}, {value:14*24*60, name:'2 Weeks'}, {value:28*24*60, name:'4 Weeks'}];
    $scope.jumpTimespanWidth = $scope.jumpTimespanWidths[7];
    $scope.jumpToNow = function() {
        var floorLofarTime = dataService.floorDate(dataService.lofarTime, 1, 5);
        dataService.viewTimeSpan = {
            from: dataService.floorDate(new Date(floorLofarTime.getTime() - 0.33*$scope.jumpTimespanWidth.value*60*1000), 1, 5),
            to: dataService.ceilDate(new Date(floorLofarTime.getTime() + 0.67*$scope.jumpTimespanWidth.value*60*1000), 1, 5)
        };

        //automatically select current task
        var currentTasks = dataService.tasks.filter(function(t) { return t.starttime <= dataService.viewTimeSpan.to && t.endime >= dataService.viewTimeSpan.from; });
        if(currentTasks.lenght > 0) {
            dataService.selected_task_id = currentTasks[0].id;
        }
    };

    //initialize are now
    $scope.jumpToNow();

    $scope.jumpToSelectedTasks = function() {
        if(dataService.selected_task_id == undefined)
            return;

        var task = dataService.taskDict[dataService.selected_task_id];

        if(task == undefined)
            return;

        var taskDurationInmsec = task.endtime.getTime() - task.starttime.getTime();
        var taskDurationInMinutes = taskDurationInmsec/60000;
        var viewSpanInMinutes = taskDurationInMinutes;

        var fittingSpans = $scope.jumpTimespanWidths.filter(function(w) { return w.value >= taskDurationInMinutes; });
        if(fittingSpans.length > 0) {
            $scope.jumpTimespanWidth = fittingSpans[0];
            viewSpanInMinutes = $scope.jumpTimespanWidth.value;
        }

        var focusTime = new Date(task.starttime.getTime() + 0.5*taskDurationInmsec);

        dataService.viewTimeSpan = {
            from: dataService.floorDate(new Date(focusTime.getTime() - 0.33*viewSpanInMinutes*60*1000), 1, 5),
            to: dataService.ceilDate(new Date(focusTime.getTime() + 0.67*viewSpanInMinutes*60*1000), 1, 5)
        };
    };

    $scope.onJumpTimespanWidthChanged = function(span) {
        var focusTime = dataService.floorDate(dataService.lofarTime, 1, 5);

        if(dataService.selected_task_id != undefined) {
            var task = dataService.taskDict[dataService.selected_task_id];

            if(task) {
                focusTime = dataService.floorDate(task.starttime, 1, 5);
            }
        }

        dataService.viewTimeSpan = {
            from: dataService.floorDate(new Date(focusTime.getTime() - 0.33*$scope.jumpTimespanWidth.value*60*1000)),
            to: dataService.ceilDate(new Date(focusTime.getTime() + 0.67*$scope.jumpTimespanWidth.value*60*1000))
        };
    };

    $scope.$watch('dataService.viewTimeSpan.from', function() {
        if(dataService.viewTimeSpan.from >= dataService.viewTimeSpan.to) {
            dataService.viewTimeSpan.to = dataService.ceilDate(new Date(dataService.viewTimeSpan.from.getTime() + 60*60*1000), 1, 5);
        }
    });

    $scope.$watch('dataService.viewTimeSpan.to', function() {
        if(dataService.viewTimeSpan.to <= dataService.viewTimeSpan.from) {
            dataService.viewTimeSpan.from = dataService.floorDate(new Date(dataService.viewTimeSpan.to.getTime() - 60*60*1000), 1, 5);
        }
    });

    $scope.$watch('dataService.viewTimeSpan', function() {
        dataService.clearTasksAndClaimsOutsideViewSpan();
        dataService.getTasksAndClaimsForViewSpan();
    }, true);

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
        $scope.$evalAsync(function() {
            dataService.lofarTime = new Date(dataService.lofarTime.getTime() + elapsed);
        });

        setTimeout(self._doTimeTick, 1000);
    };
    self._doTimeTick();
}
]);

//extend the default dateFilter so that it always displays dates as 'yyyy-MM-dd HH:mm:ss'
//without any extra timezone string.
//see also comments above why we do tricks with local and utc dates
angular.module('raeApp').config(['$provide', function($provide) {
    $provide.decorator('dateFilter', ['$delegate', function($delegate) {
        var srcFilter = $delegate;

        function zeroPaddedString(num) {
            var numstr = num.toString();
            if(numstr.length < 2) {
                return '0' + numstr;
            }
            return numstr;
        };

        var extendsFilter = function() {
            if(arguments[0] instanceof Date && arguments.length == 1) {
                var date = arguments[0];
                var dateString =  date.getFullYear() + '-' + zeroPaddedString(date.getMonth()+1) + '-' + zeroPaddedString(date.getDate()) + ' ' +
                                  zeroPaddedString(date.getHours()) + ':' + zeroPaddedString(date.getMinutes()) + ':' + zeroPaddedString(date.getSeconds());

                return dateString;
            }
            return srcFilter.apply(this, arguments);
        }

        return extendsFilter;
    }])
}])
