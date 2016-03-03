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
        var task_id = undefined;
        if(item.row.model.id.startsWith('group')) {
            var claimGroupId = item.model.id;
            var claimGroup = $scope.dataService.resourceGroupClaimDict[claimGroupId];
            task_id = claimGroup.task_id;
        }
        else {
            var claimId = item.model.id;
            var claim = $scope.dataService.resourceClaimDict[claimId];
            task_id = claim.task_id;
        }
        if(task_id) {
            var task = $scope.dataService.taskDict[task_id];
            var updatedTask = {
                id: task.id,
                starttime: item.model.from._d.toISOString(),
                endtime: item.model.to._d.toISOString()
            };
            $scope.dataService.putTask(updatedTask);
        }
    };

    function updateGanttData() {
        var ganntRowsDict = {};

        var resourceGroupsDict = $scope.dataService.resourceGroupsDict;
        var resourceGroups = $scope.dataService.resourceGroups;
        var numResourceGroups = resourceGroups.length;

        var resourceDict = $scope.dataService.resourceDict;
        var resources = $scope.dataService.resources;
        var numResources = resources.length;

        var resourceGroupMemberships = $scope.dataService.resourceGroupMemberships;

        var taskDict = $scope.dataService.filteredTaskDict;
        var numTasks = $scope.dataService.filteredTasks.length;

        var resourceClaims = $scope.dataService.resourceClaims;
        var numResourceClaims = resourceClaims.length;

        //dict resourceGroup2GanttRows for fast lookup of ganttrows based on groupId
        var resourceGroup2GanttRows = {};

        // recursive method which creates ganntrows for a resourceGroup(Id),
        // and its childs, and adds the subtree to the parentRow
        var createGanttRowTree = function(groupId, parentRow) {
            var resourceGroup = resourceGroupsDict[groupId];

            var groupRowId = 'group_' + groupId;
            if(parentRow) {
                groupRowId += '_parent' + parentRow.id;
            }

            var ganntRow = {
                'id': groupRowId,
                'parent': parentRow ? parentRow.id : null,
                'name': resourceGroup.name,
                'tasks': []
            };

            ganntRowsDict[groupRowId] = ganntRow;

            //store ganntRow also in dict resourceGroup2GanttRows for fast lookup based on groupId
            if(!resourceGroup2GanttRows.hasOwnProperty(groupId)) {
                resourceGroup2GanttRows[groupId] = [];
            }
            resourceGroup2GanttRows[groupId].push(ganntRow);

            //recurse over the childs
            var numChilds = resourceGroupMemberships.groups[groupId].child_ids.length;
            for(var i = 0; i < numChilds; i++) {
                var childGroupId = resourceGroupMemberships.groups[groupId].child_ids[i];
                createGanttRowTree(childGroupId, ganntRow);
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
                    //add a ganntRow for the resource to each parent ganntRow
                    for(var parentGroupId of parentGroupIds) {
                        //note that one parentResourceGroup can actually have multiple rows
                        //since each resourceGroup itself can have multiple parents
                        var parentGanttRows = resourceGroup2GanttRows[parentGroupId];

                        for(var parentGanttRow of parentGanttRows) {
                            var resourceGanttRowId = 'resource_' + resource.id + '_' + parentGanttRow.id;
                            var ganntRow = {
                                'id': resourceGanttRowId,
                                'parent': parentGanttRow.id,
                                'name': resource.name,
                                'tasks': []
                            };

                            ganntRowsDict[resourceGanttRowId] = ganntRow;

                            //store ganntRow also in dict resource2GanttRows for fast lookup based on groupId
                            if(!resource2GanttRows.hasOwnProperty(resourceId)) {
                                resource2GanttRows[resourceId] = [];
                            }
                            resource2GanttRows[resourceId].push(ganntRow);
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
                var ganntRow = {
                    'id': resourceGanttRowId,
                    'name': resource.name,
                    'tasks': []
                };

                ganntRowsDict[resourceGanttRowId] = ganntRow;
                resource2GanttRows[resourceId] = [ganntRow];
            }
        }

        //and finally assign each resourceclaim to its resource in each group
        if(numResourceClaims > 0 && numTasks > 0) {
            for(var i = 0; i < numResourceClaims; i++) {
                var claim = resourceClaims[i];
                var task = taskDict[claim.task_id];

                if(!task) {
                    continue;
                }

                var ganntRows = resource2GanttRows[claim.resource_id];

                if(!ganntRows) {
                    continue;
                }

                for(var ganntRow of ganntRows) {
                    var claimTask = {
                        id: claim.id,
                        name: task.name,
                        from: claim.starttime,
                        to: claim.endtime,
                        color: self.taskStatusColors[task.status]
                    };

                    ganntRow.tasks.push(claimTask);
                }
            }
        }

        var ganntRows = [];

        for (var rowId in ganntRowsDict)
            ganntRows.push(ganntRowsDict[rowId]);

        $scope.ganttData = ganntRows;

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
