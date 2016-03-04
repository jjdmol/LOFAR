// $Id: ganttresourcecontroller.js 32761 2015-11-02 11:50:21Z schaap $

var ganttResourceControllerMod = angular.module('GanttResourceControllerMod', [
                                        'gantt',
                                        'gantt.sortable',
                                        'gantt.movable',
                                        'gantt.drawtask',
                                        'gantt.tooltips',
                                        'gantt.bounds',
                                        'gantt.progress',
                                        'gantt.tree',
                                        'gantt.groups',
                                        'gantt.overlap',
                                        'gantt.resizeSensor']).config(['$compileProvider', function($compileProvider) {
    $compileProvider.debugInfoEnabled(false); // Remove debug info (angularJS >= 1.3)
}]);

ganttResourceControllerMod.controller('GanttResourceController', ['$scope', 'dataService', function($scope, dataService) {

    var self = this;
    self.doInitialCollapse = true;

    $scope.dataService = dataService;
    $scope.ganttData = []

    self.taskStatusColors = {'prepared':'#aaff00',
                             'approved':'#ffaa00',
                             'on_hold':'#ff0000',
                             'conflict':'#ffccaa',
                             'prescheduled': '#6666ff',
                             'scheduled': '#ff66ff',
                             'queued': '#bb6644',
                             'active': '#77ff77',
                             'completing': '#776688',
                             'finished': '#66ff33',
                             'aborted': '#ff3366',
                             'error': '#ff4488',
                             'obsolete': '#555555'}

    $scope.options = {
        mode: 'custom',
        scale: 'day',
        sideMode: 'Tree',
        treeHeaderContent: '<i class="fa fa-align-justify"></i> {{getHeader()}}',
        autoExpand: 'both',
        api: function(api) {
            // API Object is used to control methods and events from angular-gantt.
            $scope.api = api;

            api.core.on.ready($scope, function () {
                    api.tasks.on.moveEnd($scope, moveHandler);
                    api.tasks.on.resizeEnd($scope, moveHandler);
                }
            );
        }
    };

    function moveHandler(item)
    {
        var task = item.model.raTask;
        var updatedTask = {
            id: task.id,
            starttime: item.model.from._d.toISOString(),
            endtime: item.model.to._d.toISOString()
        };
        $scope.dataService.putTask(updatedTask);
    };

    function updateGanttData() {
        var ganttRowsDict = {};

        var resourceGroupsDict = $scope.dataService.resourceGroupsDict;
        var resourceGroups = $scope.dataService.resourceGroups;
        var numResourceGroups = resourceGroups.length;

        var resourceDict = $scope.dataService.resourceDict;
        var resources = $scope.dataService.resources;
        var numResources = resources.length;

        var resourceGroupMemberships = $scope.dataService.resourceGroupMemberships;

        var taskDict = $scope.dataService.filteredTaskDict;
        var numTasks = $scope.dataService.filteredTasks.length;

        var resourceClaimDict = $scope.dataService.resourceClaimDict;
        var resourceClaims = $scope.dataService.resourceClaims;
        var numResourceClaims = resourceClaims.length;

        //dict resourceGroup2GanttRows for fast lookup of ganttrows based on groupId
        var resourceGroup2GanttRows = {};

        // recursive method which creates ganttrows for a resourceGroup(Id),
        // and its childs, and adds the subtree to the parentRow
        var createGanttRowTree = function(groupId, parentRow) {
            var resourceGroup = resourceGroupsDict[groupId];

            var groupRowId = 'group_' + groupId;
            if(parentRow) {
                groupRowId += '_parent' + parentRow.id;
            }

            var ganttRow = {
                'id': groupRowId,
                'parent': parentRow ? parentRow.id : null,
                'name': resourceGroup.name,
                'tasks': []
            };

            ganttRowsDict[groupRowId] = ganttRow;

            //store ganttRow also in dict resourceGroup2GanttRows for fast lookup based on groupId
            if(!resourceGroup2GanttRows.hasOwnProperty(groupId)) {
                resourceGroup2GanttRows[groupId] = [];
            }
            resourceGroup2GanttRows[groupId].push(ganttRow);

            //recurse over the childs
            var numChilds = resourceGroupMemberships.groups[groupId].child_ids.length;
            for(var i = 0; i < numChilds; i++) {
                var childGroupId = resourceGroupMemberships.groups[groupId].child_ids[i];
                createGanttRowTree(childGroupId, ganttRow);
            }
        };

        //build tree of resourceGroups
        //note that one resourceGroup can be a child of multiple parents
        if(resourceGroupMemberships.hasOwnProperty('groups')) {
            for(var groupId in resourceGroupMemberships.groups) {
                if(resourceGroupMemberships.groups[groupId].parent_ids.length == 0) {
                    //resourceGroup is a root item (no parents)
                    //so start creating a ganttRow tree for this root and all its descendants
                    createGanttRowTree(groupId, null);
                }
            }
        }

        //dict resource2GanttRows for fast lookup of ganttrows based on resourceId
        var resource2GanttRows = {};

        //add resources to their parent resourceGroups
        //note that one resource can be a child of multiple parent resourceGroups
        if(resourceGroupMemberships.hasOwnProperty('resources')) {
            for(var resourceId in resourceGroupMemberships.resources) {
                var resource = resourceDict[resourceId];
                if(resource) {
                    //of which parent(s) is this resource a child?
                    var parentGroupIds = resourceGroupMemberships.resources[resourceId].parent_group_ids;

                    //loop over parents
                    //add a ganttRow for the resource to each parent ganttRow
                    for(var parentGroupId of parentGroupIds) {
                        //note that one parentResourceGroup can actually have multiple rows
                        //since each resourceGroup itself can have multiple parents
                        var parentGanttRows = resourceGroup2GanttRows[parentGroupId];

                        for(var parentGanttRow of parentGanttRows) {
                            var resourceGanttRowId = 'resource_' + resource.id + '_' + parentGanttRow.id;
                            var ganttRow = {
                                id: resourceGanttRowId,
                                parent: parentGanttRow.id,
                                name: resource.name,
                                tasks: []
                            };

                            ganttRowsDict[resourceGanttRowId] = ganttRow;

                            //store ganttRow also in dict resource2GanttRows for fast lookup based on groupId
                            if(!resource2GanttRows.hasOwnProperty(resourceId)) {
                                resource2GanttRows[resourceId] = [];
                            }
                            resource2GanttRows[resourceId].push(ganttRow);
                        }
                    }
                }
            }
        }

        //there are also resources which are not part of a group
        //add these as well.
        for(var resourceId in resourceDict) {
            var resource = resourceDict[resourceId];

            if(!resource2GanttRows.hasOwnProperty(resourceId)) {
                var resourceGanttRowId = 'resource_' + resource.id;
                var ganttRow = {
                    id: resourceGanttRowId,
                    name: resource.name,
                    tasks: []
                };

                ganttRowsDict[resourceGanttRowId] = ganttRow;
                resource2GanttRows[resourceId] = [ganttRow];
            }
        }

        if(numResourceClaims > 0 && numTasks > 0) {
            //dict resource2Claims for fast lookup of claims based on resourceId
            var resource2Claims = {};

            //and finally assign each resourceclaim to its resource in each group
            for(var claim of resourceClaims) {
                var resourceId = claim.resource_id;
                var task = taskDict[claim.task_id];

                if(!task) {
                    continue;
                }

                if(!resource2Claims.hasOwnProperty(resourceId)) {
                    resource2Claims[resourceId] = [];
                }
                resource2Claims[resourceId].push(claim);

                var ganttRows = resource2GanttRows[resourceId];

                if(!ganttRows) {
                    continue;
                }

                for(var ganttRow of ganttRows) {
                    var claimTask = {
                        id: claim.id,
                        name: task.name,
                        from: claim.starttime,
                        to: claim.endtime,
                        color: self.taskStatusColors[task.status],
                        raTask: task
                    };

                    ganttRow.tasks.push(claimTask);
                }
            }

            // recursive method which aggregates the properties of the descendants tree
            var aggregateDescendants = function(groupId) {
                var aggregatedClaims = {};

                var resourceIds = resourceGroupMemberships.groups[groupId].resource_ids;

                for(var resourceId of resourceIds) {
                    var claims = resource2Claims[resourceId];

                    for(var claim of claims) {
                        var taskId = claim.task_id;
                        if(taskId in aggregatedClaims) {
                            if(claim.starttime < aggregatedClaims[taskId].starttime) {
                                aggregatedClaims[taskId].starttime = claim.starttime;
                            }
                            if(claim.endtime > aggregatedClaims[taskId].endtime) {
                                aggregatedClaims[taskId].endtime = claim.endtime;
                            }
                        } else {
                            aggregatedClaims[taskId] = { starttime: claim.starttime,
                                                          endtime: claim.endtime };
                        }
                    }
                }

                var childGroupIds = resourceGroupMemberships.groups[groupId].child_ids;

                for(var childGroupId of childGroupIds) {

                    var subAggregatedClaims = aggregateDescendants(childGroupId);

                    for(var taskId in subAggregatedClaims) {
                        var subAggregatedClaim = subAggregatedClaims[taskId];
                        if(taskId in aggregatedClaims) {
                            if(subAggregatedClaim.starttime < aggregatedClaims[taskId].starttime) {
                                aggregatedClaims[taskId].starttime = subAggregatedClaim.starttime;
                            }
                            if(subAggregatedClaim.endtime > aggregatedClaims[taskId].endtime) {
                                aggregatedClaims[taskId].endtime = subAggregatedClaim.endtime;
                            }
                        } else {
                            aggregatedClaims[taskId] = { starttime: subAggregatedClaim.starttime,
                                                          endtime: subAggregatedClaim.endtime };
                        }
                    }
                }

                var ganttRows = resourceGroup2GanttRows[groupId];
                for(var ganttRow of ganttRows) {
                    for(var taskId in aggregatedClaims) {
                        var aggClaimForTask = aggregatedClaims[taskId];
                        var task = taskDict[taskId];
                        if(task) {
                            var claimTask = {
                                id: 'aggregatedClaimForTask_' + taskId + '_' + ganttRow.id,
                                name: task.name,
                                from: aggClaimForTask.starttime,
                                to: aggClaimForTask.endtime,
                                color: self.taskStatusColors[task.status],
                                raTask: task
                            };

                            ganttRow.tasks.push(claimTask);
                        }
                    }
                }
                return aggregatedClaims;
            };

            //now that the whole tree has been built,
            //and all resourceClaims are processed
            //loop over the root resourceGroup again
            //and aggregate the claims of the subtrees
            if(resourceGroupMemberships.hasOwnProperty('groups')) {
                for(var groupId in resourceGroupMemberships.groups) {
                    if(resourceGroupMemberships.groups[groupId].parent_ids.length == 0) {
                        //resourceGroup is a root item (no parents)
                        //aggregate the claims of the subtrees
                        aggregateDescendants(groupId);
                    }
                }
            }
        }

        var ganttRows = [];

        for (var rowId in ganttRowsDict)
            ganttRows.push(ganttRowsDict[rowId]);

        $scope.ganttData = ganttRows;

        if(self.doInitialCollapse && numResources && numResourceGroups)
        {
            doInitialCollapse = false;
//             setTimeout(function() { $scope.api.tree.collapseAll(); }, 50);
        }
    };

    $scope.$watch('dataService.tasks', updateGanttData, true);
    $scope.$watch('dataService.resources', updateGanttData);
    $scope.$watch('dataService.resourceClaims', updateGanttData, true);
    $scope.$watch('dataService.resourceGroups', updateGanttData);
    $scope.$watch('dataService.resourceGroupMemberships', updateGanttData);
    $scope.$watch('dataService.filteredTaskDict', updateGanttData);
}
]);
